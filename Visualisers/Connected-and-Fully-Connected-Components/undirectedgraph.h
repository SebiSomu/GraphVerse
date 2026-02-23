#ifndef UNDIRECTEDGRAPH_H
#define UNDIRECTEDGRAPH_H
#include "graph.h"
#include <QDebug>

class UndirectedGraph : public Graph
{
public:
    UndirectedGraph();
    void addEdge(Node &f, Node &s) override;
    void drawEdge(QPainter& p) const override;
    std::string getGraphType() const override;
};

#endif // UNDIRECTEDGRAPH_H
