#include "bfs_traversal.h"
#include "graph_utils.h"
#include <queue>
#include <unordered_set>

std::vector<TraversalStep> BFSTraversal::solve(const IGraph& graph, int startIndex, int stopAt) const
{
    bool isDirected = graph.getGraphType() == "directed";
    auto adj = GraphUtils::buildSimpleAdjList(graph, isDirected);
    
    std::unordered_map<int, int> visitedDist; // nodeIndex -> distance
    std::unordered_map<int, int> parents;    // nodeIndex -> parentIndex
    std::vector<TraversalStep> steps;
    std::queue<int> q;

    visitedDist[startIndex] = 0;
    parents[startIndex] = -1;
    q.push(startIndex);

    while (!q.empty()) {
        int cur = q.front(); q.pop();
        int dist = visitedDist[cur];
        
        steps.push_back({cur, parents[cur], dist});
        if (cur == stopAt) break;

        for (int nb : adj[cur]) {
            if (visitedDist.find(nb) == visitedDist.end()) {
                visitedDist[nb] = dist + 1;
                parents[nb] = cur;
                q.push(nb);
            }
        }
    }
    return steps;
}
