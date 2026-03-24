#ifndef CONDENSED_GRAPH_SOLVER_H
#define CONDENSED_GRAPH_SOLVER_H

#include "../graph.h"
#include <unordered_map>
#include <vector>
#include <list>


class CondensedGraphSolver {
public:
    CondensedGraph solve(const Graph& graph, int numComponents, const std::unordered_map<int, int>& nodeToComponent) const;
};

#endif // CONDENSED_GRAPH_SOLVER_H
