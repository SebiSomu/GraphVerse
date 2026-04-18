#ifndef I_TRAVERSAL_H
#define I_TRAVERSAL_H

#include "graph_algorithm_types.h"
#include "../graph_interfaces_isp.h"  // ISP-compliant interfaces
#include <vector>
#include <functional>
#include <string>

// Traversal algorithm interface using IGraphComplete
// Traversals need IGraphComplete because they access getGraphType() from IGraphMetadata
class ITraversalAlgorithm {
public:
    virtual ~ITraversalAlgorithm() = default;
    virtual std::vector<TraversalStep> solve(
        const IGraphComplete& graph, int startIndex, int stopAt = -1) const = 0;
    virtual std::string name() const = 0;
};

#endif // I_TRAVERSAL_H
