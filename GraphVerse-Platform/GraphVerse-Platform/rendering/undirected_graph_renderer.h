#ifndef UNDIRECTED_GRAPH_RENDERER_H
#define UNDIRECTED_GRAPH_RENDERER_H

#include "graph_renderer.h"

class UndirectedGraphRenderer : public IGraphRenderer {
public:
    void render(QPainter& p, const Graph& graph,
                const ComponentResult* compResults = nullptr,
                const CondensedResult* condensedResults = nullptr) override;
};

#endif // UNDIRECTED_GRAPH_RENDERER_H
