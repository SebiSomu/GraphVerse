#ifndef DFS_TRAVERSAL_H
#define DFS_TRAVERSAL_H

#include "i_traversal.h"

class DFSTraversal : public ITraversalAlgorithm {
public:
    std::vector<TraversalStep> solve(const IGraph& graph, int startIndex, int stopAt = -1) const override;
    std::string name() const override { return "DFS"; }
};

#endif // DFS_TRAVERSAL_H
