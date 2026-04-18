#ifndef GRAPH_INTERFACES_ISP_H
#define GRAPH_INTERFACES_ISP_H

// ISP-compliant split of IGraph interface
// Separates data access, operations, and metadata into focused interfaces

#include <list>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <QtCore/QPoint>

#include "Node.h"
#include "edge.h"

// IGraphData - Read-only data access (ISP)
class IGraphData {
public:
    virtual ~IGraphData() = default;
    virtual const std::list<Node>& getNodes() const = 0;
    virtual const std::vector<Edge>& getEdges() const = 0;
};

// IGraphDataMutable - Mutable data access (ISP)
class IGraphDataMutable {
public:
    virtual ~IGraphDataMutable() = default;
    virtual std::list<Node>& getNodes() = 0;
    virtual std::vector<Edge>& getEdges() = 0;
};

// IGraphOperations - Graph modification operations (ISP)
class IGraphOperations {
public:
    virtual ~IGraphOperations() = default;
    virtual void addNode(QPoint p) = 0;
    virtual void removeNode(int index) = 0;
    virtual void addEdge(Node &f, Node &s, int cost = 1) = 0;
    virtual void clear() = 0;
};

// IGraphMetadata - Graph type information (ISP)
class IGraphMetadata {
public:
    virtual ~IGraphMetadata() = default;
    virtual std::string_view getGraphType() const = 0;
};

// IGraphComplete - Combines all interfaces for backward compatibility
// Inherits from IGraphData for const access and defines non-const overloads directly
class IGraphComplete : public IGraphData,
                       public IGraphOperations,
                       public IGraphMetadata {
public:
    virtual ~IGraphComplete() = default;
    
    // Non-const overloads for mutable access (ISP - IGraphDataMutable functionality)
    // Note: const versions are inherited from IGraphData
    using IGraphData::getNodes;
    using IGraphData::getEdges;
    virtual std::list<Node>& getNodes() = 0;
    virtual std::vector<Edge>& getEdges() = 0;
};

// Helper structs for algorithms
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

#endif // GRAPH_INTERFACES_ISP_H
