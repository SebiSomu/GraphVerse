#include "graph.h"

Graph::Graph() {
    m_nodes.reserve(500);
}

Graph::~Graph(){}

void Graph::addNode(QPoint p) {
    if(m_nodes.size() + 1 >= m_nodes.capacity()) {
        std::vector<Edge> tempEdges = m_edges;
        m_edges.clear();
        m_nodes.reserve(m_nodes.capacity() * 2);

        Node n;
        n.setCoord(p);
        n.setIndex(static_cast<int>(m_nodes.size() + 1));
        m_nodes.push_back(n);

        for(const auto& ed : tempEdges) {
            Node* first = nullptr;
            Node* second = nullptr;
            for(auto& node : m_nodes) {
                if(node.getIndex() == ed.getFirst().getIndex())  first = &node;
                if(node.getIndex() == ed.getSecond().getIndex()) second = &node;
            }
            if(first && second) {
                Edge newEdge(first, second);
                m_edges.push_back(newEdge);
            }
        }
    } else {
        Node n;
        n.setCoord(p);
        n.setIndex(static_cast<int>(m_nodes.size() + 1));
        m_nodes.push_back(n);
    }
}

std::vector<Node>& Graph::getNodes() { return m_nodes; }
const std::vector<Node>& Graph::getNodes() const { return m_nodes; }
std::vector<Edge>& Graph::getEdges() { return m_edges; }
const std::vector<Edge>& Graph::getEdges() const { return m_edges; }
