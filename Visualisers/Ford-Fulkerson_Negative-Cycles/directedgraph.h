#ifndef DIRECTEDGRAPH_H
#define DIRECTEDGRAPH_H
#include "graph.h"
#include <QLineF>
#include <QPolygonF>
#include <QDebug>
#include <QtMath>

class DirectedGraph : public Graph
{
public:
    DirectedGraph();
    void addEdge(Node &f, Node &s, int capacity = 1) override;
    void drawEdge(QPainter& p) const override;
    std::string getGraphType() const override;
};

#endif // DIRECTEDGRAPH_H
