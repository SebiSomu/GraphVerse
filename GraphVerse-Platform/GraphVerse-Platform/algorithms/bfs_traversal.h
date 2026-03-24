#ifndef BFS_TRAVERSAL_H
#define BFS_TRAVERSAL_H

#include "i_traversal.h"

class BFSTraversal : public ITraversalAlgorithm {
public:
    std::vector<int> solve(const Graph& graph, int startIndex, int stopAt = -1,
                           std::function<void(int)> onVisit = nullptr,
                           std::function<void(int, int)> onEdge = nullptr) const override;
    std::string name() const override { return "BFS"; }
};

#endif // BFS_TRAVERSAL_H
