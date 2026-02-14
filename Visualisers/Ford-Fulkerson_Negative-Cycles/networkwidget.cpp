#include "networkwidget.h"
#include <QMouseEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

NetworkWidget::NetworkWidget(QWidget *parent)
    : QWidget(parent)
    , m_network(nullptr)
    , m_mode(Mode_View)
    , m_iterationIndex(-1)
    , m_showFinalResult(false)
    , m_firstNode(-1)
    , m_isDragging(false)
{
    setMinimumSize(800, 600);
    setStyleSheet("background-color: #2d2d2d;");
    setMouseTracking(true);
}

void NetworkWidget::setNetwork(FlowNetwork* network) {
    m_network = network;
    update();
}

void NetworkWidget::setMode(InteractionMode mode) {
    m_mode = mode;
    m_firstNode = -1;
    m_isDragging = false;
    update();
}

void NetworkWidget::setIterationIndex(int index) {
    m_iterationIndex = index;
    update();
}

int NetworkWidget::getNodeAt(const QPoint& pos) {
    if (!m_network) return -1;

    const auto& positions = m_network->getNodePositions();
    for (size_t i = 0; i < positions.size(); i++) {
        QPointF nodePos = positions[i];
        if (QLineF(pos, nodePos).length() < NODE_RADIUS) {
            return i + 1;
        }
    }
    return -1;
}

void NetworkWidget::mousePressEvent(QMouseEvent *event) {
    if (!m_network) return;

    if (m_mode == Mode_AddNode) {
        if (event->button() == Qt::LeftButton) {
            m_network->addNode(event->pos());
            update();
        }
    } else if (m_mode == Mode_AddEdge) {
        if (event->button() == Qt::LeftButton) {
            int node = getNodeAt(event->pos());
            if (node != -1) {
                m_firstNode = node;
                m_isDragging = true;
                m_tempMousePos = event->pos();
            }
        }
    } else if (m_mode == Mode_SetSource) {
        int node = getNodeAt(event->pos());
        if (node != -1) {
            m_network->setSource(node);
            update();
        }
    } else if (m_mode == Mode_SetSink) {
        int node = getNodeAt(event->pos());
        if (node != -1) {
            m_network->setSink(node);
            update();
        }
    }
}

void NetworkWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_isDragging) {
        m_tempMousePos = event->pos();
        update();
    }
}

void NetworkWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (m_mode == Mode_AddEdge && m_isDragging) {
        int targetNode = getNodeAt(event->pos());
        if (targetNode != -1 && m_firstNode != -1 && targetNode != m_firstNode) {
            if (m_network->getCapacity(m_firstNode, targetNode) > 0) {
                QMessageBox::warning(this, "Warning", "Edge already exists!");
            } else {
                bool ok;
                int capacity = QInputDialog::getInt(this, "Capacity",
                                                    QString("Edge capacity %1 -> %2 (must be > 0):").arg(m_firstNode).arg(targetNode),
                                                    1, 1, 10000, 1, &ok);
                if (ok) {
                    m_network->addEdge(m_firstNode, targetNode, capacity);
                    bool okCost;
                    int cost = QInputDialog::getInt(this, "Cost",
                                                    QString("Edge cost %1 -> %2 (can be negative):").arg(m_firstNode).arg(targetNode),
                                                    0, -10000, 10000, 1, &okCost);
                    if (okCost)
                        m_network->setEdgeCost(m_firstNode, targetNode, cost);
                }
            }
        }
        m_firstNode = -1;
        m_isDragging = false;
        update();
    }
}

void NetworkWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (!m_network) return;

    const auto& iterations = m_network->getIterations();
    bool displayingIteration = (m_iterationIndex >= 0 && m_iterationIndex < (int)iterations.size());

    const std::vector<std::vector<int>>* residualMatrix = nullptr;
    std::set<std::pair<int, int>> augmentEdges;

    if (displayingIteration) {
        const auto& state = iterations[m_iterationIndex];
        residualMatrix = &state.residualCapacity;

        const auto& path = state.augmentingPath;
        if (!path.empty()) {
            for (size_t i = 0; i < path.size() - 1; i++) {
                augmentEdges.insert({path[i], path[i+1]});
            }
        }
    }

    const auto& positions = m_network->getNodePositions();
    int n = positions.size();

    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++) {
            bool isForwardPath = (augmentEdges.count({i, j}) > 0);
            bool isBackwardPath = (augmentEdges.count({j, i}) > 0);

            if (isBackwardPath) continue;

            int capacity = m_network->getCapacity(i, j);
            bool isRealEdge = (capacity > 0);
            if (!isRealEdge && !isForwardPath) continue;

            bool hasBoth = (m_network->getCapacity(i, j) > 0 && m_network->getCapacity(j, i) > 0);
            bool isReverse = hasBoth && (i > j);

            int residual = 0;
            if (displayingIteration) {
                if (residualMatrix && i-1 < (int)residualMatrix->size() && j-1 < (int)(*residualMatrix)[i-1].size()) {
                    residual = (*residualMatrix)[i-1][j-1];
                }
                if (isForwardPath) {
                    drawEdge(painter, positions[i-1], positions[j-1], capacity, residual, m_network->getEdgeCost(i,j), false, true, isReverse);
                } else if (isRealEdge) {
                    bool isMinCut = (m_iterationIndex == (int)iterations.size() - 1) && m_network->isMinCutEdge(i, j);
                    drawEdge(painter, positions[i-1], positions[j-1], capacity, residual, m_network->getEdgeCost(i,j), isMinCut, false, isReverse);
                }
            } else {
                if (isRealEdge) {
                    drawEdge(painter, positions[i-1], positions[j-1], capacity, -1, m_network->getEdgeCost(i,j), false, false, isReverse);
                }
            }
        }
    }

    if (m_isDragging && m_firstNode != -1) {
        painter.setPen(QPen(Qt::white, 2, Qt::DashLine));
        painter.drawLine(positions[m_firstNode-1], m_tempMousePos);
    }

    for (int i = 0; i < n; i++) {
        bool isSource = ((i + 1) == m_network->getSource());
        bool isSink = ((i + 1) == m_network->getSink());
        drawNode(painter, i + 1, positions[i], isSource, isSink);
    }
}

void NetworkWidget::drawNode(QPainter& painter, int index, const QPointF& pos, bool isSource, bool isSink)
{
    QColor fillColor = QColor(144, 238, 144);
    QColor borderColor = QColor(34, 139, 34);
    int borderWidth = 3;

    if (isSource) {
        fillColor = QColor(100, 255, 100);
        borderColor = QColor(0, 100, 0);
        borderWidth = 5;
    }
    if (isSink) {
        fillColor = QColor(255, 150, 150);
        borderColor = QColor(200, 0, 0);
        borderWidth = 5;
    }

    painter.setPen(QPen(borderColor, borderWidth));
    painter.setBrush(fillColor);
    painter.drawEllipse(pos, NODE_RADIUS, NODE_RADIUS);

    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setPointSize(10);
    font.setBold(true);
    painter.setFont(font);

    painter.drawText(QRectF(pos.x() - NODE_RADIUS, pos.y() - NODE_RADIUS,
                            NODE_RADIUS * 2, NODE_RADIUS * 2),
                     Qt::AlignCenter, QString::number(index));
}

void NetworkWidget::drawEdge(QPainter& painter, const QPointF& from, const QPointF& to,
                             int capacity, int residual, int cost, bool isMinCut, bool isOnPath,
                             bool isReverse)
{
    QLineF line(from, to);
    double angle = std::atan2(line.dy(), line.dx());

    QPointF start = from + QPointF(NODE_RADIUS * cos(angle), NODE_RADIUS * sin(angle));
    QPointF end   = to   - QPointF((NODE_RADIUS + 5) * cos(angle), (NODE_RADIUS + 5) * sin(angle));

    QColor color = QColor(100, 149, 237);
    int width = 2;

    if (isMinCut) {
        color = Qt::red;
        width = 4;
    } else if (isOnPath) {
        color = QColor(255, 140, 0);
        width = 3;
    }

    painter.setPen(QPen(color, width));
    painter.drawLine(start, end);

    double arrowSize = ARROW_SIZE;
    QPointF p1 = end - QPointF(arrowSize * cos(angle - M_PI / 6), arrowSize * sin(angle - M_PI / 6));
    QPointF p2 = end - QPointF(arrowSize * cos(angle + M_PI / 6), arrowSize * sin(angle + M_PI / 6));

    QPolygonF arrowHead;
    arrowHead << end << p1 << p2;
    painter.setBrush(color);
    painter.drawPolygon(arrowHead);

    if (residual >= 0 || capacity > 0) {
        QString text;
        if (residual == -1)
            text = cost != 0 ? QString("%1 [c:%2]").arg(capacity).arg(cost)
                             : QString::number(capacity);
        else
            text = cost != 0 ? QString("%1 [c:%2]").arg(residual).arg(cost)
                             : QString::number(residual);

        QPointF mid = (start + end) / 2.0;
        QPointF perp(-std::sin(angle) * 14, std::cos(angle) * 14);
        mid += perp;

        QFont font = painter.font();
        font.setPointSize(10);
        painter.setFont(font);
        QFontMetrics fm(font);
        int uw = fm.horizontalAdvance(text) + 10;
        int uh = fm.height() + 4;

        QRectF labelRect(mid.x() - uw/2, mid.y() - uh/2, uw, uh);

        painter.setPen(Qt::black);
        painter.setBrush(Qt::white);
        painter.drawRect(labelRect);
        painter.drawText(labelRect, Qt::AlignCenter, text);
    }
}
