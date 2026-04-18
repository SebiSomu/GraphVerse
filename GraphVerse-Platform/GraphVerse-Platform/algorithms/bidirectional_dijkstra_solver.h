#ifndef BIDIRECTIONAL_DIJKSTRA_SOLVER_H
#define BIDIRECTIONAL_DIJKSTRA_SOLVER_H

#include "i_shortest_path.h"

class BidirectionalDijkstraSolver : public IShortestPathAlgorithm {
public:
    std::vector<PathStep> solve(const Graph& graph, int startIdx, int endIdx,
                                std::vector<int>& outPath) const override;
    std::string name() const override { return "Bidirectional Dijkstra"; }
};

#endif // BIDIRECTIONAL_DIJKSTRA_SOLVER_H
