#include "undirected_graph_renderer.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QLineF>
#include <QtMath>
#include "component_colorizer.h"

void UndirectedGraphRenderer::render(QPainter& p, const Graph& graph) {
    const double nodeRadius = 10.0;
    int numComponents = graph.getNumComponents();
    bool componentsFound = (numComponents > 0);

    // Draw Edges
    for (const auto& ed : graph.getEdges()) {
        QPointF start(ed.getFirst().getX(), ed.getFirst().getY());
        QPointF end(ed.getSecond().getX(), ed.getSecond().getY());
        QLineF line(start, end);
        double angle = std::atan2(line.dy(), line.dx());
        QPointF adjStart = start + QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));
        QPointF adjEnd = end - QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));

        if (componentsFound) {
            int comp = graph.getComponentColor(ed.getFirst().getIndex());
            QColor edgeCol = ComponentColorizer::getColorForComponent(comp, numComponents);
            p.setPen(QPen(edgeCol, 2));
        } else {
            p.setPen(QPen(Qt::white, 1));
        }
        p.drawLine(adjStart, adjEnd);
    }

    // Draw Nodes
    for (const auto& n : graph.getNodes()) {
        QRect r(n.getX() - static_cast<int>(nodeRadius), n.getY() - static_cast<int>(nodeRadius),
                static_cast<int>(nodeRadius) * 2, static_cast<int>(nodeRadius) * 2);
        if (componentsFound) {
            int comp = graph.getComponentColor(n.getIndex());
            QColor nodeColor = ComponentColorizer::getColorForComponent(comp, numComponents);
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
