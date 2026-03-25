#ifndef MAX_FLOW_SOLVER_H
#define MAX_FLOW_SOLVER_H

#include "../graph_interfaces.h"
#include <vector>

struct MaxFlowResult {
    int maxFlow;
    std::vector<std::pair<int, int>> matchedEdges; // For bipartite matching convenience
};

class MaxFlowSolver {
public:
    MaxFlowResult solve(const IGraph& graph, int sourceIdx, int sinkIdx);
};

#endif // MAX_FLOW_SOLVER_H
