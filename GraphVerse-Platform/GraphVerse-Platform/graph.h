#ifndef GRAPH_H
#define GRAPH_H
#include "edge.h"
#include "graph_interfaces.h"
#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <string>

class Graph : public IGraph, public IComponentAnalysis
{
protected:
    std::list<Node> m_nodes;
    std::vector<Edge> m_edges;
    int m_numComponents;
    std::unordered_map<int, int> m_componentsColors;

public:
    Graph();
    virtual ~Graph();
    void addNode(QPoint p) override;
    void removeNode(int index) override;
    void addEdge(Node &f, Node &s, int cost = 1) override = 0;

    std::list<Node>& getNodes() override;
    const std::list<Node>& getNodes() const override;
    std::vector<Edge>& getEdges() override;
    const std::vector<Edge>& getEdges() const override;
    std::string getGraphType() const override = 0;
    void setComponentData(int num, const std::unordered_map<int, int>& colors) override;
    int getComponentColor(int nodeIndex) const override;
    int getNumComponents() const override;
};

#endif // GRAPH_H
