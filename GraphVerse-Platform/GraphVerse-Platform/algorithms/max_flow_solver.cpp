#include "max_flow_solver.h"
#include <queue>
#include <algorithm>
#include <unordered_map>

MaxFlowResult MaxFlowSolver::solve(const IGraph& graph, int sourceIdx, int sinkIdx) {
    const auto& nodes = graph.getNodes();
    const auto& edges = graph.getEdges();

    // Map node indices to 0..N-1 for internal processing
    std::unordered_map<int, int> idToInternal;
    std::unordered_map<int, int> internalToId;
    int n = 0;
    for (const auto& node : nodes) {
        idToInternal[node.getIndex()] = n;
        internalToId[n] = node.getIndex();
        n++;
    }

    if (!idToInternal.contains(sourceIdx) || !idToInternal.contains(sinkIdx)) {
        return { 0, {} };
    }

    int s = idToInternal[sourceIdx];
    int t = idToInternal[sinkIdx];

    std::vector<std::vector<int>> capacity(n, std::vector<int>(n, 0));
    for (const auto& edge : edges) {
        int u = idToInternal[edge.getFirst().getIndex()];
        int v = idToInternal[edge.getSecond().getIndex()];
        capacity[u][v] += edge.getCost(); // Support multiple edges or capacity
    }

    std::vector<std::vector<int>> residual = capacity;
    std::vector<int> parent(n, -1);

    auto bfs = [&]() -> bool {
        std::fill(parent.begin(), parent.end(), -1);
        std::queue<int> q;
        q.push(s);
        parent[s] = s;
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (int v = 0; v < n; ++v) {
                if (parent[v] == -1 && residual[u][v] > 0) {
                    parent[v] = u;
                    if (v == t) return true;
                    q.push(v);
                }
            }
        }
        return false;
    };

    int maxFlow = 0;
    while (bfs()) {
        int pathFlow = 1e9;
        for (int v = t; v != s; v = parent[v]) {
            int u = parent[v];
            pathFlow = std::min(pathFlow, residual[u][v]);
        }
        for (int v = t; v != s; v = parent[v]) {
            int u = parent[v];
            residual[u][v] -= pathFlow;
            residual[v][u] += pathFlow;
        }
        maxFlow += pathFlow;
    }

    MaxFlowResult result;
    result.maxFlow = maxFlow;

    // Identify matched edges (where flow was sent)
    for (const auto& edge : edges) {
        int uIdx = edge.getFirst().getIndex();
        int vIdx = edge.getSecond().getIndex();
        int u = idToInternal[uIdx];
        int v = idToInternal[vIdx];
        
        // If initial capacity was 1 (bipartite case) and residual is 0, it's matched
        if (capacity[u][v] > 0 && residual[u][v] < capacity[u][v]) {
            // For Bipartite Matching specifically, we might want to track how much flow
            // But here we just return the pair
            result.matchedEdges.emplace_back(uIdx, vIdx);
        }
    }

    return result;
}
