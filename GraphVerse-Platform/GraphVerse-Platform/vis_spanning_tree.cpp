#include "vis_spanning_tree.h"
#include "algorithms/dfs_spanning_tree_solver.h"
#include "algorithms/bfs_spanning_tree_solver.h"
#include <QPainter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <vector>
#include <string>
#include <random>
#include <ctime>
#include <cmath>

VisSpanningTree::VisSpanningTree(QWidget* parent)
    : QWidget(parent), m_graph(nullptr), m_timer(new QTimer(this)),
      m_statusLabel(new QLabel(this)), m_timerLabel(new QLabel(this)),
      m_speedLabel(new QLabel("Speed", this)), m_pauseBtn(new QPushButton("Pause", this)),
      m_speedSlider(new QSlider(Qt::Horizontal, this)), m_toolbar(nullptr),
      m_stepIdx(0), m_animDone(false), m_currentAlgo(AlgoType::None),
      m_currentNode(-1), m_currentEdge({-1,-1}), m_rootNode(0)
{
    setMinimumSize(800, 600);
    
    auto* btnDFS = new QPushButton("DFS Tree", this);
    auto* btnBFS = new QPushButton("BFS Tree", this);
    auto* btnR = new QPushButton("Reset", this);
    
    m_pauseBtn->setEnabled(false);
    m_speedSlider->setRange(1, 100);
    m_speedSlider->setValue(50);
    m_speedSlider->setFixedWidth(150);
    m_paused = false;
    m_elapsedMs = 0;
    
    for(auto* b : {btnDFS, btnBFS, btnR, m_pauseBtn}) { 
        b->setFixedHeight(34); 
        b->setFixedWidth(110); 
    }
    
    QString base = "QPushButton{border-radius:6px;font-weight:bold;font-size:12px;}";
    btnDFS->setStyleSheet(base + "QPushButton{background:#3b82f6;color:white;}QPushButton:hover{background:#2563eb;}");
    btnBFS->setStyleSheet(base + "QPushButton{background:#10b981;color:white;}QPushButton:hover{background:#059669;}");
    btnR->setStyleSheet(base + "QPushButton{background:#ef4444;color:white;}QPushButton:hover{background:#dc2626;}");
    m_pauseBtn->setStyleSheet(base + "QPushButton{background:#6b7280;color:white;}QPushButton:hover{background:#4b5563;}");
    
    m_speedLabel->setStyleSheet("color:#cbd5e1;font-size:11px;");
    m_speedSlider->setStyleSheet(
        "QSlider::groove:horizontal { height: 4px; background: #374151; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: #60a5fa; width: 12px; margin: -5px 0; border-radius: 6px; }");

    m_statusLabel->setStyleSheet("color: white; font-size: 13px; font-weight: bold;");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setMinimumWidth(300);
    
    m_timerLabel->setStyleSheet("color:#facc15;font-size:12px;font-weight:bold;");
    m_timerLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    updateTimerLabel();

    auto* lbl = new QLabel("🟢 Tree Edge   🔴 Back Edge (cycle)   🟠 Cross Edge   ⬜ Unexplored   ⭐ Root Node", this);
    lbl->setStyleSheet("color:#aaaaaa;font-size:11px;");
    lbl->setAlignment(Qt::AlignCenter);
    
    m_toolbar = new QWidget(this);
    auto* vbox = new QVBoxLayout(m_toolbar);
    vbox->setContentsMargins(8,6,8,4);
    vbox->setSpacing(4);
    
    auto* hbox = new QHBoxLayout();
    hbox->addWidget(btnDFS); 
    hbox->addWidget(btnBFS); 
    hbox->addSpacing(16); 
    hbox->addWidget(btnR); 
    hbox->addWidget(m_pauseBtn);
    hbox->addSpacing(8); 
    hbox->addWidget(m_speedLabel); 
    hbox->addWidget(m_speedSlider);
    hbox->addStretch(1);
    hbox->addWidget(m_statusLabel, 3);
    hbox->addStretch(1);
    hbox->addWidget(m_timerLabel);
    
    vbox->addLayout(hbox);
    vbox->addWidget(lbl);
    
    m_toolbar->setGeometry(0,0,width(), 70);
    
    connect(btnDFS, &QPushButton::clicked, this, &VisSpanningTree::onDFSClicked);
    connect(btnBFS, &QPushButton::clicked, this, &VisSpanningTree::onBFSClicked);
    connect(btnR, &QPushButton::clicked, this, &VisSpanningTree::onResetClicked);
    connect(m_pauseBtn, &QPushButton::clicked, this, &VisSpanningTree::onPauseClicked);
    connect(m_speedSlider, &QSlider::valueChanged, this, &VisSpanningTree::onSpeedChanged);
    connect(m_timer, &QTimer::timeout, this, &VisSpanningTree::onAnimationTick);
    
    srand(static_cast<unsigned>(time(nullptr)));
    // buildGraph() is now called in showEvent to ensure proper centering
}

