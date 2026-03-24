#ifndef DIRECTED_GRAPH_RENDERER_H
#define DIRECTED_GRAPH_RENDERER_H

#include "graph_renderer.h"

class DirectedGraphRenderer : public IGraphRenderer {
public:
    void render(QPainter& p, const Graph& graph,
                const ComponentResult* compResults = nullptr,
                const CondensedResult* condensedResults = nullptr) override;

private:
    void drawCondensedGraph(QPainter& p, const Graph& graph, const CondensedResult* results, const ComponentResult* compResults) const;
    void drawNormalGraph(QPainter& p, const Graph& graph, const ComponentResult* results) const;
};

#endif // DIRECTED_GRAPH_RENDERER_H
