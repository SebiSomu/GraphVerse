#ifndef I_TRAVERSAL_H
#define I_TRAVERSAL_H

#include "../graph.h"
#include <vector>
#include <functional>
#include <string>

class ITraversalAlgorithm {
public:
    virtual ~ITraversalAlgorithm() = default;
    virtual std::vector<int> solve(
        const Graph& graph, int startIndex, int stopAt = -1,
        std::function<void(int)> onVisit = nullptr,
        std::function<void(int, int)> onEdge = nullptr) const = 0;
    virtual std::string name() const = 0;
};

#endif // I_TRAVERSAL_H
