#ifndef I_MST_ALGORITHM_H
#define I_MST_ALGORITHM_H

#include "graph_algorithm_types.h"
#include "../graph.h"
#include <vector>
#include <string>

class IMSTAlgorithm {
public:
    virtual ~IMSTAlgorithm() = default;
    virtual std::vector<MSTStep> solve(const Graph& graph) const = 0;
    virtual std::string name() const = 0;
};

#endif // I_MST_ALGORITHM_H
