#include "vis_shortest.h"
#include <QPainter>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <vector>
#include <string>
#include <random>
#include <ctime>
#include <cmath>

VisShortest::VisShortest(QWidget* parent)
    : QWidget(parent), m_graph(nullptr), m_startIdx(-1), m_endIdx(-1),
      m_timer(new QTimer(this)), m_statusLabel(new QLabel(this)),
      m_timerLabel(new QLabel(this)), m_speedLabel(new QLabel("Speed", this)),
      m_pauseBtn(new QPushButton("Pause", this)), m_speedSlider(new QSlider(Qt::Horizontal, this)), m_toolbar(nullptr),
      m_stepIdx(0), m_animDone(false),
      m_currentNode(-1), m_hasCurrent(false), m_currentAlgo(AlgoType::None)
{
    setMinimumSize(COLS*SPACING+offsetX()*2, ROWS*SPACING+70+90);
    auto* btnD = new QPushButton("Dijkstra", this);
    auto* btnA = new QPushButton("A*", this);
    auto* btnBF= new QPushButton("Bellman-Ford", this);
    auto* btnFW= new QPushButton("Floyd-Warshall", this);
    auto* btnR = new QPushButton("Reset", this);
    m_pauseBtn->setEnabled(false);
    m_speedSlider->setRange(1, 100);
    m_speedSlider->setValue(65);
    m_speedSlider->setFixedWidth(180);
    m_paused = false;
    m_elapsedMs = 0;
    btnD->setFixedSize(110,34); btnA->setFixedSize(80,34); btnBF->setFixedSize(130,34);
    btnFW->setFixedSize(150,34); btnR->setFixedSize(90,34); m_pauseBtn->setFixedSize(90,34);
    QString base = "QPushButton{border-radius:6px;font-weight:bold;font-size:12px;}";
    btnD->setStyleSheet(base+"QPushButton{background:#1e90ff;color:white;}QPushButton:hover{background:#1070d0;}");
    btnA->setStyleSheet(base+"QPushButton{background:#27ae60;color:white;}QPushButton:hover{background:#1e8449;}");
    btnBF->setStyleSheet(base+"QPushButton{background:#e67e22;color:white;}QPushButton:hover{background:#ca6f1e;}");
    btnFW->setStyleSheet(base+"QPushButton{background:#9b59b6;color:white;}QPushButton:hover{background:#7d3c98;}");
    btnR->setStyleSheet(base+"QPushButton{background:#e74c3c;color:white;}QPushButton:hover{background:#c0392b;}");
    m_pauseBtn->setStyleSheet(base+"QPushButton{background:#6b7280;color:white;}QPushButton:hover{background:#4b5563;}");
    m_speedLabel->setStyleSheet("color:#cbd5e1;font-size:11px;");
    m_speedSlider->setStyleSheet(
        "QSlider::groove:horizontal { height: 4px; background: #374151; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: #60a5fa; width: 12px; margin: -5px 0; border-radius: 6px; }");

    m_statusLabel->setStyleSheet("color: white; font-size: 13px; font-weight: bold;");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setMinimumWidth(360);
    m_timerLabel->setStyleSheet("color:#facc15;font-size:12px;font-weight:bold;");
    m_timerLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    updateTimerLabel();

    auto* lbl = new QLabel("Click node = Start   Shift+Click = End", this);
    lbl->setStyleSheet("color:#aaaaaa;font-size:11px;");
    m_toolbar = new QWidget(this);
    auto* hbox = new QHBoxLayout(m_toolbar);
    hbox->addWidget(btnD); hbox->addWidget(btnA); hbox->addWidget(btnBF); hbox->addWidget(btnFW);
    hbox->addSpacing(10); hbox->addWidget(btnR); hbox->addWidget(m_pauseBtn);
    hbox->addSpacing(8); hbox->addWidget(m_speedLabel); hbox->addWidget(m_speedSlider);
    hbox->addStretch(1);
    hbox->addWidget(m_statusLabel, 3);
    hbox->addStretch(1);
    hbox->addWidget(m_timerLabel);
    hbox->addSpacing(12);
    hbox->addWidget(lbl);
    hbox->setContentsMargins(8,6,8,4);
    m_toolbar->setGeometry(0,0,width(),50);
    connect(btnD, &QPushButton::clicked, this, &VisShortest::onDijkstraClicked);
    connect(btnA, &QPushButton::clicked, this, &VisShortest::onAStarClicked);
    connect(btnBF,&QPushButton::clicked, this, &VisShortest::onBellmanFordClicked);
    connect(btnFW,&QPushButton::clicked, this, &VisShortest::onFloydWarshallClicked);
    connect(btnR, &QPushButton::clicked, this, &VisShortest::onResetClicked);
    connect(m_pauseBtn, &QPushButton::clicked, this, &VisShortest::onPauseClicked);
    connect(m_speedSlider, &QSlider::valueChanged, this, &VisShortest::onSpeedChanged);
    connect(m_timer, &QTimer::timeout, this, &VisShortest::onAnimationTick);
    srand(static_cast<unsigned>(time(nullptr)));
    buildGraph();
}

