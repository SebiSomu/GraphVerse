#ifndef CONNECTED_COMPONENTS_SOLVER_H
#define CONNECTED_COMPONENTS_SOLVER_H

#include "../graph_interfaces.h"

#include <unordered_map>
#include <vector>

class ConnectedComponentsSolver {
public:
    // Returns number of components and fills a map of nodeId -> componentId
    int solve(const IGraph& graph, std::unordered_map<int, int>& outComponentMap) const;

};

#endif // CONNECTED_COMPONENTS_SOLVER_H