VisSpanningTree::~VisSpanningTree() = default;

void VisSpanningTree::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    if (m_toolbar)
        m_toolbar->setGeometry(0, 0, width(), 70);
}

void VisSpanningTree::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    static bool firstShow = true;
    if (firstShow) {
        firstShow = false;
        buildGraph();
    }
}

void VisSpanningTree::hideEvent(QHideEvent* event) {
    onResetClicked();
    QWidget::hideEvent(event);
}

void VisSpanningTree::buildGraph() {
    m_timer->stop(); 
    m_graph = std::make_unique<UndirectedGraph>();
    
    m_steps.clear(); 
    m_stepIdx = 0; 
    m_animDone = false; 
    m_currentAlgo = AlgoType::None;
    m_paused = false;
    m_pauseBtn->setText("Pause");
    m_pauseBtn->setEnabled(false);
    m_elapsedMs = 0;
    
    m_visitedNodes.clear();
    m_completedNodes.clear();
    m_treeEdges.clear();
    m_backEdges.clear();
    m_crossEdges.clear();
    m_currentNode = -1;
    m_currentEdge = {-1, -1};
    
    updateTimerLabel();
    m_statusLabel->clear();
    updateLabel();
    
    std::mt19937 rng(static_cast<unsigned>(time(nullptr)));
    std::uniform_int_distribution<int> costDist(1, 10);
    
    // Organic layout with ~10 nodes
    int w = width() > 800 ? width() : 800;
    int h = height() > 600 ? height() : 600;
    int cx = w / 2;
    int cy = h / 2 + 30; // offset for toolbar
    int radius = std::min(w, h) / 3;
    
    int numNodes = 10;
    for (int i = 0; i < numNodes; ++i) {
        double angle = 2.0 * M_PI * i / numNodes;
        int x = cx + static_cast<int>(radius * std::cos(angle));
        int y = cy + static_cast<int>(radius * std::sin(angle));
        
        // Add some jitter
        x += (rng() % 40) - 20;
        y += (rng() % 40) - 20;
        
        m_graph->addNode(QPoint(x, y));
    }
    
    std::vector<Node*> nodes;
    for(auto& n : m_graph->getNodes()) nodes.push_back(&n);
    
    // Outer ring
    for (int i = 0; i < numNodes; ++i) {
        m_graph->addEdge(*nodes[i], *nodes[(i+1)%numNodes], costDist(rng));
    }
    
    // Some cross connections to create cycles and cross edges
    m_graph->addEdge(*nodes[0], *nodes[4], costDist(rng));
    m_graph->addEdge(*nodes[1], *nodes[6], costDist(rng));
    m_graph->addEdge(*nodes[3], *nodes[7], costDist(rng));
    m_graph->addEdge(*nodes[8], *nodes[2], costDist(rng));
    m_graph->addEdge(*nodes[5], *nodes[9], costDist(rng));
    
    m_rootNode = nodes.empty() ? -1 : nodes[0]->getIndex(); // Set root to first node's index
    update();
}

void VisSpanningTree::startAnimation(AlgoType algo) {
    m_timer->stop(); 
    m_currentAlgo = algo;
    
    m_visitedNodes.clear();
    m_completedNodes.clear();
    m_treeEdges.clear();
    m_backEdges.clear();
    m_crossEdges.clear();
    m_currentNode = -1;
    m_currentEdge = {-1, -1};
    
    m_stepIdx = 0; 
    m_animDone = false;
    m_paused = false;
    m_pauseBtn->setText("Pause");
    m_pauseBtn->setEnabled(true);
    m_elapsedMs = 0;
    m_elapsedClock.restart();
    
    updateTimerLabel();
    m_statusLabel->setText("Initializing...");
    
    if(algo == AlgoType::DFS) {
        m_steps = DFSSpanningTreeSolver{}.solve(*m_graph, m_rootNode);
    } else if(algo == AlgoType::BFS) {
        m_steps = BFSSpanningTreeSolver{}.solve(*m_graph, m_rootNode);
    }
    
    m_timer->start(currentInterval());
}

