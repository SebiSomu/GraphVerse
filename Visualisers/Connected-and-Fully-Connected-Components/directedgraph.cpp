#include "directedgraph.h"
#include <string>

DirectedGraph::DirectedGraph() : m_showCondensedGraph(false) {}

void DirectedGraph::addEdge(Node &f, Node &s)
{
    m_edges.emplace_back(&f, &s);
    m_componentsColors.clear();
    m_numComponents = 0;
    m_showCondensedGraph = false;
}

void DirectedGraph::buildAdjacencyList() {
    m_adjacencyList.clear();

    for(const auto& node : m_nodes) {
        m_adjacencyList[node.getIndex()] = std::vector<int>();
    }

    for(const auto& edge : m_edges) {
        int from = edge.getFirst().getIndex();
        int to = edge.getSecond().getIndex();
        m_adjacencyList[from].push_back(to);
    }
}

void DirectedGraph::fillOrder(int nodeIndex, std::unordered_set<int>& visited, std::stack<int>& Stack)
{
    std::stack<std::pair<int, size_t>> dfsStack;

    dfsStack.push({nodeIndex, 0});
    visited.insert(nodeIndex);

    while(!dfsStack.empty()) {
        int currentNode = dfsStack.top().first;
        size_t& currentIndex = dfsStack.top().second;

        auto it = m_adjacencyList.find(currentNode);
        if(it == m_adjacencyList.end()) {
            Stack.push(currentNode);
            dfsStack.pop();
            continue;
        }

        const auto& neighbors = it->second;

        if(currentIndex < neighbors.size()) {
            int neighbor = neighbors[currentIndex];
            currentIndex++;

            if(visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                dfsStack.push({neighbor, 0});
            }
        }
        else {
            Stack.push(currentNode);
            dfsStack.pop();
        }
    }
}

void DirectedGraph::DFSUtil(int nodeIndex, std::unordered_set<int>& visited,
                            std::unordered_map<int, std::vector<int>>& transposeAdj,
                            int component)
{
    std::stack<std::pair<int, size_t>> stack;

    stack.push({nodeIndex, 0});
    visited.insert(nodeIndex);

    if(nodeIndex >= 1 && nodeIndex <= static_cast<int>(m_componentsColors.size())) {
        m_componentsColors[nodeIndex - 1] = component;
    }

    while(!stack.empty()) {
        int currentNode = stack.top().first;
        size_t& currentIndex = stack.top().second;

        auto it = transposeAdj.find(currentNode);
        if(it == transposeAdj.end()) {
            stack.pop();
            continue;
        }

        const auto& neighbors = it->second;

        if(currentIndex < neighbors.size()) {
            int neighbor = neighbors[currentIndex];
            currentIndex++;

            if(visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                if(neighbor >= 1 && neighbor <= static_cast<int>(m_componentsColors.size())) {
                    m_componentsColors[neighbor - 1] = component;
                }
                stack.push({neighbor, 0});
            }
        }
        else {
            stack.pop();
        }
    }
}

std::unordered_map<int, std::vector<int>> DirectedGraph::getTranspose()
{
    std::unordered_map<int, std::vector<int>> transposeAdj;

    for(const auto& node : m_nodes) {
        transposeAdj[node.getIndex()] = std::vector<int>();
    }

    for(const auto& edge : m_edges) {
        int from = edge.getFirst().getIndex();
        int to = edge.getSecond().getIndex();
        transposeAdj[to].push_back(from);
    }

    return transposeAdj;
}

void DirectedGraph::findConnectedComponents()
{
    if(m_nodes.empty()) {
        m_numComponents = 0;
        return;
    }

    buildAdjacencyList();

    m_componentsColors.resize(m_nodes.size());
    std::fill(m_componentsColors.begin(), m_componentsColors.end(), -1);

    std::stack<int> finishTimeStack;
    std::unordered_set<int> visited;

    for(const auto& node : m_nodes) {
        int nodeIndex = node.getIndex();
        if(visited.find(nodeIndex) == visited.end()) {
            fillOrder(nodeIndex, visited, finishTimeStack);
        }
    }

    auto transposeAdj = getTranspose();

    visited.clear();
    m_numComponents = 0;

    while(!finishTimeStack.empty()) {
        int nodeIndex = finishTimeStack.top();
        finishTimeStack.pop();

        if(visited.find(nodeIndex) == visited.end()) {
            DFSUtil(nodeIndex, visited, transposeAdj, m_numComponents);
            m_numComponents++;
        }
    }

    buildCondensedGraph();
    m_showCondensedGraph = true;
}

