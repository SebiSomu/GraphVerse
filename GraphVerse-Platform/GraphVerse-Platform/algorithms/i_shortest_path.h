#ifndef I_SHORTEST_PATH_H
#define I_SHORTEST_PATH_H

#include "graph_algorithm_types.h"
#include "../graph_interfaces_isp.h"  // ISP-compliant interfaces
#include <vector>
#include <string>

// Shortest path algorithm interface using ISP-compliant IGraphData
// Algorithms only need read access to graph nodes and edges
class IShortestPathAlgorithm {
public:
    virtual ~IShortestPathAlgorithm() = default;
    virtual std::vector<PathStep> solve(
        const IGraphData& graph, int startIdx, int endIdx,
        std::vector<int>& outPath) const = 0;
    virtual std::string name() const = 0;
};

#endif // I_SHORTEST_PATH_H
