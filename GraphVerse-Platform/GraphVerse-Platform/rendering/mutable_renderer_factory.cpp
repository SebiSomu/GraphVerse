#include "mutable_renderer_factory.h"

std::unique_ptr<MutableGraphRenderer> MutableRendererFactory::createRenderer() {
    return std::make_unique<MutableGraphRenderer>();
}

std::unique_ptr<MutableGraphRenderer> MutableRendererFactory::createRenderer(bool directed) {
    auto renderer = std::make_unique<MutableGraphRenderer>();
    // Settings will be configured by the caller via renderWithState
    (void)directed; // Parameter available for future use
    return renderer;
}

std::unique_ptr<MutableGraphRenderer> MutableRendererFactory::createRenderer(const Graph& graph) {
    auto renderer = std::make_unique<MutableGraphRenderer>();
    // Settings will be configured by the caller based on graph type
    (void)graph; // Parameter available for future use
    return renderer;
}
