#ifndef GRAPH_UTILS_H
#define GRAPH_UTILS_H

#include "graph_algorithm_types.h"
#include "../graph_interfaces.h"

#include <unordered_map>
#include <vector>
#include <algorithm>

namespace GraphUtils {

// Build unweighted adjacency list
inline std::unordered_map<int, std::vector<int>> buildSimpleAdjList(const IGraph& graph, bool directed = true) {

    std::unordered_map<int, std::vector<int>> adj;
    for (const auto& n : graph.getNodes()) adj[n.getIndex()] = {};
    for (const auto& ed : graph.getEdges()) {
        int u = ed.getFirst().getIndex();
        int v = ed.getSecond().getIndex();
        adj[u].push_back(v);
        if (!directed) adj[v].push_back(u);
    }
    return adj;
}

// Build weighted adjacency list (directed: one-way edges only)
inline std::unordered_map<int, std::vector<std::pair<int, int>>> buildWeightedAdjList(const IGraph& graph) {

    std::unordered_map<int, std::vector<std::pair<int, int>>> adj;
    for (const auto& n : graph.getNodes()) adj[n.getIndex()] = {};
    for (const auto& ed : graph.getEdges())
        adj[ed.getFirst().getIndex()].emplace_back(ed.getSecond().getIndex(), ed.getCost());
    return adj;
}

// Reconstruct path from parent map
inline std::vector<int> reconstructPath(int end, const std::unordered_map<int, int>& parent) {
    std::vector<int> path;
    for (int cur = end; cur != -1; ) {
        path.push_back(cur);
        auto it = parent.find(cur);
        cur = (it != parent.end()) ? it->second : -1;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

// Mark final path steps
inline void markFinalPath(std::vector<PathStep>& steps, const std::vector<int>& outPath) {
    for (auto& s : steps)
        for (int idx : outPath)
            if (s.nodeIndex == idx) { s.isFinal = true; break; }
}

} // namespace GraphUtils

#endif // GRAPH_UTILS_H
