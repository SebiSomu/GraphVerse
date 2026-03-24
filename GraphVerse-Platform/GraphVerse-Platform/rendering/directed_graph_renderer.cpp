#include "directed_graph_renderer.h"
#include "../directedgraph.h"
#include "component_colorizer.h"
#include <QPen>
#include <QBrush>
#include <QLineF>
#include <QtMath>

void DirectedGraphRenderer::render(QPainter& p, const Graph& graph) {
    const DirectedGraph* dg = dynamic_cast<const DirectedGraph*>(&graph);
    if (!dg) return;

    if (dg->isShowingCondensedGraph() && dg->getNumComponents() > 0) {
        drawCondensedGraph(p, graph);
    } else {
        drawNormalGraph(p, graph);
    }
}

void DirectedGraphRenderer::drawNormalGraph(QPainter& p, const Graph& graph) const {
    const double arrowSize = 15.0;
    const double nodeRadius = 10.0;
    bool componentsFound = (graph.getNumComponents() > 0);

    // Edges
    for (const auto& ed : graph.getEdges()) {
        QPointF start(ed.getFirst().getX(), ed.getFirst().getY());
        QPointF end(ed.getSecond().getX(), ed.getSecond().getY());
        QLineF line(start, end);
        double angle = std::atan2(line.dy(), line.dx());
        QPointF adjStart = start + QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));
        QPointF adjEnd = end - QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));

        if (componentsFound) {
            int comp = graph.getComponentColor(ed.getFirst().getIndex());
            QColor edgeCol = ComponentColorizer::getColorForComponent(comp, graph.getNumComponents());
            p.setPen(QPen(edgeCol, 2));
        } else {
            p.setPen(QPen(Qt::white, 1));
        }
        p.drawLine(adjStart, adjEnd);
        p.drawLine(adjEnd, adjEnd - QPointF(arrowSize * cos(angle - M_PI / 6), arrowSize * sin(angle - M_PI / 6)));
        p.drawLine(adjEnd, adjEnd - QPointF(arrowSize * cos(angle + M_PI / 6), arrowSize * sin(angle + M_PI / 6)));
    }

    // Nodes
    for (const auto& n : graph.getNodes()) {
        QRect r(n.getX() - static_cast<int>(nodeRadius), n.getY() - static_cast<int>(nodeRadius),
                static_cast<int>(nodeRadius) * 2, static_cast<int>(nodeRadius) * 2);
        if (componentsFound) {
            int comp = graph.getComponentColor(n.getIndex());
            QColor nodeColor = ComponentColorizer::getColorForComponent(comp, graph.getNumComponents());
            p.setPen(QPen(nodeColor, 2));
            p.setBrush(QBrush(nodeColor.lighter(160)));
        } else {
            p.setPen(QPen(Qt::black, 1));
            p.setBrush(QBrush(Qt::white));
        }
        p.drawEllipse(r);
        p.setPen(Qt::black);
        p.drawText(r, Qt::AlignCenter, QString::number(n.getIndex()));
    }
}

void DirectedGraphRenderer::drawCondensedGraph(QPainter& p, const Graph& graph) const {
    const DirectedGraph* dg = dynamic_cast<const DirectedGraph*>(&graph);
    if (!dg) return;

    const auto& condensed = dg->getCondensedGraph();
    constexpr double arrowSize = 15.0;
    constexpr int nodeWidth = 80, nodeHeight = 40;
    constexpr double a = nodeWidth / 2.0, b = nodeHeight / 2.0;

    for (const auto& ed : condensed.edges) {
        QPointF startCenter(ed.getFirst().getX(), ed.getFirst().getY());
        QPointF endCenter(ed.getSecond().getX(), ed.getSecond().getY());
        QPointF direction = endCenter - startCenter;
        double length = std::sqrt(direction.x() * direction.x() + direction.y() * direction.y());
        if (length > 0) {
            QPointF unitDirection = direction / length;
            double t1 = 1.0 / std::sqrt((unitDirection.x() * unitDirection.x()) / (a * a) +
                                         (unitDirection.y() * unitDirection.y()) / (b * b));
            QPointF adjStart = startCenter + t1 * unitDirection;
            QPointF reverseDirection = startCenter - endCenter;
            QPointF unitRv = reverseDirection / length;
            double t2 = 1.0 / std::sqrt((unitRv.x() * unitRv.x()) / (a * a) + (unitRv.y() * unitRv.y()) / (b * b));
            QPointF adjEnd = endCenter + t2 * unitRv;
            
            QLineF line(adjStart, adjEnd);
            double angle = std::atan2(line.dy(), line.dx());
            int comp = ed.getFirst().getIndex() - 1;
            QColor edgeColor = ComponentColorizer::getColorForComponent(comp, graph.getNumComponents());
            p.setPen(QPen(edgeColor, 3));
            p.drawLine(adjStart, adjEnd);
            p.drawLine(adjEnd, adjEnd - QPointF(arrowSize * cos(angle - M_PI / 6), arrowSize * sin(angle - M_PI / 6)));
            p.drawLine(adjEnd, adjEnd - QPointF(arrowSize * cos(angle + M_PI / 6), arrowSize * sin(angle + M_PI / 6)));
        }
    }

    for (const auto& n : condensed.nodes) {
        QRect r(n.getX() - nodeWidth / 2, n.getY() - nodeHeight / 2, nodeWidth, nodeHeight);
        int comp = n.getIndex() - 1;
        QColor nodeColor = ComponentColorizer::getColorForComponent(comp, graph.getNumComponents());
        p.setPen(QPen(nodeColor, 3));
        p.setBrush(QBrush(nodeColor.lighter(180)));
        p.drawEllipse(r);
        p.setPen(Qt::black);
        
        QString label; bool first = true;
        for (const auto& origNode : graph.getNodes()) {
            if (graph.getComponentColor(origNode.getIndex()) == comp) {
                if (!first) label += ",";
                label += QString::number(origNode.getIndex());
                first = false;
            }
        }
        QFont font = p.font(); font.setPointSize(10); font.setBold(true); p.setFont(font);
        p.drawText(r, Qt::AlignCenter, label);
    }
}
