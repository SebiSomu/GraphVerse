#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_displayMode(DisplayMode::InitialGraph)
    , m_graphLoaded(false)
    , m_floydDone(false)
    , m_knGenerated(false)
    , m_mstDone(false)
    , m_tspDone(false)
{
    ui->setupUi(this);
    setWindowTitle("Problema Comis-Voiajorului - TSP");
    resize(900, 600);
    
    connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindow::onLoadGraph);
    connect(ui->btnFloyd, &QPushButton::clicked, this, &MainWindow::onFloydWarshall);
    connect(ui->btnKn, &QPushButton::clicked, this, &MainWindow::onGenerateKn);
    connect(ui->btnMST, &QPushButton::clicked, this, &MainWindow::onCalculateMST);
    connect(ui->btnTSP, &QPushButton::clicked, this, &MainWindow::onGenerateTSP);
    
    ui->btnFloyd->setEnabled(false);
    ui->btnKn->setEnabled(false);
    ui->btnMST->setEnabled(false);
    ui->btnTSP->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::scaleCoordinates() {
    if (m_graph.getNodes().empty()) return;
    
    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();
    
    for (const auto& node : m_graph.getNodes()) {
        minX = std::min(minX, node.getX());
        maxX = std::max(maxX, node.getX());
        minY = std::min(minY, node.getY());
        maxY = std::max(maxY, node.getY());
    }
    
    double padding = 80;
    double availableWidth = width() - 2 * padding - 180;
    double availableHeight = height() - 2 * padding;
    
    double scaleX = availableWidth / (maxX - minX);
    double scaleY = availableHeight / (maxY - minY);
    double scale = std::min(scaleX, scaleY);
    
    for (auto& node : m_graph.getNodes()) {
        double newX = padding + (node.getX() - minX) * scale;
        double newY = padding + (node.getY() - minY) * scale;
        node.setCoord(QPointF(newX, newY));
    }
}

void MainWindow::paintEvent(QPaintEvent *event) {
    QMainWindow::paintEvent(event);
    
    if (!m_graphLoaded) return;
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    bool showMST = (m_displayMode == DisplayMode::MST);
    bool showTSP = (m_displayMode == DisplayMode::TSPCircuit);
    
    m_graph.drawGraph(painter, showMST, showTSP);
    
    painter.save();
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);
    
    int infoY = height() - 120;
    int infoX = 20;
    
    QString modeText;
    switch (m_displayMode) {
        case DisplayMode::InitialGraph:
            modeText = "Graf Initial (conex)";
            break;
        case DisplayMode::CompleteGraph:
            modeText = "Graf Complet Kn (dupa Floyd-Warshall)";
            break;
        case DisplayMode::MST:
            modeText = "Arbore Partial de Cost Minim (Kruskal)";
            break;
        case DisplayMode::TSPCircuit:
            modeText = "Circuit TSP Aproximativ (Preordine)";
            break;
    }
    
    painter.drawText(infoX, infoY, "Mod: " + modeText);
    painter.drawText(infoX, infoY + 20, "Noduri: " + QString::number(m_graph.getNodes().size()));
    painter.drawText(infoX, infoY + 40, "Muchii: " + QString::number(m_graph.getEdges().size()));
    
    if (m_tspDone) {
        const auto& circuit = m_graph.getTSPCircuit();
        QString circuitStr = "Circuit: ";
        for (size_t i = 0; i < circuit.size(); i++) {
            circuitStr += QString::number(circuit[i]);
            if (i < circuit.size() - 1) circuitStr += " -> ";
        }
        
        QRect textRect(infoX, infoY + 60, width() - 40, height() - (infoY + 60)); 
        painter.drawText(textRect, Qt::TextWordWrap, circuitStr);
    }
    
    painter.restore();
}

void MainWindow::onLoadGraph() {
    QString filename = QCoreApplication::applicationDirPath() + "/orase.txt";
    
    if (!QFile::exists(filename)) filename = "orase.txt";
    if (!QFile::exists(filename)) filename = "../orase.txt";
    if (!QFile::exists(filename)) filename = "../../orase.txt";

    if (m_graph.loadFromFile(filename)) {
        m_graphLoaded = true;
        m_displayMode = DisplayMode::InitialGraph;
        scaleCoordinates();
        ui->btnFloyd->setEnabled(true);
        ui->lblStatus->setText("Graf incarcat: " + QString::number(m_graph.getNodes().size()) + " orase");
        update();
    } else {
        QMessageBox::warning(this, "Eroare", "Nu s-a putut incarca fisierul orase.txt");
    }
}

void MainWindow::onFloydWarshall() {
    m_graph.floydWarshall();
    m_floydDone = true;
    ui->btnKn->setEnabled(true);
    ui->lblStatus->setText("Floyd-Warshall completat - distante minime calculate");
    update();
}

void MainWindow::onGenerateKn() {
    m_graph.generateCompleteGraph();
    m_knGenerated = true;
    m_displayMode = DisplayMode::CompleteGraph;
    ui->btnMST->setEnabled(true);
    ui->lblStatus->setText("Graf complet Kn generat: " + QString::number(m_graph.getEdges().size()) + " muchii");
    update();
}

void MainWindow::onCalculateMST() {
    m_graph.kruskalMST();
    m_mstDone = true;
    m_displayMode = DisplayMode::MST;
    ui->btnTSP->setEnabled(true);
    ui->lblStatus->setText("MST calculat cu Kruskal: " + QString::number(m_graph.getMSTEdges().size()) + " muchii");
    update();
}

void MainWindow::onGenerateTSP() {
    m_graph.generateTSPCircuit();
    m_tspDone = true;
    m_displayMode = DisplayMode::TSPCircuit;
    
    const auto& circuit = m_graph.getTSPCircuit();
    int totalCost = 0;
    const auto& dist = m_graph.getDistMatrix();
    for (size_t i = 0; i < circuit.size() - 1; i++) {
        totalCost += dist[circuit[i] - 1][circuit[i + 1] - 1];
    }
    
    ui->lblStatus->setText("Circuit TSP generat - Cost total: " + QString::number(totalCost));
    update();
}
