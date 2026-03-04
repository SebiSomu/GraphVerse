#include "graph.h"
#include <stack>
#include <algorithm>

Graph::Graph() : m_numComponents(0) {
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
                if(node.getIndex() == ed.getFirst().getIndex()) first = &node;
                if(node.getIndex() == ed.getSecond().getIndex()) second = &node;
            }
            if(first && second) {
                m_edges.emplace_back(first, second, ed.getCost());
            }
        }
    } else {
        Node n;
        n.setCoord(p);
        n.setIndex(static_cast<int>(m_nodes.size() + 1));
        m_nodes.push_back(n);
    }
    m_componentsColors.clear();
    m_numComponents = 0;
}

std::vector<Node>& Graph::getNodes() { return m_nodes; }
const std::vector<Node>& Graph::getNodes() const { return m_nodes; }
std::vector<Edge>& Graph::getEdges() { return m_edges; }
const std::vector<Edge>& Graph::getEdges() const { return m_edges; }

void Graph::buildAdjacencyList() {
    m_adjacencyList.clear();
    for(const auto& node : m_nodes)
        m_adjacencyList[node.getIndex()] = std::vector<int>();
    for(const auto& edge : m_edges) {
        int firstIdx = edge.getFirst().getIndex();
        int secondIdx = edge.getSecond().getIndex();
        m_adjacencyList[firstIdx].push_back(secondIdx);
        m_adjacencyList[secondIdx].push_back(firstIdx);
    }
}

void Graph::DFS(int nodeIndex, int component, std::unordered_set<int>& visited) {
    std::stack<std::pair<int, size_t>> stack;
    stack.push({nodeIndex, 0});
    visited.insert(nodeIndex);
    m_componentsColors[nodeIndex - 1] = component;

    while(!stack.empty()) {
        int currentNode = stack.top().first;
        size_t& currentIndex = stack.top().second;
        const auto& neighbors = m_adjacencyList[currentNode];
        if(currentIndex < neighbors.size()) {
            int neighbor = neighbors[currentIndex];
            currentIndex++;
            if(visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                m_componentsColors[neighbor - 1] = component;
                stack.push({neighbor, 0});
            }
        } else {
            stack.pop();
        }
    }
}

void Graph::findConnectedComponents() {
    if(m_nodes.empty()) { m_numComponents = 0; return; }
    buildAdjacencyList();
    m_componentsColors.resize(m_nodes.size());
    std::fill(m_componentsColors.begin(), m_componentsColors.end(), -1);
    std::unordered_set<int> visited;
    m_numComponents = 0;
    for(const auto& node : m_nodes) {
        int nodeIndex = node.getIndex();
        if(visited.find(nodeIndex) == visited.end()) {
            DFS(nodeIndex, m_numComponents, visited);
            m_numComponents++;
        }
    }
}

int Graph::getComponentColor(int nodeIndex) const {
    if(nodeIndex < 1 || nodeIndex > (int)m_componentsColors.size()) return -1;
    return m_componentsColors[nodeIndex - 1];
}

int Graph::getNumComponents() const { return m_numComponents; }

void Graph::findStronglyConnectedComponents() {}

QColor Graph::getColorForComponent(int component) const {
    if (m_numComponents <= 0) return QColor(255, 255, 255);
    static const std::vector<QColor> baseColors = {
        QColor(255, 0, 0), QColor(255, 255, 0), QColor(0, 0, 255),
        QColor(128, 0, 128), QColor(0, 255, 0), QColor(255, 165, 0),
        QColor(165, 42, 42), QColor(255, 105, 180)
    };
    int baseColorsSize = static_cast<int>(baseColors.size());
    if (component < baseColorsSize) return baseColors[component];
    int extraComponent = component - baseColorsSize;
    int totalExtra = m_numComponents - baseColorsSize;
    if (totalExtra <= 0) return baseColors[0];
    float progression = static_cast<float>(extraComponent) / totalExtra;
    int colorCategory = extraComponent % 12;
    switch (colorCategory) {
    case 0: return QColor(120+(int)(135*progression), 20+(int)(180*progression), 20+(int)(180*progression));
    case 1: return QColor(180+(int)(60*progression), 180+(int)(60*progression), 50+(int)(50*progression));
    case 2: return QColor(30+(int)(40*progression), 80+(int)(100*progression), 150+(int)(80*progression));
    case 3: return QColor(100+(int)(80*progression), 30+(int)(40*progression), 120+(int)(80*progression));
    case 4: return QColor(30+(int)(40*progression), 120+(int)(100*progression), 80+(int)(80*progression));
    case 5: return QColor(180+(int)(60*progression), 100+(int)(80*progression), 30+(int)(40*progression));
    case 6: return QColor(0, 120+(int)(80*progression), 120+(int)(80*progression));
    case 7: return QColor(120+(int)(80*progression), 0, 80+(int)(100*progression));
    case 8: return QColor(160+(int)(60*progression), 140+(int)(60*progression), 100+(int)(60*progression));
    case 9: return QColor(0, 100+(int)(80*progression), 140+(int)(80*progression));
    case 10: return QColor(160+(int)(60*progression), 80+(int)(80*progression), 100+(int)(80*progression));
    case 11: return QColor(120+(int)(60*progression), 80+(int)(60*progression), 140+(int)(60*progression));
    default: return QColor(255, 0, 0);
    }
}
