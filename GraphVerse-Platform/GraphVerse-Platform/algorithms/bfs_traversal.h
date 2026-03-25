#ifndef BFS_TRAVERSAL_H
#define BFS_TRAVERSAL_H

#include "i_traversal.h"

class BFSTraversal : public ITraversalAlgorithm {
public:
    std::vector<TraversalStep> solve(const IGraph& graph, int startIndex, int stopAt = -1) const override;
    std::string name() const override { return "BFS"; }
};

#endif // BFS_TRAVERSAL_H
