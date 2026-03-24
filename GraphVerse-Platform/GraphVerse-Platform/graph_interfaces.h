#ifndef GRAPH_INTERFACES_H
#define GRAPH_INTERFACES_H

#include <list>
#include <vector>
#include <string>
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
    virtual std::string getGraphType() const = 0;
};

class IComponentAnalysis {
public:
    virtual ~IComponentAnalysis() = default;
    virtual void setComponentData(int num, const std::unordered_map<int, int>& colors) = 0;
    virtual int getComponentColor(int nodeIndex) const = 0;
    virtual int getNumComponents() const = 0;
};

#endif // GRAPH_INTERFACES_H
