#ifndef I_SHORTEST_PATH_H
#define I_SHORTEST_PATH_H

#include "graph_algorithm_types.h"
#include "../graph.h"
#include <vector>
#include <string>

class IShortestPathAlgorithm {
public:
    virtual ~IShortestPathAlgorithm() = default;
    virtual std::vector<PathStep> solve(
        const Graph& graph, int startIdx, int endIdx,
        std::vector<int>& outPath) const = 0;
    virtual std::string name() const = 0;
};

#endif // I_SHORTEST_PATH_H
