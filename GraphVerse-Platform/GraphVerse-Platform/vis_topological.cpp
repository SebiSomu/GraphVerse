#include "vis_topological.h"
#include "algorithms/topological_sort_solver.h"
#include <QPainter>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCursor>
#include <cmath>

VisTopologicalSort::VisTopologicalSort(QWidget* parent) : QWidget(parent), m_graph(nullptr), m_firstNode(nullptr), m_draggedNode(nullptr) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    
    // Result label (empty initially)
    m_resultLabel = new QLabel("", this);
    m_resultLabel->setStyleSheet("color: #aaaaaa; font-size: 16px; font-weight: bold; padding: 10px;");
    m_resultLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_resultLabel);
    
    // Buttons
    auto* btnLayout = new QHBoxLayout();
    m_btnSort = new QPushButton("🔀 Topological Sort", this);
    m_btnClear = new QPushButton("🗑️ Clear", this);
    m_btnSort->setStyleSheet("QPushButton { background-color: #3b82f6; color: white; padding: 8px 16px; border-radius: 6px; }");
    m_btnClear->setStyleSheet("QPushButton { background-color: #ef4444; color: white; padding: 8px 16px; border-radius: 6px; }");
    btnLayout->addWidget(m_btnSort);
    btnLayout->addWidget(m_btnClear);
    layout->addLayout(btnLayout);
    
    // Add stretch so canvas has space to draw
    layout->addStretch(1);
    
    connect(m_btnSort, &QPushButton::clicked, this, &VisTopologicalSort::onSortClicked);
    connect(m_btnClear, &QPushButton::clicked, this, &VisTopologicalSort::onClearClicked);
    
    m_graph = std::make_unique<DirectedGraph>();
    setLayout(layout);
    setMinimumSize(800, 600);
}

VisTopologicalSort::~VisTopologicalSort() {}

void VisTopologicalSort::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
}

void VisTopologicalSort::mousePressEvent(QMouseEvent* event) {
    if (!m_graph) return;
    
    m_pressPosition = event->pos();
    m_dragging = false;
    m_hasMoved = false;
    
    if (event->button() == Qt::LeftButton) {
        // Check if clicked on a node
        for (auto& node : m_graph->getNodes()) {
            QPoint nodePos = node.getCoord();
            int dx = event->pos().x() - nodePos.x();
            int dy = event->pos().y() - nodePos.y();
            if (dx*dx + dy*dy <= 30*30) {
                m_draggedNode = &node;
                m_dragging = true;
                return;
            }
        }
    }
}

void VisTopologicalSort::mouseMoveEvent(QMouseEvent* event) {
    if (m_dragging && m_draggedNode) {
        QPoint newPos = event->pos();
        m_draggedNode->setCoord(newPos);
        m_hasMoved = true;
        update();
    }
}

void VisTopologicalSort::mouseReleaseEvent(QMouseEvent* event) {
    if (!m_graph) return;
    
    if (event->button() == Qt::RightButton) {
        // Add new node
        m_graph->addNode(event->pos());
        update();
        return;
    }
    
    if (event->button() == Qt::LeftButton) {
        if (m_dragging && m_draggedNode) {
            if (!m_hasMoved) {
                // Click on node - add edge
                if (m_firstNode == nullptr) {
                    m_firstNode = m_draggedNode;
                    update();
                } else if (m_firstNode != m_draggedNode) {
                    m_graph->addEdge(*m_firstNode, *m_draggedNode, 1);
                    m_firstNode = nullptr;
                    update();
                } else {
                    m_firstNode = nullptr;
                    update();
                }
            }
        }
    }
    
    m_dragging = false;
    m_draggedNode = nullptr;
}

void VisTopologicalSort::onSortClicked() {
    if (!m_graph || m_graph->getNodes().empty()) return;
    
    TopologicalSortSolver solver;
    
    // Check for cycle
    if (solver.hasCycle(*m_graph)) {
        m_resultLabel->setText("❌ Cycle detected! Cannot sort.");
        m_resultLabel->setStyleSheet("color: #ef4444; font-size: 16px; font-weight: bold; padding: 10px;");
        return;
    }
    
    // Get sorted order
    m_sortedOrder.clear();
    solver.solve(*m_graph, m_sortedOrder);
    
    // Build order text
    QString orderText = "📋 Order: ";
    for (size_t i = 0; i < m_sortedOrder.size(); ++i) {
        if (i > 0) orderText += " → ";
        orderText += QString::number(m_sortedOrder[i]);
    }
    
    m_resultLabel->setText(orderText);
    m_resultLabel->setStyleSheet("color: #22c55e; font-size: 16px; font-weight: bold; padding: 10px;");
}

void VisTopologicalSort::onClearClicked() {
    m_graph->clear();
    m_firstNode = nullptr;
    m_sortedOrder.clear();
    m_resultLabel->clear();
    update();
}

void VisTopologicalSort::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    
    if (!m_graph) return;
    
    // Draw edges
    for (const auto& edge : m_graph->getEdges()) {
        QPoint from = edge.getFirst().getCoord();
        QPoint to = edge.getSecond().getCoord();
        
        // Draw arrow
        p.setPen(QPen(QColor(140, 148, 190), 2));
        p.drawLine(from, to);
        
        // Draw arrowhead
        double angle = atan2(to.y() - from.y(), to.x() - from.x());
        int arrowSize = 10;
        QPoint arrowTip(
            to.x() - (int)(arrowSize * cos(angle)),
            to.y() - (int)(arrowSize * sin(angle))
        );
        QPoint arrowLeft(
            arrowTip.x() + (int)(arrowSize * cos(angle + 2.5)),
            arrowTip.y() + (int)(arrowSize * sin(angle + 2.5))
        );
        QPoint arrowRight(
            arrowTip.x() + (int)(arrowSize * cos(angle - 2.5)),
            arrowTip.y() + (int)(arrowSize * sin(angle - 2.5))
        );
        
        p.drawLine(to, arrowLeft);
        p.drawLine(to, arrowRight);
    }
    
    // Draw pending edge
    if (m_firstNode) {
        p.setPen(QPen(Qt::yellow, 2, Qt::DashLine));
        p.drawLine(m_firstNode->getCoord(), QCursor::pos() - mapToGlobal(QPoint(0,0)) + m_firstNode->getCoord());
    }
    
    // Draw nodes (simple, no coloring)
    for (auto& node : m_graph->getNodes()) {
        QPoint pos = node.getCoord();
        int idx = node.getIndex();
        
        // Highlight first selected node
        QColor borderColor = (&node == m_firstNode) ? Qt::yellow : QColor(45, 212, 191);
        int penWidth = (&node == m_firstNode) ? 4 : 2;
        
        p.setBrush(QColor(20, 184, 166)); // Teal default
        p.setPen(QPen(borderColor, penWidth));
        p.drawEllipse(pos, 22, 22);
        
        // Draw index
        p.setPen(Qt::white);
        QFont f = p.font();
        f.setPointSize(10);
        f.setBold(true);
        p.setFont(f);
        p.drawText(QRect(pos.x() - 22, pos.y() - 22, 44, 44), Qt::AlignCenter, QString::number(idx));
    }
    
    // Draw "Drag to move" hint at bottom
    p.setPen(QColor(170, 170, 170));
    QFont hintFont = p.font();
    hintFont.setPointSize(9);
    p.setFont(hintFont);
    p.drawText(10, height() - 10, "Drag to move • Click to select edge start/end • Right-click to add node");
}
