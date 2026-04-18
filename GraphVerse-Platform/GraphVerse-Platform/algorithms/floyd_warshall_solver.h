#ifndef FLOYD_WARSHALL_SOLVER_H
#define FLOYD_WARSHALL_SOLVER_H

#include "i_shortest_path.h"

class FloydWarshallSolver : public IShortestPathAlgorithm {
public:
    std::vector<PathStep> solve(const IGraphData& graph, int startIdx, int endIdx,
                                std::vector<int>& outPath) const override;
    std::string name() const override { return "Floyd-Warshall"; }
};

#endif // FLOYD_WARSHALL_SOLVER_H
