#ifndef MUTABLE_RENDERER_FACTORY_H
#define MUTABLE_RENDERER_FACTORY_H

#include "mutable_graph_renderer.h"
#include <memory>

class MutableRendererFactory {
public:
    static std::unique_ptr<MutableGraphRenderer> createRenderer();
    
    // Create with default settings for directed/undirected
    static std::unique_ptr<MutableGraphRenderer> createRenderer(bool directed);
    static std::unique_ptr<MutableGraphRenderer> createRenderer(const Graph& graph);
};

#endif // MUTABLE_RENDERER_FACTORY_H
