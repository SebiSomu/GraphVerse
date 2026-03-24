#include "bfs_traversal.h"
#include "graph_utils.h"
#include <queue>
#include <unordered_set>

std::vector<int> BFSTraversal::solve(const Graph& graph, int startIndex, int stopAt,
                                      std::function<void(int)> onVisit,
                                      std::function<void(int, int)> onEdge) const
{
    auto adj = GraphUtils::buildSimpleAdjList(graph);
    std::unordered_set<int> visited;
    std::vector<int> order;
    std::queue<int> q;

    visited.insert(startIndex);
    q.push(startIndex);

    while (!q.empty()) {
        int cur = q.front(); q.pop();
        order.push_back(cur);
        if (onVisit) onVisit(cur);
        if (cur == stopAt) break;
        for (int nb : adj[cur]) {
            if (!visited.count(nb)) {
                visited.insert(nb);
                if (onEdge) onEdge(cur, nb);
                q.push(nb);
            }
        }
    }
    return order;
}
