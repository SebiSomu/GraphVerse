#include "directedgraph.h"

DirectedGraph::DirectedGraph() {}

void DirectedGraph::addEdge(Node &f, Node &s, int capacity)
{
    Edge newEdge(&f, &s, capacity);
    m_edges.push_back(newEdge);
}

void DirectedGraph::drawEdge(QPainter &p) const
{
    const double arrowSize = 15.0;
    const double nodeRadius = 10.0;

    for(const auto &ed : m_edges)
    {
        QPointF start(ed.getFirst().getX(), ed.getFirst().getY());
        QPointF end(ed.getSecond().getX(), ed.getSecond().getY());

        QLineF line(start, end);
        double angle = std::atan2(line.dy(), line.dx());

        QPointF adjustedStart = start + QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));
        QPointF adjustedEnd = end - QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));

        p.drawLine(adjustedStart, adjustedEnd);

        QPointF arrowP1 = adjustedEnd - QPointF(arrowSize * cos(angle - M_PI / 6),
                                                arrowSize * sin(angle - M_PI / 6));
        QPointF arrowP2 = adjustedEnd - QPointF(arrowSize * cos(angle + M_PI / 6),
                                                arrowSize * sin(angle + M_PI / 6));

        p.drawLine(adjustedEnd, arrowP1);
        p.drawLine(adjustedEnd, arrowP2);

        QPointF midPoint = (adjustedStart + adjustedEnd) / 2.0;
        QString capacityText = QString::number(ed.getCapacity());
        QPointF offset(-17 * sin(angle), 17 * cos(angle));
        QPointF textPos = midPoint + offset;

        p.save();
        p.setPen(Qt::green);
        QFont font = p.font();
        font.setBold(true);
        font.setPointSize(12);
        p.setFont(font);
        p.drawText(textPos, capacityText);
        p.restore();
    }

    for(const auto &n : m_nodes)
    {
        QRect r(n.getX() - nodeRadius, n.getY() - nodeRadius, nodeRadius*2, nodeRadius*2);
        p.drawEllipse(r);
        QString s = QString::number(n.getIndex());
        p.drawText(r, Qt::AlignCenter, s);
    }
}

std::string DirectedGraph::getGraphType() const
{
    return "Directed";
}
