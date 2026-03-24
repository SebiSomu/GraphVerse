#include "graph_renderer_factory.h"
#include "directed_graph_renderer.h"
#include "undirected_graph_renderer.h"

std::unique_ptr<IGraphRenderer> GraphRendererFactory::createRenderer(const Graph& graph) {
    if (graph.getGraphType() == "Directed") {
        return std::make_unique<DirectedGraphRenderer>();
    } else {
        return std::make_unique<UndirectedGraphRenderer>();
    }
}