void VisSpanningTree::onAnimationTick() {
    if(m_stepIdx >= static_cast<int>(m_steps.size())) { 
        m_elapsedMs += m_elapsedClock.elapsed();
        m_timer->stop(); 
        m_animDone = true; 
        m_currentNode = -1;
        m_currentEdge = {-1, -1};
        m_pauseBtn->setEnabled(false);
        updateTimerLabel();
        updateLabel();
        update(); 
        return; 
    }
    
    SpanningTreeStep step = m_steps[m_stepIdx++];
    m_currentAction = step.action;
    
    auto getOrderedEdge = [](int u, int v) {
        return (u < v) ? std::make_pair(u, v) : std::make_pair(v, u);
    };

    switch(step.action) {
        case SpanningTreeStep::Action::VisitNode:
            m_visitedNodes.insert(step.nodeIndex);
            m_currentNode = step.nodeIndex;
            m_currentEdge = {-1, -1};
            break;
        case SpanningTreeStep::Action::TreeEdge:
            m_treeEdges.insert(getOrderedEdge(step.edgeFrom, step.edgeTo));
            m_currentEdge = getOrderedEdge(step.edgeFrom, step.edgeTo);
            break;
        case SpanningTreeStep::Action::BackEdge:
            m_backEdges.insert(getOrderedEdge(step.edgeFrom, step.edgeTo));
            m_currentEdge = getOrderedEdge(step.edgeFrom, step.edgeTo);
            break;
        case SpanningTreeStep::Action::CrossEdge:
            m_crossEdges.insert(getOrderedEdge(step.edgeFrom, step.edgeTo));
            m_currentEdge = getOrderedEdge(step.edgeFrom, step.edgeTo);
            break;
        case SpanningTreeStep::Action::CompleteNode:
            m_completedNodes.insert(step.nodeIndex);
            m_currentNode = step.nodeIndex;
            m_currentEdge = {-1, -1};
            break;
    }
    
    updateLabel();
    updateTimerLabel();
    update();
}

void VisSpanningTree::updateLabel() {
    if(m_currentAlgo == AlgoType::None) { m_statusLabel->clear(); return; }
    
    QString algoName = (m_currentAlgo == AlgoType::DFS) ? "DFS Spanning Tree" : "BFS Spanning Tree";
    QColor algoCol = (m_currentAlgo == AlgoType::DFS) ? QColor(59, 130, 246) : QColor(16, 185, 129);
    
    if (m_animDone) {
        m_statusLabel->setText(QString("%1: Done! (%2 tree edges, %3 back edges, %4 cross edges)")
            .arg(algoName).arg(m_treeEdges.size()).arg(m_backEdges.size()).arg(m_crossEdges.size()));
    } else {
        m_statusLabel->setText(QString("%1: Building... (%2/%3 nodes, %4 tree edges)")
            .arg(algoName).arg(m_visitedNodes.size()).arg(m_graph->getNodes().size()).arg(m_treeEdges.size()));
    }
    
    m_statusLabel->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: bold;").arg(algoCol.name()));
}

void VisSpanningTree::updateTimerLabel() {
    qint64 totalMs = m_elapsedMs;
    if (!m_paused && m_timer->isActive())
        totalMs += m_elapsedClock.elapsed();
    double seconds = static_cast<double>(totalMs) / 1000.0;
    m_timerLabel->setText(QString("Time: %1s").arg(seconds, 0, 'f', 2));
}

int VisSpanningTree::currentInterval() const {
    int slider = m_speedSlider ? m_speedSlider->value() : 50;
    int minMs = 50;
    int maxMs = 1500;
    int slow = 101 - slider;
    int range = maxMs - minMs;
    return minMs + (slow * slow * range) / 10000;
}

void VisSpanningTree::onDFSClicked() { startAnimation(AlgoType::DFS); }
void VisSpanningTree::onBFSClicked() { startAnimation(AlgoType::BFS); }
void VisSpanningTree::onResetClicked() { buildGraph(); }
void VisSpanningTree::onSpeedChanged(int) {
    if (m_timer->isActive())
        m_timer->setInterval(currentInterval());
}

void VisSpanningTree::onPauseClicked() {
    if (m_currentAlgo == AlgoType::None || m_animDone)
        return;
    if (m_paused) {
        m_paused = false;
        m_pauseBtn->setText("Pause");
        m_elapsedClock.restart();
        m_timer->start(currentInterval());
        return;
    }
    m_paused = true;
    m_pauseBtn->setText("Resume");
    m_elapsedMs += m_elapsedClock.elapsed();
    m_timer->stop();
    updateTimerLabel();
}

