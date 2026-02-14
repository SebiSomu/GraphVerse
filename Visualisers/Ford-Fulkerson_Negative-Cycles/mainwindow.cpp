#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_network(new FlowNetwork())
    , m_networkWidget(new NetworkWidget(this))
{
    ui->setupUi(this);
    setWindowTitle("Ford-Fulkerson / Negative Cycle Elimination");
    resize(1200, 800);
    m_networkWidget->setNetwork(m_network);
    setupUI();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_network;
}

void MainWindow::setupUI() {
    QWidget* central = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    QHBoxLayout* controls = new QHBoxLayout();

    m_modeGroup = new QButtonGroup(this);

    QRadioButton* rbAddNode   = new QRadioButton("Add Nodes",        this);
    QRadioButton* rbAddEdge   = new QRadioButton("Add Edges",        this);
    QRadioButton* rbSetSource = new QRadioButton("Set Source",       this);
    QRadioButton* rbSetSink   = new QRadioButton("Set Sink",         this);
    QRadioButton* rbView      = new QRadioButton("View (Algorithm)", this);

    m_modeGroup->addButton(rbAddNode,   0);
    m_modeGroup->addButton(rbAddEdge,   1);
    m_modeGroup->addButton(rbSetSource, 2);
    m_modeGroup->addButton(rbSetSink,   3);
    m_modeGroup->addButton(rbView,      4);

    rbAddNode->setChecked(true);
    m_networkWidget->setMode(NetworkWidget::Mode_AddNode);

    controls->addWidget(rbAddNode);
    controls->addWidget(rbAddEdge);
    controls->addWidget(rbSetSource);
    controls->addWidget(rbSetSink);
    controls->addWidget(rbView);

    connect(m_modeGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &MainWindow::onModeChanged);

    QPushButton* btnStart  = new QPushButton("Start Algorithm",       this);
    m_btnNextStep          = new QPushButton("Next Step",             this);
    QPushButton* btnNegCyc = new QPushButton("Elim. Neg. Cycles",     this);
    QPushButton* btnReset  = new QPushButton("Reset Result",          this);
    QPushButton* btnClear  = new QPushButton("Clear All",             this);

    m_btnNextStep->setEnabled(false);

    controls->addStretch();
    controls->addWidget(btnStart);
    controls->addWidget(m_btnNextStep);
    controls->addWidget(btnNegCyc);
    controls->addWidget(btnReset);
    controls->addWidget(btnClear);

    connect(btnStart,  &QPushButton::clicked, this, &MainWindow::onStartAlgorithm);
    connect(m_btnNextStep, &QPushButton::clicked, this, &MainWindow::onNextStep);
    connect(btnNegCyc, &QPushButton::clicked, this, &MainWindow::onEliminateNegCycles);
    connect(btnReset,  &QPushButton::clicked, this, &MainWindow::onReset);
    connect(btnClear,  &QPushButton::clicked, this, &MainWindow::onClear);

    mainLayout->addLayout(controls);

    m_statusLabel = new QLabel("Build the network, then press 'Start Algorithm'.", this);
    m_statusLabel->setStyleSheet("font-weight:bold; padding:5px; background-color:#eee; border:1px solid #ccc; color:black;");
    mainLayout->addWidget(m_statusLabel);

    mainLayout->addWidget(m_networkWidget);
    setCentralWidget(central);
}

void MainWindow::onModeChanged(int id) {
    if      (id == 0) m_networkWidget->setMode(NetworkWidget::Mode_AddNode);
    else if (id == 1) m_networkWidget->setMode(NetworkWidget::Mode_AddEdge);
    else if (id == 2) m_networkWidget->setMode(NetworkWidget::Mode_SetSource);
    else if (id == 3) m_networkWidget->setMode(NetworkWidget::Mode_SetSink);
    else if (id == 4) m_networkWidget->setMode(NetworkWidget::Mode_View);
}

void MainWindow::onStartAlgorithm() {
    if (m_network->getNumNodes() < 2) {
        QMessageBox::warning(this, "Error", "Graph is too small!");
        return;
    }
    if (m_network->getSource() == -1 || m_network->getSink() == -1) {
        QMessageBox::warning(this, "Error", "Source or Sink not set!");
        return;
    }
    m_network->initializeAlgorithm();
    m_networkWidget->setIterationIndex(0);
    m_statusLabel->setText("Algorithm initialized. Press 'Next Step'.");
    m_btnNextStep->setEnabled(true);
    m_modeGroup->button(4)->setChecked(true);
    m_networkWidget->setMode(NetworkWidget::Mode_View);
}

void MainWindow::onNextStep() {
    bool hasPath = m_network->performStep();
    if (hasPath) {
        const auto& iterations = m_network->getIterations();
        m_networkWidget->setIterationIndex(iterations.size() - 1);
        m_statusLabel->setText(iterations.back().description);
    } else {
        m_network->finalizeMinCut();
        m_network->addFinalState();
        const auto& iterations = m_network->getIterations();
        m_networkWidget->setIterationIndex(iterations.size() - 1);
        m_statusLabel->setText(QString("Algorithm finished! Max Flow: %1")
                                   .arg(iterations.back().totalFlow));
        m_btnNextStep->setEnabled(false);
    }
}

void MainWindow::onEliminateNegCycles() {
    if (m_network->getNumNodes() < 2) {
        QMessageBox::warning(this, "Error", "Graph is too small!");
        return;
    }
    int cycles = m_network->eliminateNegativeCycles();
    if (cycles == 0) {
        m_statusLabel->setText("No negative cycles found in the residual graph.");
    } else {
        const auto& iterations = m_network->getIterations();
        m_networkWidget->setIterationIndex(iterations.size() - 1);
        m_networkWidget->setMode(NetworkWidget::Mode_View);
        m_modeGroup->button(4)->setChecked(true);
        m_statusLabel->setText(QString("Eliminated %1 negative cycle(s). Residual graph updated.").arg(cycles));
    }
    m_networkWidget->update();
}

void MainWindow::onReset() {
    m_network->resetFlow();
    m_networkWidget->setIterationIndex(-1);
    m_statusLabel->setText("Results reset.");
    m_btnNextStep->setEnabled(false);
}

void MainWindow::onClear() {
    m_network->clear();
    m_networkWidget->setIterationIndex(-1);
    m_statusLabel->setText("Network cleared.");
    m_btnNextStep->setEnabled(false);
    m_networkWidget->update();
}
