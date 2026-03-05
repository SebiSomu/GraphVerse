#include "vis_flow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <cmath>

// ── VisFlow ──

VisFlow::VisFlow(QWidget* parent) : QWidget(parent),
    m_network(new FlowNetwork()), m_networkWidget(new NetworkWidget(this))
{
    m_networkWidget->setNetwork(m_network);
    setupUI();
}
VisFlow::~VisFlow() { delete m_network; }

void VisFlow::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    auto* controls = new QHBoxLayout();
    m_modeGroup = new QButtonGroup(this);
    auto* rbAddNode   = new QRadioButton("Add Nodes", this);
    auto* rbAddEdge   = new QRadioButton("Add Edges", this);
    auto* rbSetSource = new QRadioButton("Set Source", this);
    auto* rbSetSink   = new QRadioButton("Set Sink", this);
    auto* rbView      = new QRadioButton("View (Algorithm)", this);
    for(auto* rb : {rbAddNode, rbAddEdge, rbSetSource, rbSetSink, rbView})
        rb->setStyleSheet("color:white;font-size:11px;");
    m_modeGroup->addButton(rbAddNode,0); m_modeGroup->addButton(rbAddEdge,1);
    m_modeGroup->addButton(rbSetSource,2); m_modeGroup->addButton(rbSetSink,3);
    m_modeGroup->addButton(rbView,4);
    rbAddNode->setChecked(true); m_networkWidget->setMode(NetworkWidget::InteractionMode::Mode_AddNode);
    controls->addWidget(rbAddNode); controls->addWidget(rbAddEdge);
    controls->addWidget(rbSetSource); controls->addWidget(rbSetSink); controls->addWidget(rbView);
    connect(m_modeGroup, QOverload<int>::of(&QButtonGroup::idClicked), this, &VisFlow::onModeChanged);
    auto* btnStart = new QPushButton("Start Algorithm", this);
    m_btnNextStep = new QPushButton("Next Step", this);
    auto* btnNegCyc = new QPushButton("Elim. Neg. Cycles", this);
    auto* btnReset = new QPushButton("Reset Result", this);
    auto* btnClear = new QPushButton("Clear All", this);
    m_btnNextStep->setEnabled(false);
    controls->addStretch();
    controls->addWidget(btnStart); controls->addWidget(m_btnNextStep);
    controls->addWidget(btnNegCyc); controls->addWidget(btnReset); controls->addWidget(btnClear);
    connect(btnStart, &QPushButton::clicked, this, &VisFlow::onStartAlgorithm);
    connect(m_btnNextStep, &QPushButton::clicked, this, &VisFlow::onNextStep);
    connect(btnNegCyc, &QPushButton::clicked, this, &VisFlow::onEliminateNegCycles);
    connect(btnReset, &QPushButton::clicked, this, &VisFlow::onReset);
    connect(btnClear, &QPushButton::clicked, this, &VisFlow::onClear);
    mainLayout->addLayout(controls);
    m_statusLabel = new QLabel("Build the network, then press 'Start Algorithm'.", this);
    m_statusLabel->setStyleSheet("font-weight:bold;padding:5px;background-color:#555;border:1px solid #777;color:white;");
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(m_networkWidget);
}

void VisFlow::onModeChanged(int id) {
    if      (id==0) m_networkWidget->setMode(NetworkWidget::InteractionMode::Mode_AddNode);
    else if (id==1) m_networkWidget->setMode(NetworkWidget::InteractionMode::Mode_AddEdge);
    else if (id==2) m_networkWidget->setMode(NetworkWidget::InteractionMode::Mode_SetSource);
    else if (id==3) m_networkWidget->setMode(NetworkWidget::InteractionMode::Mode_SetSink);
    else if (id==4) m_networkWidget->setMode(NetworkWidget::InteractionMode::Mode_View);
}

void VisFlow::onStartAlgorithm() {
    if (m_network->getNumNodes() < 2) { QMessageBox::warning(this,"Error","Graph is too small!"); return; }
    if (m_network->getSource()==-1||m_network->getSink()==-1) { QMessageBox::warning(this,"Error","Source or Sink not set!"); return; }
    m_network->initializeAlgorithm(); m_networkWidget->setIterationIndex(0);
    m_statusLabel->setText("Algorithm initialized. Press 'Next Step'.");
    m_btnNextStep->setEnabled(true);
    m_modeGroup->button(4)->setChecked(true); m_networkWidget->setMode(NetworkWidget::InteractionMode::Mode_View);
}

void VisFlow::onNextStep() {
    bool hasPath = m_network->performStep();
    if (hasPath) {
        const auto& it = m_network->getIterations();
        m_networkWidget->setIterationIndex(it.size()-1); m_statusLabel->setText(it.back().description);
    } else {
        m_network->finalizeMinCut(); m_network->addFinalState();
        const auto& it = m_network->getIterations();
        m_networkWidget->setIterationIndex(it.size()-1);
        m_statusLabel->setText(QString("Algorithm finished! Max Flow: %1").arg(it.back().totalFlow));
        m_btnNextStep->setEnabled(false);
    }
}

