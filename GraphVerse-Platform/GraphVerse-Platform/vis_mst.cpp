#include "vis_mst.h"
#include <QPainter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <vector>
#include <string>
#include <random>
#include <ctime>
#include <cmath>

VisMST::VisMST(QWidget* parent)
    : QWidget(parent), m_graph(nullptr), m_timer(new QTimer(this)),
      m_statusLabel(new QLabel(this)), m_timerLabel(new QLabel(this)),
      m_speedLabel(new QLabel("Speed", this)), m_pauseBtn(new QPushButton("Pause", this)),
      m_speedSlider(new QSlider(Qt::Horizontal, this)), m_toolbar(nullptr),
      m_stepIdx(0), m_animDone(false), m_hasCurrent(false), m_currentAlgo(AlgoType::None)
{
    setMinimumSize(COLS*SPACING+offsetX()*2, ROWS*SPACING+70+90);
    auto* btnK = new QPushButton("Kruskal", this);
    auto* btnP = new QPushButton("Prim", this);
    auto* btnB = new QPushButton("Boruvka", this);
    auto* btnR = new QPushButton("Reset", this);
    m_pauseBtn->setEnabled(false);
    m_speedSlider->setRange(1, 100);
    m_speedSlider->setValue(70);
    m_speedSlider->setFixedWidth(180);
    m_paused = false;
    m_elapsedMs = 0;
    for(auto* b : {btnK,btnP,btnB,btnR,m_pauseBtn}) { b->setFixedHeight(34); b->setFixedWidth(110); }
    QString base = "QPushButton{border-radius:6px;font-weight:bold;font-size:12px;}";
    btnK->setStyleSheet(base+"QPushButton{background:#1e90ff;color:white;}QPushButton:hover{background:#1070d0;}");
    btnP->setStyleSheet(base+"QPushButton{background:#27ae60;color:white;}QPushButton:hover{background:#1e8449;}");
    btnB->setStyleSheet(base+"QPushButton{background:#9b59b6;color:white;}QPushButton:hover{background:#7d3c98;}");
    btnR->setStyleSheet(base+"QPushButton{background:#e74c3c;color:white;}QPushButton:hover{background:#c0392b;}");
    m_pauseBtn->setStyleSheet(base+"QPushButton{background:#6b7280;color:white;}QPushButton:hover{background:#4b5563;}");
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

    auto* lbl = new QLabel("Green = added to MST   Red = rejected   White = final MST", this);
    lbl->setStyleSheet("color:#aaaaaa;font-size:11px;");
    m_toolbar = new QWidget(this);
    auto* hbox = new QHBoxLayout(m_toolbar);
    hbox->addWidget(btnK); hbox->addWidget(btnP); hbox->addWidget(btnB);
    hbox->addSpacing(16); hbox->addWidget(btnR); hbox->addWidget(m_pauseBtn);
    hbox->addSpacing(8); hbox->addWidget(m_speedLabel); hbox->addWidget(m_speedSlider);
    hbox->addStretch(1);
    hbox->addWidget(m_statusLabel, 3);
    hbox->addStretch(1);
    hbox->addWidget(m_timerLabel);
    hbox->addSpacing(12);
    hbox->addWidget(lbl);
    hbox->setContentsMargins(8,6,8,4);
    m_toolbar->setGeometry(0,0,width(),50);
    connect(btnK, &QPushButton::clicked, this, &VisMST::onKruskalClicked);
    connect(btnP, &QPushButton::clicked, this, &VisMST::onPrimClicked);
    connect(btnB, &QPushButton::clicked, this, &VisMST::onBoruvkaClicked);
    connect(btnR, &QPushButton::clicked, this, &VisMST::onResetClicked);
    connect(m_pauseBtn, &QPushButton::clicked, this, &VisMST::onPauseClicked);
    connect(m_speedSlider, &QSlider::valueChanged, this, &VisMST::onSpeedChanged);
    connect(m_timer, &QTimer::timeout, this, &VisMST::onAnimationTick);
    srand(static_cast<unsigned>(time(nullptr)));
    buildGraph();
}

