#include "dijkstra_solver.h"
#include "graph_utils.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>

std::vector<PathStep> DijkstraSolver::solve(const Graph& graph, int startIdx, int endIdx,
                                             std::vector<int>& outPath) const
{
    auto adj = GraphUtils::buildWeightedAdjList(graph);
    std::unordered_map<int, int> dist, parent;
    std::unordered_set<int> settled;
    std::vector<PathStep> steps;

    for (const auto& n : graph.getNodes()) dist[n.getIndex()] = INF_COST;
    dist[startIdx] = 0; parent[startIdx] = -1;

    using T = std::pair<int, int>;
    std::priority_queue<T, std::vector<T>, std::greater<>> pq;
    pq.emplace(0, startIdx);

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (settled.count(u)) continue;
        settled.insert(u);
        steps.push_back({u, parent.count(u) ? parent[u] : -1, d, false});
        if (u == endIdx) break;
        for (auto& [v, w] : adj[u]) {
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                parent[v] = u;
                pq.emplace(dist[v], v);
            }
        }
    }

    outPath = GraphUtils::reconstructPath(endIdx, parent);
    GraphUtils::markFinalPath(steps, outPath);
    return steps;
}
