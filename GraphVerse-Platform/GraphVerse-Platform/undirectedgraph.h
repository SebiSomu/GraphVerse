#ifndef UNDIRECTEDGRAPH_H
#define UNDIRECTEDGRAPH_H
#include "graph.h"

class UndirectedGraph : public Graph
{
public:
    UndirectedGraph();
    void addEdge(Node &f, Node &s, int cost = 1) override;
    std::string getGraphType() const override;
};

#endif // UNDIRECTEDGRAPH_H