VisMST::~VisMST() { delete m_graph; }

void VisMST::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    if (m_toolbar)
        m_toolbar->setGeometry(0, 0, width(), 50);
}

void VisMST::buildGraph() {
    m_timer->stop(); delete m_graph; m_graph = new UndirectedGraph();
    m_steps.clear(); m_accepted.clear(); m_rejected.clear();
    m_hasCurrent = false; m_stepIdx = 0; m_animDone = false; m_currentAlgo = AlgoType::None;
    m_paused = false;
    m_pauseBtn->setText("Pause");
    m_pauseBtn->setEnabled(false);
    m_elapsedMs = 0;
    updateTimerLabel();
    m_statusLabel->clear();
    updateLabel();
    std::mt19937 rng(static_cast<unsigned>(time(nullptr)));
    std::uniform_int_distribution<int> costDist(1, 99);
    const int ox = offsetX(), oy = offsetY();
    for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++) m_graph->addNode(QPoint(ox+c*SPACING, oy+r*SPACING));
    
    std::vector<Node*> nodes;
    for(auto& n : m_graph->getNodes()) nodes.push_back(&n);
    
    auto idx = [&](int r, int c){ return r*COLS+c; };
    for(int r=0;r<ROWS;r++) for(int c=0;c<COLS-1;c++) m_graph->addEdge(*nodes[idx(r,c)], *nodes[idx(r,c+1)], costDist(rng));
    for(int r=0;r<ROWS-1;r++) for(int c=0;c<COLS;c++) m_graph->addEdge(*nodes[idx(r,c)], *nodes[idx(r+1,c)], costDist(rng));
    update();
}

void VisMST::startAnimation(AlgoType algo) {
    m_timer->stop(); m_currentAlgo = algo;
    m_accepted.clear(); m_rejected.clear(); m_hasCurrent = false; m_stepIdx = 0; m_animDone = false;
    m_paused = false;
    m_pauseBtn->setText("Pause");
    m_pauseBtn->setEnabled(true);
    m_elapsedMs = 0;
    m_elapsedClock.restart();
    updateTimerLabel();
    m_statusLabel->setText("Initializing...");
    if(algo==AlgoType::Kruskal) m_steps=m_graph->kruskal();
    else if(algo==AlgoType::Prim) m_steps=m_graph->prim();
    else m_steps=m_graph->boruvka();
    m_timer->start(currentInterval());
}

void VisMST::onAnimationTick() {
    if(m_stepIdx >= static_cast<int>(m_steps.size())) { 
        m_elapsedMs += m_elapsedClock.elapsed();
        m_timer->stop(); m_hasCurrent=false; m_animDone=true; 
        m_pauseBtn->setEnabled(false);
        updateTimerLabel();
        updateLabel();
        update(); return; 
    }
    m_current = m_steps[m_stepIdx++]; m_hasCurrent = true;
    if(m_current.accepted) m_accepted.push_back(m_current); else m_rejected.push_back(m_current);
    updateLabel();
    updateTimerLabel();
    update();
}

void VisMST::updateLabel() {
    if(m_currentAlgo == AlgoType::None) { m_statusLabel->clear(); return; }
    QString algoName; QColor algoCol;
    if(m_currentAlgo==AlgoType::Kruskal) { algoName="Kruskal"; algoCol=QColor(80,160,255); }
    else if(m_currentAlgo==AlgoType::Prim) { algoName="Prim"; algoCol=QColor(46,204,113); }
    else { algoName="Boruvka"; algoCol=QColor(175,122,197); }
    
    int totalCost=0; for(const auto& s:m_accepted) totalCost+=s.cost;
    QString status = m_animDone ? QString("%1: Done! Total cost: %2").arg(algoName).arg(totalCost)
                                : QString("%1: Building... (%2 edges)").arg(algoName).arg(static_cast<int>(m_accepted.size()));
    m_statusLabel->setText(status);
    m_statusLabel->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: bold;").arg(algoCol.name()));
}

