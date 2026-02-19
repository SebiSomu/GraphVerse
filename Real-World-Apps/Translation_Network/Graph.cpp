#include "Graph.h"
#include <deque>

Graph::Graph() {}
Graph::~Graph() {}

void Graph::addNode(const std::string& language) {
    int idx = static_cast<int>(m_nodes.size() + 1);
    m_nodes.emplace_back(idx, language);
}

Node* Graph::findByLanguage(const std::string& language) {
    for (auto& n : m_nodes)
        if (n.getLanguage() == language) return &n;
    return nullptr;
}

const Node* Graph::findByLanguage(const std::string& language) const {
    for (const auto& n : m_nodes)
        if (n.getLanguage() == language) return &n;
    return nullptr;
}

Node* Graph::findByIndex(int index) {
    for (auto& n : m_nodes)
        if (n.getIndex() == index) return &n;
    return nullptr;
}

const Node* Graph::findByIndex(int index) const {
    for (const auto& n : m_nodes)
        if (n.getIndex() == index) return &n;
    return nullptr;
}

std::deque<Node>& Graph::getNodes() { return m_nodes; }
const std::deque<Node>& Graph::getNodes() const { return m_nodes; }
std::vector<Edge>& Graph::getEdges() { return m_edges; }
const std::vector<Edge>& Graph::getEdges() const { return m_edges; }