VisShortest::~VisShortest() { delete m_graph; }

void VisShortest::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    if (m_toolbar)
        m_toolbar->setGeometry(0, 0, width(), 50);
}

void VisShortest::buildGraph() {
    m_timer->stop(); delete m_graph; m_graph = new DirectedGraph();
    resetAnimation(); m_startIdx = -1; m_endIdx = -1;
    std::mt19937 rng(static_cast<unsigned>(time(nullptr)));
    std::uniform_int_distribution<int> costDist(1, 20);
    const int ox = offsetX(), oy = offsetY();
    for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++) m_graph->addNode(QPoint(ox+c*SPACING, oy+r*SPACING));
    auto& nodes = m_graph->getNodes();
    auto idx = [&](int r, int c){ return r*COLS+c; };
    for(int r=0;r<ROWS;r++) for(int c=0;c<COLS-1;c++) {
        m_graph->addEdge(nodes[idx(r,c)], nodes[idx(r,c+1)], costDist(rng));
        m_graph->addEdge(nodes[idx(r,c+1)], nodes[idx(r,c)], costDist(rng));
    }
    for(int r=0;r<ROWS-1;r++) for(int c=0;c<COLS;c++) {
        m_graph->addEdge(nodes[idx(r,c)], nodes[idx(r+1,c)], costDist(rng));
        m_graph->addEdge(nodes[idx(r+1,c)], nodes[idx(r,c)], costDist(rng));
    }
    update();
}

void VisShortest::resetAnimation() {
    m_steps.clear(); m_bestCost.clear(); m_parent.clear(); m_finalPath.clear();
    m_stepIdx=0; m_animDone=false; m_hasCurrent=false; m_currentNode=-1; m_currentAlgo=AlgoType::None;
    m_paused = false;
    m_pauseBtn->setText("Pause");
    m_pauseBtn->setEnabled(false);
    m_elapsedMs = 0;
    updateTimerLabel();
    m_statusLabel->clear();
    updateLabel();
}

int VisShortest::nodeAt(QPoint pos) const {
    for(const auto& n : m_graph->getNodes()) {
        int dx=pos.x()-n.getX(), dy=pos.y()-n.getY();
        if(std::sqrt(dx*dx+dy*dy) <= NODE_R+4) return n.getIndex();
    }
    return -1;
}

void VisShortest::mousePressEvent(QMouseEvent* e) {
    if(!m_graph) return;
    int hit = nodeAt(e->pos());
    if(hit == -1) return;
    if(e->modifiers() & Qt::ShiftModifier) m_endIdx = hit;
    else { m_startIdx = hit; if(m_startIdx==m_endIdx) m_endIdx=-1; }
    resetAnimation(); update();
}

