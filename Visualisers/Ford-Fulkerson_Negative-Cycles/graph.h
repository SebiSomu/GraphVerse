#ifndef GRAPH_H
#define GRAPH_H
#include "edge.h"
#include <vector>
#include <QPainter>
#include <QPointF>

class Graph
{
protected:
    std::vector<Node> m_nodes;
    std::vector<Edge> m_edges;

public:
    Graph();
    virtual ~Graph();
    void addNode(QPointF p);
    virtual void addEdge(Node &f, Node &s, int capacity = 1) = 0;
    virtual void drawEdge(QPainter& p) const = 0;
    std::vector<Node>& getNodes();
    const std::vector<Node>& getNodes() const;
    std::vector<Edge>& getEdges();
    const std::vector<Edge>& getEdges() const;
    virtual std::string getGraphType() const = 0;
};

#endif // GRAPH_H
