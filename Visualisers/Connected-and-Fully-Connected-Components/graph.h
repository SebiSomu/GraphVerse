#ifndef GRAPH_H
#define GRAPH_H
#include "edge.h"
#include <vector>
#include <QPainter>
#include <unordered_map>
#include <unordered_set>
#include <QColor>

class Graph
{
protected:
    std::vector<Node> m_nodes;
    std::vector<Edge> m_edges;
    std::vector<int> m_componentsColors;
    int m_numComponents;
    std::unordered_map<int, std::vector<int>> m_adjacencyList;
    virtual void buildAdjacencyList();
    void DFS(int nodeIndex, int component, std::unordered_set<int>& visited);

public:
    Graph();
    virtual ~Graph();
    void addNode(QPoint p);
    virtual void addEdge(Node &f, Node &s) = 0;
    virtual void drawEdge(QPainter& p) const = 0;
    std::vector<Node>& getNodes();
    const std::vector<Node>& getNodes() const;
    std::vector<Edge>& getEdges();
    const std::vector<Edge>& getEdges() const;
    virtual std::string getGraphType() const = 0;
    virtual void findConnectedComponents();
    virtual void findStronglyConnectedComponents();
    int getComponentColor(int nodeIndex) const;
    int getNumComponents() const;
    QColor getColorForComponent(int component) const;
};

#endif // GRAPH_H
