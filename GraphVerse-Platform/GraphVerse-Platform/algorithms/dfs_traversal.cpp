#include "dfs_traversal.h"
#include "graph_utils.h"
#include <stack>
#include <unordered_set>

std::vector<int> DFSTraversal::solve(const Graph& graph, int startIndex, int stopAt,
                                      std::function<void(int)> onVisit,
                                      std::function<void(int, int)> onEdge) const
{
    auto adj = GraphUtils::buildSimpleAdjList(graph);
    std::unordered_set<int> visited;
    std::vector<int> order;
    std::stack<std::pair<int, int>> stkWithParent;

    stkWithParent.push({startIndex, -1});
    visited.insert(startIndex);

    while (!stkWithParent.empty()) {
        auto [cur, par] = stkWithParent.top(); stkWithParent.pop();
        order.push_back(cur);
        if (onVisit) onVisit(cur);
        if (onEdge && par != -1) onEdge(par, cur);
        if (cur == stopAt) break;
        auto& nbList = adj[cur];
        for (int i = static_cast<int>(nbList.size()) - 1; i >= 0; i--) {
            int nb = nbList[i];
            if (!visited.count(nb)) {
                visited.insert(nb);
                stkWithParent.push({nb, cur});
            }
        }
    }
    return order;
}
