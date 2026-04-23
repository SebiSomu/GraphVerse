#include "mutable_graph_renderer.h"
#include "component_colorizer.h"
#include "../graph_interfaces_isp.h"
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QPen>
#include <QBrush>
#include <QLineF>
#include <QtMath>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void MutableGraphRenderer::render(QPainter& p, const Graph& graph,
                                  const ComponentResult* compResults,
                                  const CondensedResult*) {
    AnimationState emptyState;
    MutableRenderSettings settings;
    settings.directed = (graph.getGraphType() == "Directed");
    renderWithState(p, graph, emptyState, settings, -1);
}

void MutableGraphRenderer::renderWithState(QPainter& p, const Graph& graph,
                                           const AnimationState& animState,
                                           const MutableRenderSettings& settings,
                                           int selectedNodeId) {
    p.fillRect(p.viewport(), settings.backgroundColor);
    
    p.setRenderHint(QPainter::Antialiasing);
    
    const auto& nodes = graph.getNodes();
    const auto& edges = graph.getEdges();
    
    for (const auto& edge : edges) {
        drawEdge(p, edge, settings, animState);
    }
    
    for (const auto& node : nodes) {
        bool isSelected = (node.getIndex() == selectedNodeId);
        drawNode(p, node, settings, animState, isSelected);
    }
}

void MutableGraphRenderer::drawNode(QPainter& p, const Node& node, 
                                    const MutableRenderSettings& settings,
                                    const AnimationState& animState,
                                    bool isSelected) {
    int id = node.getIndex();
    QPointF center(node.getX(), node.getY());
    float radius = static_cast<float>(settings.nodeRadius);
    
    bool isAP = animState.showResult ? 
                animState.articulationPoints.count(id) : 
                animState.articulationPoints.count(id);
    bool isActive = (id == animState.activeNode);
    bool isVisited = animState.visitedNodes.count(id) > 0;
    
    // Glow effects
    if (settings.showGlowEffects) {
        if (isAP) {
            // Articulation point - red pulsing glow
            float glowR = radius + 6.f + 4.f * animState.pulseValue;
            QRadialGradient glow(center, glowR);
            glow.setColorAt(0, QColor(239, 68, 68, static_cast<int>(120 * animState.pulseValue)));
            glow.setColorAt(1, Qt::transparent);
            p.setPen(Qt::NoPen);
            p.setBrush(glow);
            p.drawEllipse(center, glowR, glowR);
        } else if (isActive) {
            // Active node - violet glow
            QRadialGradient glow(center, radius + 8);
            glow.setColorAt(0, settings.glowActiveColor);
            glow.setColorAt(1, Qt::transparent);
            p.setPen(Qt::NoPen);
            p.setBrush(glow);
            p.drawEllipse(center, radius + 8, radius + 8);
        }
    }
    
    QColor fillColor = getNodeColor(node, settings, animState, isSelected);
    
    QRadialGradient grad(center, radius);
    grad.setColorAt(0, fillColor.lighter(120));
    grad.setColorAt(1, fillColor);
    
    QPainterPath nodePath;
    nodePath.addEllipse(center, radius, radius);
    
    p.setPen(Qt::NoPen);
    p.setBrush(grad);
    p.drawPath(nodePath);
    
    // Border
    QColor borderColor = isAP ? settings.nodeAPColor :
                         isActive ? settings.nodeActiveColor :
                         isVisited ? settings.nodeVisitedColor :
                         isSelected ? settings.nodeSelectedColor :
                         QColor(71, 85, 105);
    
    float penWidth = isAP ? 2.5f : 1.5f;
    p.setPen(QPen(borderColor, penWidth));
    p.setBrush(Qt::NoBrush);
    p.drawPath(nodePath);
    
    // Node index label
    p.setPen(settings.textColor);
    p.setFont(QFont("Segoe UI", 9, QFont::Bold));
    p.drawText(QRectF(center.x() - radius, center.y() - radius,
                      radius * 2, radius * 2),
               Qt::AlignCenter, QString::number(id));
    
    // disc/low labels during animation
    if (settings.showDiscLowLabels && isVisited && 
        animState.discValues.count(id) && animState.lowValues.count(id)) {
        p.setFont(QFont("Segoe UI", 8));
        p.setPen(settings.labelColor);
        p.drawText(QPointF(center.x() + radius + 3, center.y() - 4),
                   QString("d=%1").arg(animState.discValues.at(id)));
        p.drawText(QPointF(center.x() + radius + 3, center.y() + 10),
                   QString("l=%2").arg(animState.lowValues.at(id)));
    }
}