void DirectedGraph::drawEdge(QPainter &p) const
{
    const double arrowSize = 15.0;
    const double nodeRadius = 10.0;

    if(m_showCondensedGraph && m_numComponents > 0) {
        drawCondensedGraph(p);
        return;
    }

    bool componentsFound = (m_numComponents > 0);

    for(const auto &ed : m_edges)
    {
        QPointF start(ed.getFirst().getX(), ed.getFirst().getY());
        QPointF end(ed.getSecond().getX(), ed.getSecond().getY());
        QLineF line(start, end);
        double angle = std::atan2(line.dy(), line.dx());

        QPointF adjustedStart = start + QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));
        QPointF adjustedEnd = end - QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));

        if(componentsFound) {
            int comp = getComponentColor(ed.getFirst().getIndex());
            p.setPen(QPen(getColorForComponent(comp), 2));
        } else {
            p.setPen(QPen(Qt::white, 1));
        }

        p.drawLine(adjustedStart, adjustedEnd);

        QPointF arrowP1 = adjustedEnd - QPointF(arrowSize * cos(angle - M_PI / 6),
                                                arrowSize * sin(angle - M_PI / 6));
        QPointF arrowP2 = adjustedEnd - QPointF(arrowSize * cos(angle + M_PI / 6),
                                                arrowSize * sin(angle + M_PI / 6));

        p.drawLine(adjustedEnd, arrowP1);
        p.drawLine(adjustedEnd, arrowP2);
    }

    for(const auto &n : m_nodes)
    {
        QRect r(n.getX() - nodeRadius, n.getY() - nodeRadius, nodeRadius*2, nodeRadius*2);

        if(componentsFound) {
            int comp = getComponentColor(n.getIndex());
            QColor nodeColor = getColorForComponent(comp);
            p.setPen(QPen(nodeColor, 2));
            p.setBrush(QBrush(nodeColor.lighter(160)));
        } else {
            p.setPen(QPen(Qt::black, 1));
            p.setBrush(QBrush(Qt::white));
        }

        p.drawEllipse(r);

        p.setPen(Qt::black);
        QString s = QString::number(n.getIndex());
        p.drawText(r, Qt::AlignCenter, s);
    }
}

std::string DirectedGraph::getGraphType() const
{
    return "Directed";
}

void DirectedGraph::toggleCondensedGraph()
{
    m_showCondensedGraph = !m_showCondensedGraph;
}

bool DirectedGraph::isShowingCondensedGraph() const
{
    return m_showCondensedGraph;
}

void DirectedGraph::buildCondensedGraph()
{
    m_condensedNodes.clear();
    m_condensedEdges.clear();
    m_nodeToComponent.clear();

    if(m_numComponents == 0)
        return;

    for(const auto& node : m_nodes) {
        int comp = getComponentColor(node.getIndex());
        m_nodeToComponent[node.getIndex()] = comp;
    }

    std::unordered_map<int, std::vector<int>> componentNodes;
    for(const auto& node : m_nodes) {
        int comp = getComponentColor(node.getIndex());
        componentNodes[comp].push_back(node.getIndex());
    }

    std::unordered_map<int, QPoint> componentPositions;
    for(const auto& pair : componentNodes) {
        int comp = pair.first;
        const auto& nodes = pair.second;

        int sumX = 0, sumY = 0;
        for(int nodeIdx : nodes) {
            for(const auto& node : m_nodes) {
                if(node.getIndex() == nodeIdx) {
                    sumX += node.getX();
                    sumY += node.getY();
                    break;
                }
            }
        }

        QPoint center(sumX / static_cast<int>(nodes.size()), sumY / static_cast<int>(nodes.size()));
        componentPositions[comp] = center;
    }

    for(int comp = 0; comp < m_numComponents; comp++) {
        if(componentPositions.find(comp) != componentPositions.end()) {
            Node condensedNode;
            condensedNode.setCoord(componentPositions[comp]);
            condensedNode.setIndex(comp + 1);
            m_condensedNodes.push_back(condensedNode);
        }
    }

    std::unordered_set<std::string> addedEdges;

    for(const auto& edge : m_edges) {
        int fromNode = edge.getFirst().getIndex();
        int toNode = edge.getSecond().getIndex();

        int fromComp = m_nodeToComponent[fromNode];
        int toComp = m_nodeToComponent[toNode];

        if(fromComp != toComp) {
            std::string edgeKey = std::to_string(fromComp) + "," + std::to_string(toComp);
            if(addedEdges.find(edgeKey) == addedEdges.end()) {
                Node* fromCondensed = nullptr;
                Node* toCondensed = nullptr;

                for(auto& node : m_condensedNodes) {
                    if(node.getIndex() == fromComp + 1) {
                        fromCondensed = &node;
                    }
                    if(node.getIndex() == toComp + 1) {
                        toCondensed = &node;
                    }
                }

                if(fromCondensed && toCondensed) {
                    m_condensedEdges.emplace_back(fromCondensed, toCondensed);
                    addedEdges.insert(edgeKey);
                }
            }
        }
    }
}

