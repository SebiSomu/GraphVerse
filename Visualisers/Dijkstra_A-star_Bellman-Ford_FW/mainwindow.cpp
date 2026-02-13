#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPainter>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QWidget>
#include <cstdlib>
#include <ctime>
#include <random>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_graph(nullptr)
    , m_startIdx(-1)
    , m_endIdx(-1)
    , m_timer(new QTimer(this))
    , m_displayTimer(new QTimer(this))
    , m_clockLabel(nullptr)
    , m_elapsedMs(0)
    , m_stepIdx(0)
    , m_animDone(false)
    , m_currentNode(-1)
    , m_hasCurrent(false)
    , m_currentAlgo(AlgoType::None)
{
    ui->setupUi(this);
    setWindowTitle("Pathfinding - Dijkstra / A* / Bellman-Ford / Floyd-Warshall");
    resize(COLS * SPACING + offsetX() * 2,
           ROWS * SPACING + offsetY() + 90);

    QPushButton* btnD  = new QPushButton("Dijkstra", this);
    QPushButton* btnA  = new QPushButton("A*", this);
    QPushButton* btnBF = new QPushButton("Bellman-Ford", this);
    QPushButton* btnFW = new QPushButton("Floyd-Warshall", this);
    QPushButton* btnR  = new QPushButton("Reset", this);

    btnD ->setFixedHeight(34); btnD ->setFixedWidth(110);
    btnA ->setFixedHeight(34); btnA ->setFixedWidth(80);
    btnBF->setFixedHeight(34); btnBF->setFixedWidth(130);
    btnFW->setFixedHeight(34); btnFW->setFixedWidth(150);
    btnR ->setFixedHeight(34); btnR ->setFixedWidth(90);

    QString base = "QPushButton{border-radius:6px;font-weight:bold;font-size:12px;}";
    btnD ->setStyleSheet(base+"QPushButton{background:#1e90ff;color:white;}"
                               "QPushButton:hover{background:#1070d0;}");
    btnA ->setStyleSheet(base+"QPushButton{background:#27ae60;color:white;}"
                               "QPushButton:hover{background:#1e8449;}");
    btnBF->setStyleSheet(base+"QPushButton{background:#e67e22;color:white;}"
                                "QPushButton:hover{background:#ca6f1e;}");
    btnFW->setStyleSheet(base+"QPushButton{background:#9b59b6;color:white;}"
                                "QPushButton:hover{background:#7d3c98;}");
    btnR ->setStyleSheet(base+"QPushButton{background:#e74c3c;color:white;}"
                               "QPushButton:hover{background:#c0392b;}");

    QLabel* lbl = new QLabel("Click node = Start   Shift+Click = End", this);
    lbl->setStyleSheet("color:#aaaaaa;font-size:11px;");

    m_clockLabel = new QLabel("0.000 s", this);
    m_clockLabel->setStyleSheet("color:#00dc50;font-size:18px;font-weight:bold;min-width:100px;");
    m_clockLabel->setAlignment(Qt::AlignCenter);

    QWidget* toolbar = new QWidget(this);
    QHBoxLayout* hbox    = new QHBoxLayout(toolbar);
    hbox->addWidget(btnD);
    hbox->addWidget(btnA);
    hbox->addWidget(btnBF);
    hbox->addWidget(btnFW);
    hbox->addSpacing(10);
    hbox->addWidget(btnR);
    hbox->addStretch();
    hbox->addWidget(m_clockLabel);
    hbox->addStretch();
    hbox->addWidget(lbl);
    hbox->setContentsMargins(8, 6, 8, 4);
    toolbar->setGeometry(0, 0, width(), 50);

    connect(btnD, &QPushButton::clicked, this, &MainWindow::onDijkstraClicked);
    connect(btnA, &QPushButton::clicked, this, &MainWindow::onAStarClicked);
    connect(btnBF, &QPushButton::clicked, this, &MainWindow::onBellmanFordClicked);
    connect(btnFW, &QPushButton::clicked, this, &MainWindow::onFloydWarshallClicked);
    connect(btnR, &QPushButton::clicked, this, &MainWindow::onResetClicked);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onAnimationTick);
    connect(m_displayTimer, &QTimer::timeout, this, [this]{
        if(m_currentAlgo == AlgoType::None) return;
        qint64 ms = m_animDone ? m_elapsedMs
                               : (m_elapsed.isValid() ? m_elapsed.elapsed() : 0);
        m_clockLabel->setText(QString("%1.%2 s")
                                  .arg(ms / 1000)
                                  .arg(ms % 1000, 3, 10, QChar('0')));
        m_clockLabel->setStyleSheet(
            m_animDone
                ? "color:white;font-size:18px;font-weight:bold;min-width:100px;"
                : "color:#00dc50;font-size:18px;font-weight:bold;min-width:100px;");
    });
    m_displayTimer->start(50);

    srand(static_cast<unsigned>(time(nullptr)));
    buildGraph();
}

MainWindow::~MainWindow() { delete m_graph; delete ui; }

