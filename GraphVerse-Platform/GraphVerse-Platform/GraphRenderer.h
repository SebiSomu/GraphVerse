#ifndef GRAPH_RENDERER_H
#define GRAPH_RENDERER_H

#include <QPainter>
#include <QColor>
#include <vector>
#include "graph_interfaces_isp.h"  // ISP-compliant interfaces
#include "algorithms/graph_algorithm_types.h"

struct RenderSettings {
    QColor nodeColor = QColor(20, 184, 166);       // Teal accent
    QColor nodeBorder = QColor(45, 212, 191);     // Lighter teal
    QColor textColor = QColor(235, 237, 255);     // Off-white
    QColor edgeColor = QColor(140, 148, 190, 100); // Muted blue-gray
    QColor highlightColor = QColor(245, 158, 11); // Amber for paths/MST
    int nodeRadius = 22;
    int edgeWidth = 2;
    bool showEdgeCosts = true;
};

class GraphRenderer {
public:
    static void drawGraph(QPainter& painter, const IGraphComplete& graph, const RenderSettings& settings = RenderSettings());
    
    static void drawNode(QPainter& painter, const Node& node, const RenderSettings& settings, bool highlighted = false);
    
    static void drawEdge(QPainter& painter, const Edge& edge, const RenderSettings& settings, bool highlighted = false, bool directed = false);

    static void drawMST(QPainter& painter, const IGraphData& graph, const std::vector<MSTStep>& mst, const RenderSettings& settings = RenderSettings());
    
    static void drawPath(QPainter& painter, const IGraphData& graph, const std::vector<int>& path, const RenderSettings& settings = RenderSettings());
};

#endif // GRAPH_RENDERER_H
