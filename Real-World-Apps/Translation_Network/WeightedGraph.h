#pragma once
#include "Graph.h"
#include <vector>

class UnionFind {
    std::vector<int> parent, rank;
public:
    UnionFind(int n);
    int find(int x);
    bool unite(int x, int y);
};

class WeightedGraph : public Graph
{
public:
    WeightedGraph();

    void addEdge(Node& f, Node& s, int cost) override;
    std::vector<Edge> kruskalMST() const;
    std::string getGraphType() const override;
};