void MainWindow::buildGraph()
{
    m_timer->stop();
    delete m_graph;
    m_graph = new DirectedGraph();
    resetAnimation();
    m_startIdx = -1;
    m_endIdx = -1;

    std::mt19937 rng(static_cast<unsigned>(time(nullptr)));
    std::uniform_int_distribution<int> costDist(1, 20);

    const int ox = offsetX(), oy = offsetY();

    for(int r = 0; r < ROWS; r++)
        for(int c = 0; c < COLS; c++)
            m_graph->addNode(QPoint(ox + c * SPACING, oy + r * SPACING));

    auto& nodes = m_graph->getNodes();
    auto  idx   = [&](int r, int c){ return r * COLS + c; };

    for(int r = 0; r < ROWS; r++)
        for(int c = 0; c < COLS - 1; c++) {
            m_graph->addEdge(nodes[idx(r,c)],   nodes[idx(r,c+1)], costDist(rng));
            m_graph->addEdge(nodes[idx(r,c+1)], nodes[idx(r,c)],   costDist(rng));
        }
    for(int r = 0; r < ROWS - 1; r++)
        for(int c = 0; c < COLS; c++) {
            m_graph->addEdge(nodes[idx(r,c)],   nodes[idx(r+1,c)], costDist(rng));
            m_graph->addEdge(nodes[idx(r+1,c)], nodes[idx(r,c)],   costDist(rng));
        }

    m_clockLabel->setText("0.000 s");
    m_clockLabel->setStyleSheet("color:#00dc50;font-size:18px;font-weight:bold;min-width:100px;");
    update();
}

void MainWindow::resetAnimation()
{
    m_steps.clear();
    m_bestCost.clear();
    m_parent.clear();
    m_finalPath.clear();
    m_stepIdx = 0;
    m_animDone = false;
    m_hasCurrent = false;
    m_currentNode = -1;
    m_currentAlgo = AlgoType::None;
    m_elapsedMs = 0;
}

int MainWindow::nodeAt(QPoint pos) const
{
    for(const auto& n : m_graph->getNodes()) {
        int dx = pos.x() - n.getX(), dy = pos.y() - n.getY();
        if(std::sqrt(dx*dx + dy*dy) <= NODE_R + 4)
            return n.getIndex();
    }
    return -1;
}

void MainWindow::mousePressEvent(QMouseEvent* e)
{
    if(!m_graph) return;
    int hit = nodeAt(e->pos());
    if(hit == -1) return;

    if(e->modifiers() & Qt::ShiftModifier) {
        m_endIdx = hit;
    } else {
        m_startIdx = hit;
        if(m_startIdx == m_endIdx) m_endIdx = -1;
    }
    resetAnimation();
    update();
}

void MainWindow::startAnimation(AlgoType algo)
{
    if(m_startIdx == -1 || m_endIdx == -1) return;

    m_timer->stop();
    resetAnimation();
    m_currentAlgo = algo;

    std::vector<int> path;
    if(algo == AlgoType::Dijkstra)
        m_steps = m_graph->dijkstra(m_startIdx, m_endIdx, path);
    else if(algo == AlgoType::AStar)
        m_steps = m_graph->aStar(m_startIdx, m_endIdx, path);
    else if(algo == AlgoType::BellmanFord)
        m_steps = m_graph->bellmanFord(m_startIdx, m_endIdx, path);
    else
        m_steps = m_graph->floydWarshall(m_startIdx, m_endIdx, path);

    m_finalPath = path;
    m_elapsed.start();
    m_timer->start(8);
}

void MainWindow::onAnimationTick()
{
    const int STEPS_PER_TICK = 3;

    for(int i = 0; i < STEPS_PER_TICK; ++i) {
        if(m_stepIdx >= (int)m_steps.size()) {
            m_timer->stop();
            m_animDone    = true;
            m_hasCurrent  = false;
            m_elapsedMs   = m_elapsed.elapsed();
            update();
            return;
        }
        const PathStep& s = m_steps[m_stepIdx++];
        m_currentNode     = s.nodeIndex;
        m_hasCurrent      = true;
        m_bestCost[s.nodeIndex] = s.cost;
        if(s.fromIndex != -1) m_parent[s.nodeIndex] = s.fromIndex;
    }
    update();
}

void MainWindow::onDijkstraClicked()     { startAnimation(AlgoType::Dijkstra);      }
void MainWindow::onAStarClicked()        { startAnimation(AlgoType::AStar);         }
void MainWindow::onBellmanFordClicked()  { startAnimation(AlgoType::BellmanFord);   }
void MainWindow::onFloydWarshallClicked(){ startAnimation(AlgoType::FloydWarshall); }
void MainWindow::onResetClicked()        { buildGraph(); }