void DirectedGraph::drawCondensedGraph(QPainter& p) const
{
    const double arrowSize = 15.0;
    const int nodeWidth = 80;
    const int nodeHeight = 40;
    const double a = nodeWidth / 2.0;
    const double b = nodeHeight / 2.0;

    for(const auto& ed : m_condensedEdges) {
        QPointF startCenter(ed.getFirst().getX(), ed.getFirst().getY());
        QPointF endCenter(ed.getSecond().getX(), ed.getSecond().getY());
        QPointF direction = endCenter - startCenter;
        double length = std::sqrt(direction.x() * direction.x() + direction.y() * direction.y());

        if (length > 0) {
            QPointF unitDirection = direction / length;

            double t1 = 1.0 / std::sqrt( (unitDirection.x() * unitDirection.x()) / (a * a) +
                                        (unitDirection.y() * unitDirection.y()) / (b * b) );
            QPointF adjustedStart = startCenter + t1 * unitDirection;

            QPointF reverseDirection = startCenter - endCenter;
            QPointF unitReverseDirection = reverseDirection / length;
            double t2 = 1.0 / std::sqrt( (unitReverseDirection.x() * unitReverseDirection.x()) / (a * a) +
                                        (unitReverseDirection.y() * unitReverseDirection.y()) / (b * b) );
            QPointF adjustedEnd = endCenter + t2 * unitReverseDirection;

            QLineF line(adjustedStart, adjustedEnd);
            double angle = std::atan2(line.dy(), line.dx());

            int comp = ed.getFirst().getIndex() - 1;
            p.setPen(QPen(getColorForComponent(comp), 3));
            p.drawLine(adjustedStart, adjustedEnd);

            QPointF arrowP1 = adjustedEnd - QPointF(arrowSize * std::cos(angle - M_PI / 6),
                                                    arrowSize * std::sin(angle - M_PI / 6));
            QPointF arrowP2 = adjustedEnd - QPointF(arrowSize * std::cos(angle + M_PI / 6),
                                                    arrowSize * std::sin(angle + M_PI / 6));

            p.drawLine(adjustedEnd, arrowP1);
            p.drawLine(adjustedEnd, arrowP2);
        }
    }

    for(const auto& n : m_condensedNodes) {
        QRect r(n.getX() - nodeWidth / 2, n.getY() - nodeHeight / 2, nodeWidth, nodeHeight);
        int comp = n.getIndex() - 1;
        QColor nodeColor = getColorForComponent(comp);

        p.setPen(QPen(nodeColor, 3));
        p.setBrush(QBrush(nodeColor.lighter(180)));
        p.drawEllipse(r);

        p.setPen(Qt::black);
        QString label;
        bool first = true;
        for(const auto& origNode : m_nodes) {
            if(getComponentColor(origNode.getIndex()) == comp) {
                if(!first) label += ",";
                label += QString::number(origNode.getIndex());
                first = false;
            }
        }

        QFont font = p.font();
        font.setPointSize(10);
        font.setBold(true);
        p.setFont(font);
        p.drawText(r, Qt::AlignCenter, label);
    }
}
