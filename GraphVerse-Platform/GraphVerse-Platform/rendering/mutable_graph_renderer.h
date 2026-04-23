#ifndef MUTABLE_GRAPH_RENDERER_H
#define MUTABLE_GRAPH_RENDERER_H

#include "graph_renderer.h"
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <utility>

// Animation state for mutable graph rendering
struct AnimationState {
    std::unordered_set<int> visitedNodes;
    std::unordered_set<int> activeNodes;
    std::unordered_set<int> articulationPoints;
    std::vector<std::pair<int, int>> bridges;
    std::pair<int, int> activeEdge = {-1, -1};
    int activeNode = -1;
    int activeNode2 = -1;
    float pulseValue = 0.0f;
    bool showResult = false;
    bool isAnimating = false;
    
    // Articulation bridge specific
    std::unordered_map<int, int> discValues;
    std::unordered_map<int, int> lowValues;
};

// Extended render settings for mutable graphs with dark modern theme
struct MutableRenderSettings {
    // Background
    QColor backgroundColor = QColor(8, 10, 20);
    
    // Node colors (dark modern theme from artbridge)
    QColor nodeColor = QColor(30, 41, 59);           // slate-dark (default)
    QColor nodeVisitedColor = QColor(99, 102, 241);  // indigo
    QColor nodeActiveColor = QColor(139, 92, 246);   // violet
    QColor nodeSelectedColor = QColor(16, 185, 129); // emerald
    QColor nodeAPColor = QColor(239, 68, 68);        // red (articulation point)
    
    // Edge colors
    QColor edgeColor = QColor(51, 65, 85);          // dim slate (default)
    QColor edgeTreeColor = QColor(99, 102, 241, 160); // indigo transparent
    QColor edgeActiveColor = QColor(139, 92, 246);   // violet
    QColor edgeBridgeColor = QColor(245, 158, 11);   // amber
    
    // Glow effects
    QColor glowActiveColor = QColor(139, 92, 246, 100);   // violet glow
    QColor glowAPColor = QColor(239, 68, 68, 120);        // red glow for AP
    
    // Text
    QColor textColor = QColor(226, 232, 240);        // light gray
    QColor labelColor = QColor(148, 163, 184);       // slate for labels
    
    int nodeRadius = 18;
    int edgeWidth = 2;
    int activeEdgeWidth = 3;
    int bridgeEdgeWidth = 4;
    bool showGlowEffects = true;
    bool directed = false;
    bool showDiscLowLabels = false;  
};

class MutableGraphRenderer : public IGraphRenderer {
public:
    void render(QPainter& p, const Graph& graph,
                const ComponentResult* compResults = nullptr,
                const CondensedResult* condensedResults = nullptr) override;
    
    // Extended render with animation state and settings
    void renderWithState(QPainter& p, const Graph& graph, 
                         const AnimationState& animState,
                         const MutableRenderSettings& settings = MutableRenderSettings(),
                         int selectedNodeId = -1);
    
    // Draw individual elements (for custom compositions)
    void drawNode(QPainter& p, const Node& node, const MutableRenderSettings& settings,
                  const AnimationState& animState, bool isSelected = false);
    
    void drawEdge(QPainter& p, const Edge& edge, const MutableRenderSettings& settings,
                  const AnimationState& animState);
    
    // Draw pending edge (for edge creation UI)
    void drawPendingEdge(QPainter& p, const QPoint& from, const QPoint& to);
    
protected:  // Allow derived classes to access these
    void drawBackground(QPainter& p, const QRect& rect, const QColor& color);
    void drawGlowEffect(QPainter& p, const QPointF& center, float radius, const QColor& color, float pulse);
    void drawArrowHead(QPainter& p, const QPointF& tip, double angle, double arrowSize, const QColor& color);
    bool isNodeActive(int nodeId, const AnimationState& state) const;
    bool isEdgeBridge(int u, int v, const AnimationState& state) const;
    bool isEdgeActive(int u, int v, const AnimationState& state) const;
    QColor getNodeColor(const Node& node, const MutableRenderSettings& settings, 
                        const AnimationState& state, bool isSelected) const;
    QColor getEdgeColor(const Edge& edge, const MutableRenderSettings& settings,
                        const AnimationState& state) const;
};

#endif // MUTABLE_GRAPH_RENDERER_H
