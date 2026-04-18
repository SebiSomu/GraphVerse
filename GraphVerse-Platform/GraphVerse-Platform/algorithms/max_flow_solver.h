#ifndef MAX_FLOW_SOLVER_H
#define MAX_FLOW_SOLVER_H

#include "../graph_interfaces_isp.h"  // ISP-compliant interfaces
#include <vector>

struct MaxFlowResult {
    int maxFlow;
    std::vector<std::pair<int, int>> matchedEdges; // For bipartite matching convenience
};

class MaxFlowSolver {
public:
    MaxFlowResult solve(const IGraphData& graph, int source, int sink) const;
};

#endif // MAX_FLOW_SOLVER_H