void VisShortest::startAnimation(AlgoType algo) {
    if(m_startIdx==-1 || m_endIdx==-1) return;
    m_timer->stop(); resetAnimation(); m_currentAlgo = algo;
    m_paused = false;
    m_pauseBtn->setText("Pause");
    m_pauseBtn->setEnabled(true);
    m_elapsedMs = 0;
    m_elapsedClock.restart();
    updateTimerLabel();
    std::vector<int> path;
    if(algo==AlgoType::Dijkstra)         m_steps = m_graph->dijkstra(m_startIdx, m_endIdx, path);
    else if(algo==AlgoType::AStar)       m_steps = m_graph->aStar(m_startIdx, m_endIdx, path);
    else if(algo==AlgoType::BellmanFord) m_steps = m_graph->bellmanFord(m_startIdx, m_endIdx, path);
    else                                 m_steps = m_graph->floydWarshall(m_startIdx, m_endIdx, path);
    m_finalPath = path; m_timer->start(currentInterval());
}

void VisShortest::onAnimationTick() {
    const int STEPS_PER_TICK = 3;
    for(int i=0; i<STEPS_PER_TICK; i++) {
        if(m_stepIdx >= static_cast<int>(m_steps.size())) { 
            m_elapsedMs += m_elapsedClock.elapsed();
            m_timer->stop(); m_animDone=true; m_hasCurrent=false; 
            m_pauseBtn->setEnabled(false);
            updateTimerLabel();
            updateLabel();
            update(); return; 
        }
        const auto& s = m_steps[m_stepIdx++];
        m_currentNode = s.nodeIndex; m_hasCurrent = true;
        m_bestCost[s.nodeIndex] = s.cost;
        if(s.fromIndex != -1) m_parent[s.nodeIndex] = s.fromIndex;
    }
    updateLabel();
    updateTimerLabel();
    update();
}

void VisShortest::updateLabel() {
    if(m_startIdx == -1 || m_endIdx == -1) {
        m_statusLabel->setText(m_startIdx == -1 ? "Select Start Node" : "Select End Node (Shift+Click)");
        m_statusLabel->setStyleSheet("color: #aaaaaa; font-size: 13px; font-weight: bold;");
        return;
    }
    if(m_currentAlgo == AlgoType::None) { m_statusLabel->clear(); return; }

    QString algoName; QColor algoCol = Qt::white;
    if(m_currentAlgo==AlgoType::Dijkstra)         {algoName="Dijkstra"; algoCol=QColor(80,160,255);}
    else if(m_currentAlgo==AlgoType::AStar)       {algoName="A*"; algoCol=QColor(46,204,113);}
    else if(m_currentAlgo==AlgoType::BellmanFord) {algoName="Bellman-Ford"; algoCol=QColor(230,126,34);}
    else if(m_currentAlgo==AlgoType::FloydWarshall){algoName="Floyd-Warshall"; algoCol=QColor(175,122,197);}

    int pathCost=0; if(!m_finalPath.empty() && m_bestCost.count(m_endIdx)) pathCost=m_bestCost[m_endIdx];
    QString status = m_animDone ? QString("%1: Done! Final Cost: %2").arg(algoName).arg(pathCost)
                                : QString("%1: Searching...").arg(algoName);
    m_statusLabel->setText(status);
    m_statusLabel->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: bold;").arg(algoCol.name()));
}

void VisShortest::updateTimerLabel() {
    qint64 totalMs = m_elapsedMs;
    if (!m_paused && m_timer->isActive())
        totalMs += m_elapsedClock.elapsed();
    double seconds = static_cast<double>(totalMs) / 1000.0;
    m_timerLabel->setText(QString("Time: %1s").arg(seconds, 0, 'f', 2));
}

int VisShortest::currentInterval() const {
    int slider = m_speedSlider ? m_speedSlider->value() : 65;
    int minMs = 2;
    int maxMs = 140;
    int slow = 101 - slider;
    int range = maxMs - minMs;
    return minMs + (slow * slow * range) / 10000;
}

