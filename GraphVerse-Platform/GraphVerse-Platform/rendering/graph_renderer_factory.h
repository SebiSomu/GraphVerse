#ifndef GRAPH_RENDERER_FACTORY_H
#define GRAPH_RENDERER_FACTORY_H

#include "graph_renderer.h"
#include <memory>

class GraphRendererFactory {
public:
    static std::unique_ptr<IGraphRenderer> createRenderer(const Graph& graph);
};

#endif // GRAPH_RENDERER_FACTORY_H
