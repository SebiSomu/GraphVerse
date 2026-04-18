#ifndef BELLMAN_FORD_SOLVER_H
#define BELLMAN_FORD_SOLVER_H

#include "i_shortest_path.h"

class BellmanFordSolver : public IShortestPathAlgorithm {
public:
    std::vector<PathStep> solve(const IGraphData& graph, int startIdx, int endIdx,
                                std::vector<int>& outPath) const override;
    std::string name() const override { return "Bellman-Ford"; }
};

#endif // BELLMAN_FORD_SOLVER_H
