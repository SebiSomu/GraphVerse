#ifndef DIJKSTRA_SOLVER_H
#define DIJKSTRA_SOLVER_H

#include "i_shortest_path.h"

class DijkstraSolver : public IShortestPathAlgorithm {
public:
    std::vector<PathStep> solve(const Graph& graph, int startIdx, int endIdx,
                                std::vector<int>& outPath) const override;
    std::string name() const override { return "Dijkstra"; }
};

#endif // DIJKSTRA_SOLVER_H
