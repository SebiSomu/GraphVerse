#ifndef UNDIRECTEDGRAPH_H
#define UNDIRECTEDGRAPH_H
#include "graph.h"
#include <functional>
#include <unordered_map>

// One animation step for MST visualization
struct MSTStep {
    int fromIndex;
    int toIndex;
    int cost;
    bool accepted;   // true = added to MST (green), false = rejected (red)
};

class UndirectedGraph : public Graph
{
public:
    UndirectedGraph();
    void addEdge(Node &f, Node &s, int cost = 1) override;
    void drawEdge(QPainter& p) const override;
    std::string getGraphType() const override;

    std::vector<MSTStep> kruskal() const;
    std::vector<MSTStep> prim()    const;
    std::vector<MSTStep> boruvka() const;
};

#endif // UNDIRECTEDGRAPH_H
