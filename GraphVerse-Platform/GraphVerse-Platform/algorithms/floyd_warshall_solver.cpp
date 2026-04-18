#include "floyd_warshall_solver.h"
#include "graph_utils.h"
#include <unordered_map>

std::vector<PathStep> FloydWarshallSolver::solve(const IGraphData& graph, int startIdx, int endIdx,
                                                  std::vector<int>& outPath) const
{
    std::vector<PathStep> steps;
    std::vector<int> ids;
    for (const auto& n : graph.getNodes()) ids.push_back(n.getIndex());
    int N = static_cast<int>(ids.size());

    std::unordered_map<int, int> pos;
    for (int i = 0; i < N; i++) pos[ids[i]] = i;

    std::vector<std::vector<int>> dist(N, std::vector<int>(N, INF_COST));
    std::vector<std::vector<int>> next(N, std::vector<int>(N, -1));
    for (int i = 0; i < N; i++) dist[i][i] = 0;

    for (const auto& ed : graph.getEdges()) {
        int u = pos[ed.getFirst().getIndex()], v = pos[ed.getSecond().getIndex()], w = ed.getCost();
        if (w < dist[u][v]) { dist[u][v] = w; next[u][v] = v; }
    }

    int si = pos[startIdx];
    for (int k = 0; k < N; k++)
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                if (dist[i][k] != INF_COST && dist[k][j] != INF_COST && dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    next[i][j] = next[i][k];
                    if (i == si) steps.push_back({ids[j], ids[k], dist[i][j], false});
                }

    outPath.clear();
    int u = pos[startIdx], v = pos[endIdx];
    if (dist[u][v] == INF_COST) return steps;
    outPath.push_back(startIdx);
    while (ids[u] != endIdx) { u = next[u][v]; if (u == -1) break; outPath.push_back(ids[u]); }
    GraphUtils::markFinalPath(steps, outPath);
    return steps;
}