void MutableGraphRenderer::drawEdge(QPainter& p, const Edge& edge,
                                    const MutableRenderSettings& settings,
                                    const AnimationState& animState) {
    int uid = edge.getFirst().getIndex();
    int vid = edge.getSecond().getIndex();
    QPointF A(edge.getFirst().getX(), edge.getFirst().getY());
    QPointF B(edge.getSecond().getX(), edge.getSecond().getY());
    
    double angle = std::atan2(B.y() - A.y(), B.x() - A.x());
    
    // Adjust endpoints to node boundary
    double nodeRadius = settings.nodeRadius;
    QPointF startPoint(A.x() + nodeRadius * std::cos(angle),
                       A.y() + nodeRadius * std::sin(angle));
    QPointF endPoint(B.x() - nodeRadius * std::cos(angle),
                     B.y() - nodeRadius * std::sin(angle));
    
    // Determine edge appearance
    bool isBridge = isEdgeBridge(uid, vid, animState);
    bool isActive = isEdgeActive(uid, vid, animState);
    bool isTreeEdge = !animState.showResult && 
                      animState.visitedNodes.count(uid) && 
                      animState.visitedNodes.count(vid);
    
    QPen edgePen;
    if (isBridge) {
        edgePen = QPen(settings.edgeBridgeColor, settings.bridgeEdgeWidth,
                       Qt::SolidLine, Qt::RoundCap);
    } else if (isActive) {
        edgePen = QPen(settings.edgeActiveColor, settings.activeEdgeWidth,
                       Qt::SolidLine, Qt::RoundCap);
    } else if (isTreeEdge) {
        edgePen = QPen(settings.edgeTreeColor, settings.edgeWidth,
                       Qt::SolidLine, Qt::RoundCap);
    } else {
        edgePen = QPen(settings.edgeColor, settings.edgeWidth,
                       Qt::SolidLine, Qt::RoundCap);
    }
    
    p.setPen(edgePen);
    p.drawLine(startPoint, endPoint);
    
    // Bridge label
    if (isBridge) {
        QPointF mid = (A + B) / 2.0;
        p.setFont(QFont("Segoe UI", 8, QFont::Bold));
        p.setPen(QColor(245, 158, 11, 200));
        p.drawText(mid + QPointF(4, -4), "bridge");
    }
    
    // Directional arrow for directed graphs
    if (settings.directed) {
        double arrowSize = 10.0;
        drawArrowHead(p, endPoint, angle, arrowSize, 
                       isBridge ? settings.edgeBridgeColor : 
                       isActive ? settings.edgeActiveColor : settings.edgeColor);
    }
}

void MutableGraphRenderer::drawPendingEdge(QPainter& p, const QPoint& from, const QPoint& to) {
    p.setPen(QPen(Qt::yellow, 2, Qt::DashLine));
    p.drawLine(from, to);
}

void MutableGraphRenderer::drawBackground(QPainter& p, const QRect& rect, const QColor& color) {
    p.fillRect(rect, color);
}

void MutableGraphRenderer::drawGlowEffect(QPainter& p, const QPointF& center, float radius, 
                                          const QColor& color, float pulse) {
    QRadialGradient glow(center, radius);
    glow.setColorAt(0, QColor(color.red(), color.green(), color.blue(), 
                              static_cast<int>(color.alpha() * pulse)));
    glow.setColorAt(1, Qt::transparent);
    p.setPen(Qt::NoPen);
    p.setBrush(glow);
    p.drawEllipse(center, radius, radius);
}

void MutableGraphRenderer::drawArrowHead(QPainter& p, const QPointF& tip, double angle, 
                                          double arrowSize, const QColor& color) {
    QPointF a1(tip.x() - arrowSize * std::cos(angle - M_PI / 6),
               tip.y() - arrowSize * std::sin(angle - M_PI / 6));
    QPointF a2(tip.x() - arrowSize * std::cos(angle + M_PI / 6),
               tip.y() - arrowSize * std::sin(angle + M_PI / 6));
    
    p.setBrush(color);
    p.drawPolygon(QPolygonF() << tip << a1 << a2);
}

bool MutableGraphRenderer::isNodeActive(int nodeId, const AnimationState& state) const {
    return state.activeNodes.count(nodeId) > 0;
}

bool MutableGraphRenderer::isEdgeBridge(int u, int v, const AnimationState& state) const {
    int eu = std::min(u, v), ev = std::max(u, v);
    const auto& blist = state.showResult ? state.bridges : state.bridges;
    for (const auto& [bu, bv] : blist) {
        if (bu == eu && bv == ev) return true;
    }
    return false;
}

bool MutableGraphRenderer::isEdgeActive(int u, int v, const AnimationState& state) const {
    return (state.activeNode != -1 && state.activeNode2 != -1 &&
            ((u == state.activeNode && v == state.activeNode2) ||
             (v == state.activeNode && u == state.activeNode2)));
}

QColor MutableGraphRenderer::getNodeColor(const Node& node, const MutableRenderSettings& settings,
                                          const AnimationState& state, bool isSelected) const {
    int id = node.getIndex();
    bool isAP = state.articulationPoints.count(id) > 0;
    bool isActive = (id == state.activeNode);
    bool isVisited = state.visitedNodes.count(id) > 0;
    
    if (isAP) return settings.nodeAPColor;
    if (isActive) return settings.nodeActiveColor;
    if (isVisited) return settings.nodeVisitedColor;
    if (isSelected) return settings.nodeSelectedColor;
    return settings.nodeColor;
}

QColor MutableGraphRenderer::getEdgeColor(const Edge& edge, const MutableRenderSettings& settings,
                                           const AnimationState& state) const {
    int uid = edge.getFirst().getIndex();
    int vid = edge.getSecond().getIndex();
    
    if (isEdgeBridge(uid, vid, state)) return settings.edgeBridgeColor;
    if (isEdgeActive(uid, vid, state)) return settings.edgeActiveColor;
    return settings.edgeColor;
}
