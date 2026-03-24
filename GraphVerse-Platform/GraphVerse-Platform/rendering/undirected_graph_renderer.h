#ifndef UNDIRECTED_GRAPH_RENDERER_H
#define UNDIRECTED_GRAPH_RENDERER_H

#include "graph_renderer.h"

class UndirectedGraphRenderer : public IGraphRenderer {
public:
    void render(QPainter& p, const Graph& graph) override;
};

#endif // UNDIRECTED_GRAPH_RENDERER_H
