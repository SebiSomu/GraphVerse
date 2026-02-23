#include "undirectedgraph.h"

UndirectedGraph::UndirectedGraph() {}

void UndirectedGraph::addEdge(Node &f, Node &s)
{
    m_edges.emplace_back(&f, &s);
    m_componentsColors.clear();
    m_numComponents = 0;
}

void UndirectedGraph::drawEdge(QPainter &p) const
{
    const double nodeRadius = 10.0;
    bool componentsFound = (m_numComponents > 0);

    for(const auto& ed : m_edges) {
        QPointF start(ed.getFirst().getX(), ed.getFirst().getY());
        QPointF end(ed.getSecond().getX(), ed.getSecond().getY());

        QLineF line(start, end);
        double angle = std::atan2(line.dy(), line.dx());

        QPointF adjustedStart = start + QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));
        QPointF adjustedEnd = end - QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));

        if(componentsFound) {
            int comp = getComponentColor(ed.getFirst().getIndex());
            p.setPen(QPen(getColorForComponent(comp), 2));
        } else {
            p.setPen(QPen(Qt::white, 1));
        }

        p.drawLine(adjustedStart, adjustedEnd);
    }

    for(const auto& n : m_nodes) {
        QRect r(n.getX() - nodeRadius, n.getY() - nodeRadius, nodeRadius*2, nodeRadius*2);

        if(componentsFound) {
            int comp = getComponentColor(n.getIndex());
            QColor nodeColor = getColorForComponent(comp);
            p.setPen(QPen(nodeColor, 2));
            p.setBrush(QBrush(nodeColor.lighter(160)));
        } else {
            p.setPen(QPen(Qt::black, 1));
            p.setBrush(QBrush(Qt::white));
        }

        p.drawEllipse(r);

        p.setPen(Qt::black);
        QString s = QString::number(n.getIndex());
        p.drawText(r, Qt::AlignCenter, s);
    }
}

std::string UndirectedGraph::getGraphType() const
{
    return "Undirected";
}


