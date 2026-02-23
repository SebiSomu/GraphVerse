#ifndef DIRECTEDGRAPH_H
#define DIRECTEDGRAPH_H
#include "graph.h"
#include <QLineF>
#include <QPolygonF>
#include <QDebug>
#include <QtMath>
#include <stack>
#include <unordered_set>
#include <unordered_map>

class DirectedGraph : public Graph
{
private:
    void fillOrder(int nodeIndex, std::unordered_set<int>& visited, std::stack<int>& Stack);
    void DFSUtil(int nodeIndex, std::unordered_set<int>& visited,
                 std::unordered_map<int, std::vector<int>>& transposeAdj,
                 int component);
    std::unordered_map<int, std::vector<int>> getTranspose();
    bool m_showCondensedGraph;
    std::vector<Node> m_condensedNodes;
    std::vector<Edge> m_condensedEdges;
    std::unordered_map<int, int> m_nodeToComponent;
    void buildCondensedGraph();
    void drawCondensedGraph(QPainter& p) const;

public:
    DirectedGraph();
    void addEdge(Node &f, Node &s) override;
    void drawEdge(QPainter& p) const override;
    std::string getGraphType() const override;
    void buildAdjacencyList() override;
    void findConnectedComponents() override;
    void toggleCondensedGraph();
    bool isShowingCondensedGraph() const;
};

#endif // DIRECTEDGRAPH_H
