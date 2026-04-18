#include "bellman_ford_solver.h"
#include "graph_utils.h"
#include <unordered_map>

std::vector<PathStep> BellmanFordSolver::solve(const IGraphData& graph, int startIdx, int endIdx,
                                                std::vector<int>& outPath) const
{
    std::unordered_map<int, int> dist, parent;
    std::vector<PathStep> steps;

    for (const auto& n : graph.getNodes()) dist[n.getIndex()] = INF_COST;
    dist[startIdx] = 0; parent[startIdx] = -1;

    int N = static_cast<int>(graph.getNodes().size());
    for (int round = 0; round < N - 1; ++round) {
        bool updated = false;
        for (const auto& ed : graph.getEdges()) {
            int u = ed.getFirst().getIndex(), v = ed.getSecond().getIndex(), w = ed.getCost();
            if (dist[u] != INF_COST && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                parent[v] = u;
                steps.push_back({v, u, dist[v], false});
                updated = true;
            }
        }
        if (!updated) break;
    }

    outPath = GraphUtils::reconstructPath(endIdx, parent);
    GraphUtils::markFinalPath(steps, outPath);
    return steps;
}
