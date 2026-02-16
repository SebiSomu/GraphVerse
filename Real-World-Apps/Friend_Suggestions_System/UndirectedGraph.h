#pragma once
#include "Graph.h"
#include <unordered_map>
#include <vector>
#include <functional>

struct BFSResult {
    std::unordered_map<int, int> dist;
    std::unordered_map<int, int> parent;
};

class UndirectedGraph : public Graph
{
public:
    UndirectedGraph() = default;

    void addEdge(Node& f, Node& s) override;

    BFSResult bfs(const std::string& startName,
        int maxDepth = -1,
        std::function<void(const Node&, int)> onVisit = nullptr) const;

    std::unordered_map<int, std::vector<int>> buildAdjList() const;
    std::string getGraphType() const override;
};