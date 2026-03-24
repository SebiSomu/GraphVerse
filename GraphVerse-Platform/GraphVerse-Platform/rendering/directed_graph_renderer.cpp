#include "directed_graph_renderer.h"
#include "../directedgraph.h"
#include "component_colorizer.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QLineF>
#include <QPointF>
#include <QRect>
#include <QtMath>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void DirectedGraphRenderer::render(QPainter& p, const Graph& graph, 
                                   const ComponentResult* compResults,
                                   const CondensedResult* condensedResults) {
    if (condensedResults && condensedResults->isShowing && compResults && compResults->numComponents > 0) {
        drawCondensedGraph(p, graph, condensedResults, compResults);
    } else {
        drawNormalGraph(p, graph, compResults);
    }
}

void DirectedGraphRenderer::drawNormalGraph(QPainter& p, const Graph& graph, const ComponentResult* results) const {
    const double arrowSize = 15.0;
    const double nodeRadius = 10.0;
    bool componentsFound = (results != nullptr && results->numComponents > 0);

    // Edges
    for (const auto& ed : graph.getEdges()) {
        QPointF start(ed.getFirst().getX(), ed.getFirst().getY());
        QPointF end(ed.getSecond().getX(), ed.getSecond().getY());
        QLineF line(start, end);
        double angle = std::atan2(line.dy(), line.dx());
        QPointF adjStart = start + QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));
        QPointF adjEnd = end - QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));

        if (componentsFound) {
            auto it = results->nodeToComponent.find(ed.getFirst().getIndex());
            int comp = (it != results->nodeToComponent.end()) ? it->second : -1;
            QColor edgeCol = ComponentColorizer::getColorForComponent(comp, results->numComponents);
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
            auto it = results->nodeToComponent.find(n.getIndex());
            int comp = (it != results->nodeToComponent.end()) ? it->second : -1;
            QColor nodeColor = ComponentColorizer::getColorForComponent(comp, results->numComponents);
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

void DirectedGraphRenderer::drawCondensedGraph(QPainter& p, const Graph& graph, const CondensedResult* results, const ComponentResult* compResults) const {
    if (!results || !compResults) return;

    const auto& condensed = results->graph;
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
            QColor edgeColor = ComponentColorizer::getColorForComponent(comp, compResults->numComponents);
            p.setPen(QPen(edgeColor, 3));
            p.drawLine(adjStart, adjEnd);
            p.drawLine(adjEnd, adjEnd - QPointF(arrowSize * cos(angle - M_PI / 6), arrowSize * sin(angle - M_PI / 6)));
            p.drawLine(adjEnd, adjEnd - QPointF(arrowSize * cos(angle + M_PI / 6), arrowSize * sin(angle + M_PI / 6)));
        }
    }

    for (const auto& n : condensed.nodes) {
        QRect r(n.getX() - nodeWidth / 2, n.getY() - nodeHeight / 2, nodeWidth, nodeHeight);
        int comp = n.getIndex() - 1;
        QColor nodeColor = ComponentColorizer::getColorForComponent(comp, compResults->numComponents);
        p.setPen(QPen(nodeColor, 3));
        p.setBrush(QBrush(nodeColor.lighter(180)));
        p.drawEllipse(r);
        p.setPen(Qt::black);
        
        QString label; bool first = true;
        for (const auto& origNode : graph.getNodes()) {
            auto it = compResults->nodeToComponent.find(origNode.getIndex());
            int nodeComp = (it != compResults->nodeToComponent.end()) ? it->second : -1;
            if (nodeComp == comp) {
                if (!first) label += ",";
                label += QString::number(origNode.getIndex());
                first = false;
            }
        }
        QFont font = p.font(); font.setPointSize(10); font.setBold(true); p.setFont(font);
        p.drawText(r, Qt::AlignCenter, label);
    }
}
