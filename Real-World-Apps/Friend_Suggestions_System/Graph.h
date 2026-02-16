#pragma once
#include "Node.h"
#include "Edge.h"
#include <vector>
#include <deque>
#include <string>

class Graph
{
protected:
    std::deque<Node> m_nodes;  
    std::vector<Edge> m_edges;

public:
    Graph() = default;
    virtual ~Graph();

    void addNode(const std::string& name);
    virtual void addEdge(Node& f, Node& s) = 0;

    std::deque<Node>& getNodes();
    const std::deque<Node>& getNodes() const;
    std::vector<Edge>& getEdges();
    const std::vector<Edge>& getEdges() const;

    Node* findByName(const std::string& name);
    const Node* findByName(const std::string& name) const;
    Node* findByIndex(int index);
    const Node* findByIndex(int index) const;

    virtual std::string getGraphType() const = 0;
};