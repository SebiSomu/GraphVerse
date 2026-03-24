#include "graph.h"
#include <algorithm>
#include <unordered_set>
#include <QtCore/QPoint>

Graph::Graph() : m_numComponents(0) {
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

    m_componentsColors.clear();
    m_numComponents = 0;
}

void Graph::removeNode(int index) {
    auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [index](const Node& n){ return n.getIndex() == index; });
    if (it != m_nodes.end()) {
        m_nodes.erase(it);
        m_edges.erase(std::remove_if(m_edges.begin(), m_edges.end(), [index](const Edge& e){
            return e.getFirst().getIndex() == index || e.getSecond().getIndex() == index;
        }), m_edges.end());
        m_componentsColors.clear();
        m_numComponents = 0;
    }
}

std::list<Node>& Graph::getNodes() { return m_nodes; }
const std::list<Node>& Graph::getNodes() const { return m_nodes; }
std::vector<Edge>& Graph::getEdges() { return m_edges; }
const std::vector<Edge>& Graph::getEdges() const { return m_edges; }

void Graph::setComponentData(int num, const std::unordered_map<int, int>& colors) {
    m_numComponents = num;
    m_componentsColors = colors;
}

int Graph::getComponentColor(int nodeIndex) const {
    auto it = m_componentsColors.find(nodeIndex);
    return (it != m_componentsColors.end()) ? it->second : -1;
}

int Graph::getNumComponents() const { return m_numComponents; }

