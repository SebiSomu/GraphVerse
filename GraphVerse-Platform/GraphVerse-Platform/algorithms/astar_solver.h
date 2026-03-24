#ifndef ASTAR_SOLVER_H
#define ASTAR_SOLVER_H

#include "i_shortest_path.h"

class AStarSolver : public IShortestPathAlgorithm {
public:
    std::vector<PathStep> solve(const Graph& graph, int startIdx, int endIdx,
                                std::vector<int>& outPath) const override;
    std::string name() const override { return "A*"; }
};

#endif // ASTAR_SOLVER_H