void VisFlow::onEliminateNegCycles() {
    if (m_network->getNumNodes()<2) { QMessageBox::warning(this,"Error","Graph is too small!"); return; }
    int cycles = m_network->eliminateNegativeCycles();
    if (cycles==0) { m_statusLabel->setText("No negative cycles found."); }
    else {
        const auto& it = m_network->getIterations();
        m_networkWidget->setIterationIndex(it.size()-1);
        m_networkWidget->setMode(NetworkWidget::InteractionMode::Mode_View); m_modeGroup->button(4)->setChecked(true);
        m_statusLabel->setText(QString("Eliminated %1 negative cycle(s).").arg(cycles));
    }
    m_networkWidget->update();
}

void VisFlow::onReset() {
    m_network->resetFlow(); m_networkWidget->setIterationIndex(-1);
    m_statusLabel->setText("Results reset."); m_btnNextStep->setEnabled(false);
}

void VisFlow::onClear() {
    m_network->clear(); m_networkWidget->setIterationIndex(-1);
    m_statusLabel->setText("Network cleared."); m_btnNextStep->setEnabled(false); m_networkWidget->update();
}

// ── NetworkWidget ──

NetworkWidget::NetworkWidget(QWidget* parent)
    : QWidget(parent), m_network(nullptr), m_mode(InteractionMode::Mode_View),
      m_iterationIndex(-1), m_showFinalResult(false), m_firstNode(-1), m_isDragging(false)
{
    setMinimumSize(800,600); setStyleSheet("background-color:#2d2d2d;"); setMouseTracking(true);
}

void NetworkWidget::setNetwork(FlowNetwork* n) { m_network=n; update(); }
void NetworkWidget::setMode(InteractionMode m) { m_mode=m; m_firstNode=-1; m_isDragging=false; update(); }
void NetworkWidget::setIterationIndex(int i) { m_iterationIndex=i; update(); }

int NetworkWidget::getNodeAt(const QPoint& pos) {
    if (!m_network) return -1;
    const auto& positions = m_network->getNodePositions();
    for (size_t i=0;i<positions.size();i++)
        if (QLineF(pos,positions[i]).length() < NODE_RADIUS) return i+1;
    return -1;
}

void NetworkWidget::mousePressEvent(QMouseEvent* event) {
    if (!m_network) return;
    if (m_mode== InteractionMode::Mode_AddNode) { if(event->button()==Qt::LeftButton) { m_network->addNode(event->pos()); update(); } }
    else if (m_mode== InteractionMode::Mode_AddEdge) {
        if(event->button()==Qt::LeftButton) {
            int node=getNodeAt(event->pos());
            if(node!=-1) { m_firstNode=node; m_isDragging=true; m_tempMousePos=event->pos(); }
        }
    } else if (m_mode== InteractionMode::Mode_SetSource) {
        int node=getNodeAt(event->pos()); if(node!=-1) { m_network->setSource(node); update(); }
    } else if (m_mode== InteractionMode::Mode_SetSink) {
        int node=getNodeAt(event->pos()); if(node!=-1) { m_network->setSink(node); update(); }
    }
}

void NetworkWidget::mouseMoveEvent(QMouseEvent* e) { if(m_isDragging) { m_tempMousePos=e->pos(); update(); } }

void NetworkWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (m_mode== InteractionMode::Mode_AddEdge && m_isDragging) {
        int targetNode=getNodeAt(event->pos());
        if (targetNode!=-1 && m_firstNode!=-1 && targetNode!=m_firstNode) {
            if (m_network->getCapacity(m_firstNode,targetNode)>0) {
                QMessageBox::warning(this,"Warning","Edge already exists!");
            } else {
                bool ok;
                int cap=QInputDialog::getInt(this,"Capacity",
                    QString("Edge capacity %1 -> %2 (must be > 0):").arg(m_firstNode).arg(targetNode), 1,1,10000,1,&ok);
                if(ok) {
                    m_network->addEdge(m_firstNode,targetNode,cap);
                    bool okCost;
                    int cost=QInputDialog::getInt(this,"Cost",
                        QString("Edge cost %1 -> %2 (can be negative):").arg(m_firstNode).arg(targetNode), 0,-10000,10000,1,&okCost);
                    if(okCost) m_network->setEdgeCost(m_firstNode,targetNode,cost);
                }
            }
        }
        m_firstNode=-1; m_isDragging=false; update();
    }
}

void NetworkWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this); painter.setRenderHint(QPainter::Antialiasing);
    if(!m_network) return;
    const auto& iterations = m_network->getIterations();
    bool displayingIter = (m_iterationIndex>=0 && m_iterationIndex<(int)iterations.size());
    const std::vector<std::vector<int>>* residualMatrix = nullptr;
    std::set<std::pair<int,int>> augmentEdges;
    if(displayingIter) {
        const auto& state = iterations[m_iterationIndex];
        residualMatrix = &state.residualCapacity;
        const auto& path = state.augmentingPath;
        if(!path.empty()) for(size_t i=0;i<path.size()-1;i++) augmentEdges.insert({path[i],path[i+1]});
    }
    const auto& positions = m_network->getNodePositions();
    int n = positions.size();
    for(int i=1;i<=n;i++) for(int j=1;j<=n;j++) {
        bool isForward = (augmentEdges.count({i,j})>0);
        bool isBackward = (augmentEdges.count({j,i})>0);
        if(isBackward) continue;
        int capacity = m_network->getCapacity(i,j);
        bool isReal = (capacity>0); if(!isReal && !isForward) continue;
        bool hasBoth = (m_network->getCapacity(i,j)>0 && m_network->getCapacity(j,i)>0);
        bool isReverse = hasBoth && (i>j);
        int residual = 0;
        if(displayingIter) {
            if(residualMatrix && i-1<(int)residualMatrix->size() && j-1<(int)(*residualMatrix)[i-1].size())
                residual = (*residualMatrix)[i-1][j-1];
            if(isForward) drawEdge(painter, positions[i-1], positions[j-1], capacity, residual, m_network->getEdgeCost(i,j), false, true, isReverse);
            else if(isReal) {
                bool isMinCut = (m_iterationIndex==(int)iterations.size()-1) && m_network->isMinCutEdge(i,j);
                drawEdge(painter, positions[i-1], positions[j-1], capacity, residual, m_network->getEdgeCost(i,j), isMinCut, false, isReverse);
            }
        } else {
            if(isReal) drawEdge(painter, positions[i-1], positions[j-1], capacity, -1, m_network->getEdgeCost(i,j), false, false, isReverse);
        }
    }
    if(m_isDragging && m_firstNode!=-1) {
        painter.setPen(QPen(Qt::white,2,Qt::DashLine));
        painter.drawLine(positions[m_firstNode-1], m_tempMousePos);
    }
    for(int i=0;i<n;i++) {
        bool isSource = ((i+1)==m_network->getSource());
        bool isSink = ((i+1)==m_network->getSink());
        drawNode(painter, i+1, positions[i], isSource, isSink);
    }
}

void NetworkWidget::drawNode(QPainter& painter, int index, const QPointF& pos, bool isSource, bool isSink) {
    QColor fillColor(144,238,144), borderColor(34,139,34); int borderWidth=3;
    if(isSource) { fillColor=QColor(100,255,100); borderColor=QColor(0,100,0); borderWidth=5; }
    if(isSink)   { fillColor=QColor(255,150,150); borderColor=QColor(200,0,0); borderWidth=5; }
    painter.setPen(QPen(borderColor,borderWidth)); painter.setBrush(fillColor);
    painter.drawEllipse(pos, NODE_RADIUS, NODE_RADIUS);
    painter.setPen(Qt::black);
    QFont font=painter.font(); font.setPointSize(10); font.setBold(true); painter.setFont(font);
    painter.drawText(QRectF(pos.x()-NODE_RADIUS, pos.y()-NODE_RADIUS, NODE_RADIUS*2, NODE_RADIUS*2),
                     Qt::AlignCenter, QString::number(index));
}

void NetworkWidget::drawEdge(QPainter& painter, const QPointF& from, const QPointF& to,
                             int capacity, int residual, int cost, bool isMinCut, bool isOnPath, bool) {
    QLineF line(from,to); double angle=std::atan2(line.dy(),line.dx());
    QPointF start = from+QPointF(NODE_RADIUS*cos(angle), NODE_RADIUS*sin(angle));
    QPointF end = to-QPointF((NODE_RADIUS+5)*cos(angle), (NODE_RADIUS+5)*sin(angle));
    QColor color(100,149,237); int width=2;
    if(isMinCut) { color=Qt::red; width=4; } else if(isOnPath) { color=QColor(255,140,0); width=3; }
    painter.setPen(QPen(color,width)); painter.drawLine(start,end);
    QPointF p1=end-QPointF(ARROW_SIZE*cos(angle-M_PI/6), ARROW_SIZE*sin(angle-M_PI/6));
    QPointF p2=end-QPointF(ARROW_SIZE*cos(angle+M_PI/6), ARROW_SIZE*sin(angle+M_PI/6));
    QPolygonF arrowHead; arrowHead<<end<<p1<<p2; painter.setBrush(color); painter.drawPolygon(arrowHead);
    if(residual>=0 || capacity>0) {
        QString text;
        if(residual==-1) text = cost!=0 ? QString("%1 [c:%2]").arg(capacity).arg(cost) : QString::number(capacity);
        else text = cost!=0 ? QString("%1 [c:%2]").arg(residual).arg(cost) : QString::number(residual);
        QPointF mid=(start+end)/2.0; QPointF perp(-std::sin(angle)*14, std::cos(angle)*14); mid+=perp;
        QFont font=painter.font(); font.setPointSize(10); painter.setFont(font);
        QFontMetrics fm(font);
        int uw=fm.horizontalAdvance(text)+10, uh=fm.height()+4;
        QRectF labelRect(mid.x()-uw/2, mid.y()-uh/2, uw, uh);
        painter.setPen(Qt::black); painter.setBrush(Qt::white); painter.drawRect(labelRect);
        painter.drawText(labelRect, Qt::AlignCenter, text);
    }
}
