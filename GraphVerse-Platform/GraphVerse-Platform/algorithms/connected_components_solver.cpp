#include "connected_components_solver.h"
#include "graph_utils.h"
#include <stack>
#include <unordered_set>

int ConnectedComponentsSolver::solve(const IGraph& graph, std::unordered_map<int, int>& outComponentMap) const {
    outComponentMap.clear();
    if (graph.getNodes().empty()) return 0;

    auto adj = GraphUtils::buildSimpleAdjList(graph, false); // false for undirected (bi-directional)
    std::unordered_set<int> visited;
    int numComponents = 0;

    for (const auto& node : graph.getNodes()) {
        int startNode = node.getIndex();
        if (visited.find(startNode) == visited.end()) {
            // Iterative DFS
            std::stack<int> s;
            s.push(startNode);
            visited.insert(startNode);
            outComponentMap[startNode] = numComponents;

            while (!s.empty()) {
                int u = s.top(); s.pop();
                for (int v : adj[u]) {
                    if (visited.find(v) == visited.end()) {
                        visited.insert(v);
                        outComponentMap[v] = numComponents;
                        s.push(v);
                    }
                }
            }
            numComponents++;
        }
    }
    return numComponents;
}
