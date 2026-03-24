#ifndef DFS_TRAVERSAL_H
#define DFS_TRAVERSAL_H

#include "i_traversal.h"

class DFSTraversal : public ITraversalAlgorithm {
public:
    std::vector<int> solve(const Graph& graph, int startIndex, int stopAt = -1,
                           std::function<void(int)> onVisit = nullptr,
                           std::function<void(int, int)> onEdge = nullptr) const override;
    std::string name() const override { return "DFS"; }
};

#endif // DFS_TRAVERSAL_H
