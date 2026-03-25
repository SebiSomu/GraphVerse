#ifndef GRAPH_INTERFACES_H
#define GRAPH_INTERFACES_H

#include <list>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <QtCore/QPoint>

#include "Node.h"
#include "edge.h"

class IGraph {
public:
    virtual ~IGraph() = default;
    virtual std::list<Node>& getNodes() = 0;
    virtual const std::list<Node>& getNodes() const = 0;
    virtual std::vector<Edge>& getEdges() = 0;
    virtual const std::vector<Edge>& getEdges() const = 0;
    virtual void addNode(QPoint p) = 0;
    virtual void removeNode(int index) = 0;
    virtual void addEdge(Node &f, Node &s, int cost = 1) = 0;
    virtual void clear() = 0;
    virtual std::string_view getGraphType() const = 0;
};

struct ComponentResult {
    int numComponents = 0;
    std::unordered_map<int, int> nodeToComponent;
};

struct CondensedGraph {
    std::list<Node> nodes;
    std::vector<Edge> edges;
};

struct CondensedResult {
    bool isShowing = false;
    CondensedGraph graph;
};

#endif // GRAPH_INTERFACES_H
