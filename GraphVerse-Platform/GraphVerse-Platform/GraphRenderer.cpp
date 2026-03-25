#include "GraphRenderer.h"
#include <QFontMetrics>
#include <cmath>

void GraphRenderer::drawGraph(QPainter& painter, const IGraph& graph, const RenderSettings& settings) {
    bool isDirected = graph.getGraphType() == "directed";
    
    // Draw edges first so they are behind nodes
    for (const auto& edge : graph.getEdges()) {
        drawEdge(painter, edge, settings, false, isDirected);
    }
    
    // Draw nodes
    for (const auto& node : graph.getNodes()) {
        drawNode(painter, node, settings);
    }
}

void GraphRenderer::drawNode(QPainter& painter, const Node& node, const RenderSettings& settings, bool highlighted) {
    QPoint pos = node.getCoord();
    int r = settings.nodeRadius;

    painter.setRenderHint(QPainter::Antialiasing);
    
    // Shadow effect
    painter.setBrush(QColor(0, 0, 0, 40));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(pos.x() - r + 3, pos.y() - r + 3, 2 * r, 2 * r);

    // Node body
    QRadialGradient grad(pos, r);
    QColor base = highlighted ? settings.highlightColor : settings.nodeColor;
    grad.setColorAt(0, base.lighter(120));
    grad.setColorAt(1, base);
    
    painter.setBrush(grad);
    painter.setPen(QPen(settings.nodeBorder, 2));
    painter.drawEllipse(pos.x() - r, pos.y() - r, 2 * r, 2 * r);

    // Label
    QString name = node.getName();
    if (name.isEmpty()) name = QString::number(node.getIndex());
    
    painter.setPen(settings.textColor);
    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    
    QRect textRect = painter.fontMetrics().boundingRect(name);
    painter.drawText(pos.x() - textRect.width() / 2, pos.y() + textRect.height() / 4, name);
}

void GraphRenderer::drawEdge(QPainter& painter, const Edge& edge, const RenderSettings& settings, bool highlighted, bool directed) {
    QPoint p1 = edge.getFirst().getCoord();
    QPoint p2 = edge.getSecond().getCoord();
    
    QPen pen(highlighted ? settings.highlightColor : settings.edgeColor, settings.edgeWidth);
    if (highlighted) pen.setWidth(settings.edgeWidth + 1);
    
    painter.setPen(pen);
    painter.drawLine(p1, p2);

    if (directed) {
        // Draw arrow head
        double angle = std::atan2(p2.y() - p1.y(), p2.x() - p1.x());
        int arrowSize = 10;
        
        // Offset to node boundary
        int r = settings.nodeRadius;
        QPoint arrowTip = p2 - QPoint(r * std::cos(angle), r * std::sin(angle));
        
        QPoint pArrow1 = arrowTip - QPoint(arrowSize * std::cos(angle - M_PI/6), arrowSize * std::sin(angle - M_PI/6));
        QPoint pArrow2 = arrowTip - QPoint(arrowSize * std::cos(angle + M_PI/6), arrowSize * std::sin(angle + M_PI/6));
        
        painter.setBrush(highlighted ? settings.highlightColor : settings.edgeColor);
        painter.drawPolygon(QPolygon() << arrowTip << pArrow1 << pArrow2);
    }

    if (settings.showEdgeCosts) {
        QString costStr = QString::number(edge.getCost());
        QPoint mid = (p1 + p2) / 2;
        
        painter.setPen(settings.textColor);
        painter.setFont(QFont("Segoe UI", 9));
        
        // Draw background for text to make it readable
        QRect rect = painter.fontMetrics().boundingRect(costStr).adjusted(-2, -1, 2, 1);
        rect.moveCenter(mid);
        painter.fillRect(rect, QColor(14, 17, 32, 180));
        
        painter.drawText(rect, Qt::AlignCenter, costStr);
    }
}

void GraphRenderer::drawMST(QPainter& painter, const IGraph& graph, const std::vector<MSTStep>& mst, const RenderSettings& settings) {
    auto nodes = graph.getNodes();
    auto getPos = [&](int id) {
        for (const auto& n : nodes) if (n.getIndex() == id) return n.getCoord();
        return QPoint(0, 0);
    };

    painter.setRenderHint(QPainter::Antialiasing);
    for (const auto& step : mst) {
        if (step.accepted) {
            QPoint p1 = getPos(step.fromIndex);
            QPoint p2 = getPos(step.toIndex);
            QPen pen(settings.highlightColor, settings.edgeWidth + 1);
            painter.setPen(pen);
            painter.drawLine(p1, p2);
        }
    }
    
    // Draw nodes on top
    for (const auto& node : nodes) drawNode(painter, node, settings);
}

void GraphRenderer::drawPath(QPainter& painter, const IGraph& graph, const std::vector<int>& path, const RenderSettings& settings) {
    if (path.size() < 2) return;
    
    auto nodes = graph.getNodes();
    auto getPos = [&](int id) {
        for (const auto& n : nodes) if (n.getIndex() == id) return n.getCoord();
        return QPoint(0, 0);
    };

    painter.setRenderHint(QPainter::Antialiasing);
    for (size_t i = 0; i < path.size() - 1; ++i) {
        QPoint p1 = getPos(path[i]);
        QPoint p2 = getPos(path[i+1]);
        
        QPen pen(settings.highlightColor, settings.edgeWidth + 2);
        painter.setPen(pen);
        painter.drawLine(p1, p2);
        
        // Directional arrow for path
        double angle = std::atan2(p2.y() - p1.y(), p2.x() - p1.x());
        int r = settings.nodeRadius;
        QPoint tip = p2 - QPoint(r * std::cos(angle), r * std::sin(angle));
        int arrowSize = 12;
        QPoint a1 = tip - QPoint(arrowSize * std::cos(angle - M_PI/6), arrowSize * std::sin(angle - M_PI/6));
        QPoint a2 = tip - QPoint(arrowSize * std::cos(angle + M_PI/6), arrowSize * std::sin(angle + M_PI/6));
        
        painter.setBrush(settings.highlightColor);
        painter.drawPolygon(QPolygon() << tip << a1 << a2);
    }

    // Draw nodes
    for (const auto& node : nodes) drawNode(painter, node, settings);
}
