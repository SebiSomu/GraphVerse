#include "undirectedgraph.h"
#include <cmath>

UndirectedGraph::UndirectedGraph() {}

void UndirectedGraph::addEdge(Node &f, Node &s, int cost)
{
    Edge newEdge(&f, &s, cost);
    m_edges.push_back(newEdge);
}

void UndirectedGraph::drawEdge(QPainter &p) const
{
    const double nodeRadius = 10.0;

    for(const auto& ed : m_edges) {
        QPointF start(ed.getFirst().getX(), ed.getFirst().getY());
        QPointF end(ed.getSecond().getX(), ed.getSecond().getY());

        QPointF midPoint = (start + end) / 2.0;
        QString costText = QString::number(ed.getCost());

        QLineF line(start, end);
        double angle = std::atan2(line.dy(), line.dx());
        QPointF offset(-17 * sin(angle), 17 * cos(angle));
        QPointF textPos = midPoint + offset;

        QPointF adjustedStart = start + QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));
        QPointF adjustedEnd = end - QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));
        p.drawLine(adjustedStart, adjustedEnd);

        p.save();
        p.setPen(Qt::green);
        QFont font = p.font();
        font.setBold(true);
        font.setPointSize(12);
        p.setFont(font);
        p.drawText(textPos, costText);
        p.restore();
    }

    for(const auto& n : m_nodes) {
        QRect r(n.getX() - nodeRadius, n.getY() - nodeRadius, nodeRadius*2, nodeRadius*2);
        p.drawEllipse(r);
        QString s = QString::number(n.getIndex());
        p.drawText(r, Qt::AlignCenter, s);
    }
}

std::string UndirectedGraph::getGraphType() const
{
    return "Undirected";
}
