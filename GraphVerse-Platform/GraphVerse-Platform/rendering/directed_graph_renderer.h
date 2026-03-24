#ifndef DIRECTED_GRAPH_RENDERER_H
#define DIRECTED_GRAPH_RENDERER_H

#include "graph_renderer.h"

class DirectedGraphRenderer : public IGraphRenderer {
public:
    void render(QPainter& p, const Graph& graph) override;

private:
    void drawCondensedGraph(QPainter& p, const Graph& graph) const;
    void drawNormalGraph(QPainter& p, const Graph& graph) const;
};

#endif // DIRECTED_GRAPH_RENDERER_H