void VisShortest::onDijkstraClicked()     { startAnimation(AlgoType::Dijkstra); }
void VisShortest::onAStarClicked()        { startAnimation(AlgoType::AStar); }
void VisShortest::onBellmanFordClicked()  { startAnimation(AlgoType::BellmanFord); }
void VisShortest::onFloydWarshallClicked(){ startAnimation(AlgoType::FloydWarshall); }
void VisShortest::onResetClicked()        { buildGraph(); }
void VisShortest::onSpeedChanged(int) {
    if (m_timer->isActive())
        m_timer->setInterval(currentInterval());
}
void VisShortest::onPauseClicked() {
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

void VisShortest::paintEvent(QPaintEvent*) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing); p.fillRect(rect(), Qt::black);
    if(!m_graph) return;
    const int R = NODE_R;
    p.setPen(QPen(QColor(50,50,50),1));
    for(const auto& ed : m_graph->getEdges()) {
        QPointF a(ed.getFirst().getX(), ed.getFirst().getY());
        QPointF b(ed.getSecond().getX(), ed.getSecond().getY());
        QLineF ln(a,b); double angle=std::atan2(ln.dy(),ln.dx());
        QPointF perp(-3*std::sin(angle), 3*std::cos(angle));
        p.drawLine(a+perp, b+perp);
        QPointF mid=(a+b)/2.0+perp*2;
        p.save(); QFont f=p.font(); f.setPointSize(6); p.setFont(f); p.setPen(QColor(80,80,80));
        p.drawText(mid, QString::number(ed.getCost())); p.restore();
    }
    p.setPen(QPen(QColor(0,80,180),2));
    for(auto& [child,par] : m_parent) {
        QPointF a,b;
        for(const auto& n : m_graph->getNodes()) {
            if(n.getIndex()==par)   a=QPointF(n.getX(),n.getY());
            if(n.getIndex()==child) b=QPointF(n.getX(),n.getY());
        }
        p.drawLine(a,b);
    }
    if(m_animDone && !m_finalPath.empty()) {
        p.setPen(QPen(QColor(255,220,0),3));
        for(int i=0; i+1<static_cast<int>(m_finalPath.size()); i++) {
            QPointF a,b;
            for(const auto& n : m_graph->getNodes()) {
                if(n.getIndex()==m_finalPath[i])   a=QPointF(n.getX(),n.getY());
                if(n.getIndex()==m_finalPath[i+1]) b=QPointF(n.getX(),n.getY());
            }
            p.drawLine(a,b);
        }
    }
    for(const auto& n : m_graph->getNodes()) {
        int idx=n.getIndex(); QRect r(n.getX()-R, n.getY()-R, R*2, R*2);
        bool isStart=(idx==m_startIdx), isEnd=(idx==m_endIdx), isCurrent=(m_hasCurrent && idx==m_currentNode);
        bool isPath=false;
        if(m_animDone) for(int pi:m_finalPath) if(pi==idx){isPath=true;break;}
        bool explored=m_bestCost.count(idx);
        QColor fill,border;
        if(isStart)       {fill=QColor(0,200,80);  border=Qt::white;}
        else if(isEnd)    {fill=QColor(220,40,40); border=Qt::white;}
        else if(isCurrent){fill=QColor(255,200,0); border=Qt::yellow;}
        else if(isPath)   {fill=QColor(200,170,0); border=QColor(255,220,0);}
        else if(explored) {fill=QColor(20,60,140); border=QColor(60,120,220);}
        else              {fill=QColor(40,40,50);  border=QColor(80,80,100);}
        p.setBrush(fill); p.setPen(QPen(border, isStart||isEnd?2:1)); p.drawEllipse(r);
        p.setPen(Qt::white);
        QFont f=p.font();
        if(explored && !isStart && !isEnd) { f.setPointSize(5); f.setBold(true); p.setFont(f); p.drawText(r,Qt::AlignCenter,QString::number(m_bestCost[idx])); }
        else { f.setPointSize(6); f.setBold(false); p.setFont(f); p.drawText(r,Qt::AlignCenter,QString::number(idx)); }
    }
    /* Manual draw removed - using m_statusLabel */
}
