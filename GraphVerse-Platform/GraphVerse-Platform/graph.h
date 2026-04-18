#ifndef GRAPH_H
#define GRAPH_H
#include "edge.h"
#include "graph_interfaces_isp.h"  // ISP-compliant interfaces
#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <string>

// Graph now inherits from IGraphComplete (ISP-compliant)
// This provides backward compatibility while enabling interface segregation
class Graph : public IGraphComplete
{
protected:
    std::list<Node> m_nodes;
    std::vector<Edge> m_edges;

public:
    Graph();
    virtual ~Graph();
    void addNode(QPoint p) override;
    void removeNode(int index) override;
    void addEdge(Node &f, Node &s, int cost = 1) override = 0;
    void clear() override;

    std::list<Node>& getNodes() override;
    const std::list<Node>& getNodes() const override;
    std::vector<Edge>& getEdges() override;
    const std::vector<Edge>& getEdges() const override;
    std::string_view getGraphType() const override = 0;
};

#endif // GRAPH_H
