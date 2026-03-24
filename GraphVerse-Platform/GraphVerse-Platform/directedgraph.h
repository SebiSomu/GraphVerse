#ifndef DIRECTEDGRAPH_H
#define DIRECTEDGRAPH_H
#include "graph.h"
#include "algorithms/condensed_graph_solver.h"
#include <QLineF>
#include <QPolygonF>
#include <QtMath>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class DirectedGraph : public Graph
{
public:
    DirectedGraph();
    void addEdge(Node &f, Node &s, int cost = 1) override;
    std::string_view getGraphType() const override;
};

#endif // DIRECTEDGRAPH_H
