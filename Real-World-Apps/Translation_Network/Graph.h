#pragma once
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
    Graph();
    virtual ~Graph();

    void addNode(const std::string& language);
    virtual void addEdge(Node& f, Node& s, int cost) = 0;  

    std::deque<Node>& getNodes();
    const std::deque<Node>& getNodes() const;
    std::vector<Edge>& getEdges();
    const std::vector<Edge>& getEdges() const;

    Node* findByLanguage(const std::string& language);
    const Node* findByLanguage(const std::string& language) const;
    Node* findByIndex(int index);
    const Node* findByIndex(int index) const;

    virtual std::string getGraphType() const = 0;
};