void MainWindow::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), Qt::black);

    if(!m_graph) return;

    const int R = NODE_R;

    p.setPen(QPen(QColor(50, 50, 50), 1));
    for(const auto& ed : m_graph->getEdges()) {
        QPointF a(ed.getFirst().getX(),  ed.getFirst().getY());
        QPointF b(ed.getSecond().getX(), ed.getSecond().getY());
        QLineF ln(a, b);
        double angle  = std::atan2(ln.dy(), ln.dx());
        QPointF perp(-3*std::sin(angle), 3*std::cos(angle));
        p.drawLine(a + perp, b + perp);

        QPointF mid = (a + b) / 2.0 + perp * 2;
        p.save();
        QFont f = p.font(); f.setPointSize(6); p.setFont(f);
        p.setPen(QColor(80,80,80));
        p.drawText(mid, QString::number(ed.getCost()));
        p.restore();
    }

    p.setPen(QPen(QColor(0, 80, 180), 2));
    for(auto& [child, par] : m_parent) {
        const auto& nodes = m_graph->getNodes();
        QPointF a, b;
        for(const auto& n : nodes) {
            if(n.getIndex() == par)   a = QPointF(n.getX(), n.getY());
            if(n.getIndex() == child) b = QPointF(n.getX(), n.getY());
        }
        p.drawLine(a, b);
    }

    if(m_animDone && !m_finalPath.empty()) {
        p.setPen(QPen(QColor(255, 220, 0), 3));
        const auto& nodes = m_graph->getNodes();
        for(int i = 0; i + 1 < (int)m_finalPath.size(); i++) {
            QPointF a, b;
            for(const auto& n : nodes) {
                if(n.getIndex() == m_finalPath[i])   a = QPointF(n.getX(), n.getY());
                if(n.getIndex() == m_finalPath[i+1]) b = QPointF(n.getX(), n.getY());
            }
            p.drawLine(a, b);
        }
    }

    for(const auto& n : m_graph->getNodes()) {
        int idx = n.getIndex();
        QRect r(n.getX()-R, n.getY()-R, R*2, R*2);

        bool isStart   = (idx == m_startIdx);
        bool isEnd     = (idx == m_endIdx);
        bool isCurrent = (m_hasCurrent && idx == m_currentNode);
        bool isPath    = false;
        if(m_animDone)
            for(int pi : m_finalPath) if(pi == idx) { isPath = true; break; }
        bool explored  = m_bestCost.count(idx);

        QColor fill, border;
        if(isStart)        { fill = QColor(0,200,80);   border = Qt::white; }
        else if(isEnd)     { fill = QColor(220,40,40);  border = Qt::white; }
        else if(isCurrent) { fill = QColor(255,200,0);  border = Qt::yellow; }
        else if(isPath)    { fill = QColor(200,170,0);  border = QColor(255,220,0); }
        else if(explored)  { fill = QColor(20,60,140);  border = QColor(60,120,220); }
        else               { fill = QColor(40,40,50);   border = QColor(80,80,100); }

        p.setBrush(fill);
        p.setPen(QPen(border, isStart||isEnd ? 2 : 1));
        p.drawEllipse(r);

        p.setPen(Qt::white);
        QFont f = p.font();
        if(explored && !isStart && !isEnd) {
            f.setPointSize(5); f.setBold(true); p.setFont(f);
            p.drawText(r, Qt::AlignCenter, QString::number(m_bestCost[idx]));
        } else {
            f.setPointSize(6); f.setBold(false); p.setFont(f);
            p.drawText(r, Qt::AlignCenter, QString::number(idx));
        }
    }

    QString algoName;
    QColor  algoCol = Qt::white;
    if     (m_currentAlgo == AlgoType::Dijkstra)      { algoName = "Dijkstra";       algoCol = QColor(80,160,255);  }
    else if(m_currentAlgo == AlgoType::AStar)          { algoName = "A*";             algoCol = QColor(46,204,113);  }
    else if(m_currentAlgo == AlgoType::BellmanFord)    { algoName = "Bellman-Ford";   algoCol = QColor(230,126,34);  }
    else if(m_currentAlgo == AlgoType::FloydWarshall)  { algoName = "Floyd-Warshall"; algoCol = QColor(175,122,197); }

    int ty = offsetY() + ROWS * SPACING + 10;

    if(m_startIdx == -1 || m_endIdx == -1) {
        p.setPen(QColor(150,150,150));
        QFont f = p.font(); f.setPointSize(10); f.setBold(false); p.setFont(f);
        p.drawText(QRect(offsetX(), ty, COLS*SPACING, 28),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   m_startIdx == -1 ? "Click a node to set START"
                                    : "Shift+Click a node to set END");
        return;
    }

    if(m_currentAlgo != AlgoType::None) {
        QFont f = p.font(); f.setPointSize(10); f.setBold(true); p.setFont(f);
        p.setPen(algoCol);

        int pathCost = 0;
        for(int idx : m_finalPath)
            if(m_bestCost.count(idx)) { pathCost = m_bestCost[idx]; }

        QString status = m_animDone
                             ? QString("%1 - done!  Path length: %2 nodes  Cost: %3")
                                   .arg(algoName).arg(m_finalPath.size()).arg(pathCost)
                             : QString("%1 - searching...  explored: %2 nodes")
                                   .arg(algoName).arg(m_bestCost.size());

        p.drawText(QRect(offsetX(), ty, COLS*SPACING, 28),
                   Qt::AlignLeft | Qt::AlignVCenter, status);
    }
}