void VisSpanningTree::paintEvent(QPaintEvent*) {
    QPainter p(this); 
    p.setRenderHint(QPainter::Antialiasing); 
    p.fillRect(rect(), QColor(14, 17, 32)); // Pallete::BG_SURFACE
    
    auto nodePos = [&](int idx) -> QPointF {
        for(const auto& n : m_graph->getNodes()) 
            if(n.getIndex() == idx) return QPointF(n.getX(), n.getY());
        return QPointF(0,0);
    };
    
    auto getOrderedEdge = [](int u, int v) {
        return (u < v) ? std::make_pair(u, v) : std::make_pair(v, u);
    };
    
    auto drawEd = [&](int u, int v, QColor col, int width, bool isDashed, int cost) {
        QPointF a = nodePos(u), b = nodePos(v);
        QPen pen(col, width);
        if (isDashed) pen.setStyle(Qt::DashLine);
        p.setPen(pen); 
        p.drawLine(a, b);
        
        // Draw cost
        QPointF mid = (a + b) / 2.0; 
        QLineF ln(a, b); 
        double angle = std::atan2(ln.dy(), ln.dx());
        QPointF off(-10 * std::sin(angle), 10 * std::cos(angle));
        
        p.save(); 
        QFont f = p.font(); 
        f.setPointSize(8); 
        f.setBold(true); 
        p.setFont(f);
        p.setPen(QColor(140, 148, 190)); // Palette::TEXT_SEC
        p.drawText(mid + off, QString::number(cost)); 
        p.restore();
    };

    // Draw all edges
    for(const auto& ed : m_graph->getEdges()) {
        int u = ed.getFirst().getIndex();
        int v = ed.getSecond().getIndex();
        int cost = ed.getCost();
        
        auto edgeKey = getOrderedEdge(u, v);
        
        if (m_treeEdges.count(edgeKey)) {
            // Tree Edge
            drawEd(u, v, QColor(0, 220, 100), 3, false, cost);
        } else if (m_backEdges.count(edgeKey)) {
            // Back Edge
            drawEd(u, v, QColor(239, 68, 68), 2, true, cost);
        } else if (m_crossEdges.count(edgeKey)) {
            // Cross Edge
            drawEd(u, v, QColor(245, 158, 11), 2, true, cost);
        } else {
            // Unexplored
            drawEd(u, v, QColor(60, 60, 75), 1, false, cost);
        }
        
        // Highlight active edge
        if (!m_animDone && m_currentEdge == edgeKey) {
            p.save();
            QPen glowPen(QColor(34, 211, 238, 150), 5); // Cyan glow
            p.setPen(glowPen);
            p.drawLine(nodePos(u), nodePos(v));
            p.restore();
        }
    }

    // Draw all nodes
    const int R = 12;
    for(const auto& n : m_graph->getNodes()) {
        int idx = n.getIndex();
        QRectF r(n.getX() - R, n.getY() - R, R * 2, R * 2);
        
        QColor fill = QColor(58, 58, 74); // Gri inchis default
        QColor border = QColor(100, 100, 120);
        
        if (m_completedNodes.count(idx)) {
            fill = QColor(224, 224, 224); // Alb/Gri deschis
            border = Qt::white;
        } else if (m_visitedNodes.count(idx)) {
            fill = QColor(0, 220, 100); // Verde
            border = QColor(0, 255, 120);
        }
        
        // Highlight active node
        if (!m_animDone && m_currentNode == idx) {
            fill = QColor(251, 191, 36); // Galben strălucitor
            border = Qt::white;
            
            // Glow effect
            p.save();
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(251, 191, 36, 120), 4));
            p.drawEllipse(n.getX() - R - 2, n.getY() - R - 2, (R + 2) * 2, (R + 2) * 2);
            p.restore();
        }
        
        p.setBrush(fill); 
        p.setPen(QPen(border, 2)); 
        p.drawEllipse(r);
        
        // Root Node Decorator
        if (idx == m_rootNode) {
            p.save();
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(255, 215, 0), 2)); // Auriu
            p.drawEllipse(n.getX() - R - 4, n.getY() - R - 4, (R + 4) * 2, (R + 4) * 2);
            QFont f = p.font();
            f.setPointSize(8);
            p.setFont(f);
            p.setPen(QColor(255, 215, 0));
            p.drawText(n.getX() - 6, n.getY() - R - 6, "⭐");
            p.restore();
        }
        
        // Text
        p.setPen(m_completedNodes.count(idx) ? Qt::black : Qt::white);
        QFont f = p.font(); 
        f.setPointSize(9); 
        f.setBold(true);
        p.setFont(f);
        p.drawText(r, Qt::AlignCenter, QString::number(idx));
    }
}
