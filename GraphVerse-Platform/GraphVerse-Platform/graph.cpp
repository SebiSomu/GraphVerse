#include "graph.h"
#include <algorithm>
#include <ranges>
#include <unordered_set>
#include <QtCore/QPoint>

Graph::Graph() {
}

Graph::~Graph() = default;

void Graph::addNode(QPoint p) {
    Node n;
    n.setCoord(p);
    int maxIdx = 0;
    for(const auto& node : m_nodes) 
        maxIdx = std::max(node.getIndex(), maxIdx);
    n.setIndex(maxIdx + 1);
    m_nodes.push_back(n);
}

void Graph::removeNode(int index) {
    auto it = std::ranges::find(m_nodes, index, &Node::getIndex);
    if (it != m_nodes.end()) {
        m_nodes.erase(it);
        std::erase_if(m_edges, [index](const Edge& e){
            return e.getFirst().getIndex() == index || e.getSecond().getIndex() == index;
        });
    }
}

std::list<Node>& Graph::getNodes() { return m_nodes; }
const std::list<Node>& Graph::getNodes() const { return m_nodes; }
std::vector<Edge>& Graph::getEdges() { return m_edges; }
const std::vector<Edge>& Graph::getEdges() const { return m_edges; }

