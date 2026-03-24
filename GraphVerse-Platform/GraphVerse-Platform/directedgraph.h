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
private:
    // Condensed graph state (visual feature, stays here)
    bool m_showCondensedGraph;
    CondensedGraph m_condensed;
    std::unordered_map<int, int> m_nodeToComponent;

public:
    DirectedGraph();
    void addEdge(Node &f, Node &s, int cost = 1) override;
    std::string getGraphType() const override;
    void buildCondensedGraph();
    void toggleCondensedGraph();
    bool isShowingCondensedGraph() const;
    const CondensedGraph& getCondensedGraph() const { return m_condensed; }
};

#endif // DIRECTEDGRAPH_H