void VisMST::updateTimerLabel() {
    qint64 totalMs = m_elapsedMs;
    if (!m_paused && m_timer->isActive())
        totalMs += m_elapsedClock.elapsed();
    double seconds = static_cast<double>(totalMs) / 1000.0;
    m_timerLabel->setText(QString("Time: %1s").arg(seconds, 0, 'f', 2));
}

int VisMST::currentInterval() const {
    int slider = m_speedSlider ? m_speedSlider->value() : 70;
    int minMs = 2;
    int maxMs = 120;
    int slow = 101 - slider;
    int range = maxMs - minMs;
    return minMs + (slow * slow * range) / 10000;
}

void VisMST::onKruskalClicked() { startAnimation(AlgoType::Kruskal); }
void VisMST::onPrimClicked() { startAnimation(AlgoType::Prim); }
void VisMST::onBoruvkaClicked() { startAnimation(AlgoType::Boruvka); }
void VisMST::onResetClicked() { buildGraph(); }
void VisMST::onSpeedChanged(int) {
    if (m_timer->isActive())
        m_timer->setInterval(currentInterval());
}
void VisMST::onPauseClicked() {
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

void VisMST::paintEvent(QPaintEvent*) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing); p.fillRect(rect(), Qt::black);
    const int R = NODE_R;
    auto nodePos = [&](int idx) -> QPointF {
        for(const auto& n : m_graph->getNodes()) if(n.getIndex()==idx) return QPointF(n.getX(), n.getY());
        return QPointF(0,0);
    };
    auto drawEd = [&](const MSTStep& s, QColor col, int width, bool showCost) {
        QPointF a=nodePos(s.fromIndex), b=nodePos(s.toIndex);
        p.setPen(QPen(col,width)); p.drawLine(a,b);
        if(showCost) {
            QPointF mid=(a+b)/2.0; QLineF ln(a,b); double angle=std::atan2(ln.dy(),ln.dx());
            QPointF off(-10*std::sin(angle), 10*std::cos(angle));
            p.save(); QFont f=p.font(); f.setPointSize(7); f.setBold(true); p.setFont(f);
            p.setPen(col); p.drawText(mid+off, QString::number(s.cost)); p.restore();
        }
    };
    for(const auto& ed : m_graph->getEdges()) {
        MSTStep s{ed.getFirst().getIndex(), ed.getSecond().getIndex(), ed.getCost(), false};
        drawEd(s, QColor(60,60,60), 1, false);
    }
    for(const auto& s : m_rejected) drawEd(s, QColor(160,30,30), 1, false);
    QColor mstCol = m_animDone ? Qt::white : QColor(0,220,80);
    for(const auto& s : m_accepted) drawEd(s, mstCol, 2, true);
    if(m_hasCurrent && !m_animDone) {
        QColor cur = m_current.accepted ? QColor(0,255,100) : QColor(255,60,60);
        drawEd(m_current, cur, 3, true);
    }
    for(const auto& n : m_graph->getNodes()) {
        QRect r(n.getX()-R, n.getY()-R, R*2, R*2);
        bool inMST = false;
        for(const auto& s : m_accepted) if(s.fromIndex==n.getIndex()||s.toIndex==n.getIndex()) { inMST=true; break; }
        QColor fill = inMST ? (m_animDone ? QColor(240,240,240) : QColor(0,180,60)) : QColor(50,50,60);
        QColor border = inMST ? (m_animDone ? Qt::white : QColor(0,220,80)) : QColor(130,130,150);
        p.setBrush(fill); p.setPen(QPen(border,1)); p.drawEllipse(r);
        p.setPen(inMST && m_animDone ? Qt::black : Qt::white);
        QFont f=p.font(); f.setPointSize(6); p.setFont(f);
        p.drawText(r, Qt::AlignCenter, QString::number(n.getIndex()));
    }
}
