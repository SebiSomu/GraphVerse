#include "directedgraph.h"
#include <QQueue>
#include <QStack>
#include <queue>
#include <algorithm>
#include <cmath>
#include <string>
#include <QPainter>
#include <QLineF>
#include <QRect>
#include <QColor>

DirectedGraph::DirectedGraph() : m_showCondensedGraph(false) {}

void DirectedGraph::addEdge(Node &f, Node &s, int cost)
{
    m_edges.emplace_back(&f, &s, cost);
    m_componentsColors.clear();
    m_numComponents = 0;
    m_showCondensedGraph = false;
}

// ─── Drawing ───

void DirectedGraph::drawEdge(QPainter &p) const
{
    const double arrowSize = 15.0;
    const double nodeRadius = 10.0;

    if(m_showCondensedGraph && m_numComponents > 0) {
        drawCondensedGraph(p);
        return;
    }

    bool componentsFound = (m_numComponents > 0);

    for(const auto &ed : m_edges) {
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
        p.drawLine(adjustedEnd, adjustedEnd - QPointF(arrowSize * cos(angle - M_PI/6), arrowSize * sin(angle - M_PI/6)));
        p.drawLine(adjustedEnd, adjustedEnd - QPointF(arrowSize * cos(angle + M_PI/6), arrowSize * sin(angle + M_PI/6)));
    }

    for(const auto &n : m_nodes) {
        QRect r(n.getX() - static_cast<int>(nodeRadius), n.getY() - static_cast<int>(nodeRadius), static_cast<int>(nodeRadius)*2, static_cast<int>(nodeRadius)*2);
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
        p.drawText(r, Qt::AlignCenter, QString::number(n.getIndex()));
    }
}

std::string DirectedGraph::getGraphType() const { return "Directed"; }

// ─── Adjacency lists ───

void DirectedGraph::buildAdjacencyList() {
    m_adjacencyList.clear();
    for(const auto& node : m_nodes) m_adjacencyList[node.getIndex()] = {};
    for(const auto& edge : m_edges) {
        int from = edge.getFirst().getIndex();
        int to = edge.getSecond().getIndex();
        m_adjacencyList[from].push_back(to);
    }
}

std::unordered_map<int, std::vector<int>> DirectedGraph::buildSimpleAdjList() const
{
    std::unordered_map<int, std::vector<int>> adj;
    for(const auto& n  : m_nodes) adj[n.getIndex()] = {};
    for(const auto& ed : m_edges)
        adj[ed.getFirst().getIndex()].push_back(ed.getSecond().getIndex());
    return adj;
}

std::unordered_map<int, std::vector<std::pair<int,int>>> DirectedGraph::buildWeightedAdjList() const
{
    std::unordered_map<int, std::vector<std::pair<int,int>>> adj;
    for(const auto& n  : m_nodes) adj[n.getIndex()] = {};
    for(const auto& ed : m_edges)
        adj[ed.getFirst().getIndex()].emplace_back(ed.getSecond().getIndex(), ed.getCost());
    return adj;
}

// ─── BFS / DFS ───

std::vector<int> DirectedGraph::bfs(int startIndex, int stopAt,
                                    std::function<void(int)> onVisit,
                                    std::function<void(int,int)> onEdge) const
{
    auto adj = buildSimpleAdjList();
    std::unordered_set<int> visited;
    std::vector<int> order;
    QQueue<int> q;
    visited.insert(startIndex);
    q.enqueue(startIndex);
    while(!q.isEmpty()) {
        int cur = q.dequeue();
        order.push_back(cur);
        if(onVisit) onVisit(cur);
        if(cur == stopAt) break;
        for(int nb : adj[cur]) {
            if(!visited.count(nb)) {
                visited.insert(nb);
                if(onEdge) onEdge(cur, nb);
                q.enqueue(nb);
            }
        }
    }
    return order;
}

std::vector<int> DirectedGraph::dfs(int startIndex, int stopAt,
                                    std::function<void(int)> onVisit,
                                    std::function<void(int,int)> onEdge) const
{
    auto adj = buildSimpleAdjList();
    std::unordered_set<int> visited;
    std::vector<int> order;
    QStack<std::pair<int,int>> stkWithParent;
    stkWithParent.push({startIndex, -1});
    visited.insert(startIndex);
    while(!stkWithParent.isEmpty()) {
        auto [cur, par] = stkWithParent.top(); stkWithParent.pop();
        order.push_back(cur);
        if(onVisit) onVisit(cur);
        if(onEdge && par != -1) onEdge(par, cur);
        if(cur == stopAt) break;
        auto& nbList = adj[cur];
        for(int i = static_cast<int>(nbList.size()) - 1; i >= 0; i--) {
            int nb = nbList[i];
            if(!visited.count(nb)) {
                visited.insert(nb);
                stkWithParent.push({nb, cur});
            }
        }
    }
    return order;
}

// ─── SCC (Kosaraju) ───

void DirectedGraph::fillOrder(int nodeIndex, std::unordered_set<int>& visited, std::stack<int>& Stack)
{
    std::stack<std::pair<int, size_t>> dfsStack;
    dfsStack.emplace(nodeIndex, 0);
    visited.insert(nodeIndex);
    while(!dfsStack.empty()) {
        int currentNode = dfsStack.top().first;
        size_t& currentIndex = dfsStack.top().second;
        auto it = m_adjacencyList.find(currentNode);
        if(it == m_adjacencyList.end()) { Stack.push(currentNode); dfsStack.pop(); continue; }
        const auto& neighbors = it->second;
        if(currentIndex < neighbors.size()) {
            int neighbor = neighbors[currentIndex]; currentIndex++;
            if(visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                dfsStack.emplace(neighbor, 0);
            }
        } else {
            Stack.push(currentNode); dfsStack.pop();
        }
    }
}

void DirectedGraph::DFSUtil(int nodeIndex, std::unordered_set<int>& visited,
                            std::unordered_map<int, std::vector<int>>& transposeAdj, int component)
{
    std::stack<std::pair<int, size_t>> stack;
    stack.emplace(nodeIndex, 0);
    visited.insert(nodeIndex);
    int numComp = static_cast<int>(m_componentsColors.size());
    m_componentsColors[nodeIndex] = component;
    while(!stack.empty()) {
        int currentNode = stack.top().first;
        size_t& currentIndex = stack.top().second;
        auto it = transposeAdj.find(currentNode);
        if(it == transposeAdj.end()) { stack.pop(); continue; }
        const auto& neighbors = it->second;
        if(currentIndex < neighbors.size()) {
            int neighbor = neighbors[currentIndex]; currentIndex++;
            if(visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                m_componentsColors[neighbor] = component;
                stack.emplace(neighbor, 0);
            }
        } else { stack.pop(); }
    }
}

std::unordered_map<int, std::vector<int>> DirectedGraph::getTranspose()
{
    std::unordered_map<int, std::vector<int>> transposeAdj;
    for(const auto& node : m_nodes) transposeAdj[node.getIndex()] = {};
    for(const auto& edge : m_edges) {
        int from = edge.getFirst().getIndex();
        int to = edge.getSecond().getIndex();
        transposeAdj[to].push_back(from);
    }
    return transposeAdj;
}

void DirectedGraph::findConnectedComponents()
{
    if(m_nodes.empty()) { m_numComponents = 0; return; }
    buildAdjacencyList();
    m_componentsColors.clear();
    std::stack<int> finishTimeStack;
    std::unordered_set<int> visited;
    for(const auto& node : m_nodes) {
        int nodeIndex = node.getIndex();
        if(visited.find(nodeIndex) == visited.end())
            fillOrder(nodeIndex, visited, finishTimeStack);
    }
    auto transposeAdj = getTranspose();
    visited.clear();
    m_numComponents = 0;
    while(!finishTimeStack.empty()) {
        int nodeIndex = finishTimeStack.top(); finishTimeStack.pop();
        if(visited.find(nodeIndex) == visited.end()) {
            DFSUtil(nodeIndex, visited, transposeAdj, m_numComponents);
            m_numComponents++;
        }
    }
    buildCondensedGraph();
    m_showCondensedGraph = true;
}

void DirectedGraph::toggleCondensedGraph() { m_showCondensedGraph = !m_showCondensedGraph; }
bool DirectedGraph::isShowingCondensedGraph() const { return m_showCondensedGraph; }

void DirectedGraph::buildCondensedGraph()
{
    m_condensedNodes.clear(); m_condensedEdges.clear(); m_nodeToComponent.clear();
    if(m_numComponents == 0) return;
    for(const auto& node : m_nodes)
        m_nodeToComponent[node.getIndex()] = getComponentColor(node.getIndex());

    std::unordered_map<int, std::vector<int>> componentNodes;
    for(const auto& node : m_nodes)
        componentNodes[getComponentColor(node.getIndex())].push_back(node.getIndex());

    std::unordered_map<int, QPoint> componentPositions;
    for(const auto& pair : componentNodes) {
        int comp = pair.first; const auto& nodes = pair.second;
        int sumX = 0, sumY = 0;
        for(int nodeIdx : nodes)
            for(const auto& node : m_nodes)
                if(node.getIndex() == nodeIdx) { sumX += node.getX(); sumY += node.getY(); break; }
        componentPositions[comp] = QPoint(sumX / static_cast<int>(nodes.size()), sumY / static_cast<int>(nodes.size()));
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
        int fromComp = m_nodeToComponent[edge.getFirst().getIndex()];
        int toComp = m_nodeToComponent[edge.getSecond().getIndex()];
        if(fromComp != toComp) {
            std::string edgeKey = std::to_string(fromComp) + "," + std::to_string(toComp);
            if(addedEdges.find(edgeKey) == addedEdges.end()) {
                Node* fromCondensed = nullptr; Node* toCondensed = nullptr;
                for(auto& node : m_condensedNodes) {
                    if(node.getIndex() == fromComp + 1) fromCondensed = &node;
                    if(node.getIndex() == toComp + 1) toCondensed = &node;
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
	constexpr double arrowSize = 15.0;
	constexpr int nodeWidth = 80, nodeHeight = 40;
	constexpr double a = nodeWidth / 2.0, b = nodeHeight / 2.0;

    for(const auto& ed : m_condensedEdges) {
        QPointF startCenter(ed.getFirst().getX(), ed.getFirst().getY());
        QPointF endCenter(ed.getSecond().getX(), ed.getSecond().getY());
        QPointF direction = endCenter - startCenter;
        double length = std::sqrt(direction.x()*direction.x() + direction.y()*direction.y());
        if (length > 0) {
            QPointF unitDirection = direction / length;
            double t1 = 1.0 / std::sqrt((unitDirection.x()*unitDirection.x())/(a*a) +
                                         (unitDirection.y()*unitDirection.y())/(b*b));
            QPointF adjustedStart = startCenter + t1 * unitDirection;
            QPointF reverseDirection = startCenter - endCenter;
            QPointF unitReverseDirection = reverseDirection / length;
            double t2 = 1.0 / std::sqrt((unitReverseDirection.x()*unitReverseDirection.x())/(a*a) +
                                         (unitReverseDirection.y()*unitReverseDirection.y())/(b*b));
            QPointF adjustedEnd = endCenter + t2 * unitReverseDirection;
            QLineF line(adjustedStart, adjustedEnd);
            double angle = std::atan2(line.dy(), line.dx());
            int comp = ed.getFirst().getIndex() - 1;
            p.setPen(QPen(getColorForComponent(comp), 3));
            p.drawLine(adjustedStart, adjustedEnd);
            p.drawLine(adjustedEnd, adjustedEnd - QPointF(arrowSize*std::cos(angle-M_PI/6), arrowSize*std::sin(angle-M_PI/6)));
            p.drawLine(adjustedEnd, adjustedEnd - QPointF(arrowSize*std::cos(angle+M_PI/6), arrowSize*std::sin(angle+M_PI/6)));
        }
    }

    for(const auto& n : m_condensedNodes) {
        QRect r(n.getX() - nodeWidth/2, n.getY() - nodeHeight/2, nodeWidth, nodeHeight);
        int comp = n.getIndex() - 1;
        QColor nodeColor = getColorForComponent(comp);
        p.setPen(QPen(nodeColor, 3)); p.setBrush(QBrush(nodeColor.lighter(180)));
        p.drawEllipse(r);
        p.setPen(Qt::black);
        QString label; bool first = true;
        for(const auto& origNode : m_nodes) {
            if(getComponentColor(origNode.getIndex()) == comp) {
                if(!first) label += ",";
                label += QString::number(origNode.getIndex());
                first = false;
            }
        }
        QFont font = p.font(); font.setPointSize(10); font.setBold(true); p.setFont(font);
        p.drawText(r, Qt::AlignCenter, label);
    }
}

// ─── Shortest Paths ───

static std::vector<int> reconstruct(int end, const std::unordered_map<int,int>& parent)
{
    std::vector<int> path;
    for(int cur = end; cur != -1; ) {
        path.push_back(cur);
        auto it = parent.find(cur);
        cur = (it != parent.end()) ? it->second : -1;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

std::vector<PathStep> DirectedGraph::dijkstra(int startIdx, int endIdx, std::vector<int>& outPath) const
{
    auto adj = buildWeightedAdjList();
    std::unordered_map<int,int> dist, parent;
    std::unordered_set<int> settled;
    std::vector<PathStep> steps;
    for(const auto& n : m_nodes) dist[n.getIndex()] = INF_COST;
    dist[startIdx] = 0; parent[startIdx] = -1;
    using T = std::pair<int,int>;
    std::priority_queue<T, std::vector<T>, std::greater<>> pq;
    pq.emplace(0, startIdx);
    while(!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if(settled.count(u)) continue;
        settled.insert(u);
        steps.push_back({u, parent.count(u) ? parent[u] : -1, d, false});
        if(u == endIdx) break;
        for(auto& [v, w] : adj[u]) {
            if(dist[u] + w < dist[v]) { dist[v] = dist[u] + w; parent[v] = u; pq.emplace(dist[v], v); }
        }
    }
    outPath = reconstruct(endIdx, parent);
    for(auto& s : steps) for(int idx : outPath) if(s.nodeIndex == idx) { s.isFinal = true; break; }
    return steps;
}

std::vector<PathStep> DirectedGraph::aStar(int startIdx, int endIdx, std::vector<int>& outPath) const
{
    auto adj = buildWeightedAdjList();
    std::vector<PathStep> steps;
    std::unordered_map<int, QPoint> pos;
    for(const auto& n : m_nodes) pos[n.getIndex()] = n.getCoord();
    double minRatio = std::numeric_limits<double>::infinity();
    for(const auto& ed : m_edges) {
        QPointF A(ed.getFirst().getX(), ed.getFirst().getY());
        QPointF B(ed.getSecond().getX(), ed.getSecond().getY());
        double L = QLineF(A, B).length();
        if(L > 1e-6) {
            double r = static_cast<double>(ed.getCost()) / L;
            if(r < minRatio) minRatio = r;
        }
    }
    if(!std::isfinite(minRatio) || minRatio <= 0.0) minRatio = 0.0;
    auto heuristic = [&](int a, int b) -> int {
        if(minRatio == 0.0) return 0;
        double dx = static_cast<double>(pos[a].x() - pos[b].x());
        double dy = static_cast<double>(pos[a].y() - pos[b].y());
        double eu = std::sqrt(dx*dx + dy*dy);
        double h = eu * minRatio;
        if(h < 0) h = 0;
        return static_cast<int>(h);
    };
    std::unordered_map<int,int> gCost, parent;
    std::unordered_set<int> closed;
    for(const auto& n : m_nodes) gCost[n.getIndex()] = INF_COST;
    gCost[startIdx] = 0; parent[startIdx] = -1;
    using T = std::pair<int,int>;
    std::priority_queue<T, std::vector<T>, std::greater<>> open;
    open.emplace(heuristic(startIdx, endIdx), startIdx);
    while(!open.empty()) {
        auto [f, u] = open.top(); open.pop();
        if(closed.count(u)) continue;
        closed.insert(u);
        steps.push_back({u, parent.count(u) ? parent[u] : -1, gCost[u], false});
        if(u == endIdx) break;
        for(auto& [v, w] : adj[u]) {
	        if(int ng = gCost[u] + w; ng < gCost[v])
	        {
		        gCost[v] = ng; 
	        	parent[v] = u; 
	        	open.emplace(ng + heuristic(v, endIdx), v);
	        }
        }
    }
    outPath = reconstruct(endIdx, parent);
    for(auto& s : steps) for(int idx : outPath) if(s.nodeIndex == idx) { s.isFinal = true; break; }
    return steps;
}

std::vector<PathStep> DirectedGraph::bellmanFord(int startIdx, int endIdx, std::vector<int>& outPath) const
{
    std::unordered_map<int,int> dist, parent;
    std::vector<PathStep> steps;
    for(const auto& n : m_nodes) dist[n.getIndex()] = INF_COST;
    dist[startIdx] = 0; parent[startIdx] = -1;
    int N = static_cast<int>(m_nodes.size());
    for(int round = 0; round < N - 1; ++round) {
        bool updated = false;
        for(const auto& ed : m_edges) {
            int u = ed.getFirst().getIndex(), v = ed.getSecond().getIndex(), w = ed.getCost();
            if(dist[u] != INF_COST && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w; parent[v] = u;
                steps.push_back({v, u, dist[v], false}); updated = true;
            }
        }
        if(!updated) break;
    }
    outPath = reconstruct(endIdx, parent);
    for(auto& s : steps) for(int idx : outPath) if(s.nodeIndex == idx) { s.isFinal = true; break; }
    return steps;
}

std::vector<PathStep> DirectedGraph::floydWarshall(int startIdx, int endIdx, std::vector<int>& outPath) const
{
    std::vector<PathStep> steps;
    std::vector<int> ids;
    for(const auto& n : m_nodes) ids.push_back(n.getIndex());
    int N = static_cast<int>(ids.size());
    std::unordered_map<int,int> pos;
    for(int i = 0; i < N; i++) pos[ids[i]] = i;
    std::vector<std::vector<int>> dist(N, std::vector<int>(N, INF_COST));
    std::vector<std::vector<int>> next(N, std::vector<int>(N, -1));
    for(int i = 0; i < N; i++) dist[i][i] = 0;
    for(const auto& ed : m_edges) {
        int u = pos[ed.getFirst().getIndex()], v = pos[ed.getSecond().getIndex()], w = ed.getCost();
        if(w < dist[u][v]) { dist[u][v] = w; next[u][v] = v; }
    }
    int si = pos[startIdx];
    for(int k = 0; k < N; k++)
        for(int i = 0; i < N; i++)
            for(int j = 0; j < N; j++)
                if(dist[i][k] != INF_COST && dist[k][j] != INF_COST && dist[i][k]+dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k]+dist[k][j]; next[i][j] = next[i][k];
                    if(i == si) steps.push_back({ids[j], ids[k], dist[i][j], false});
                }
    outPath.clear();
    int u = pos[startIdx], v = pos[endIdx];
    if(dist[u][v] == INF_COST) return steps;
    outPath.push_back(startIdx);
    while(ids[u] != endIdx) { u = next[u][v]; if(u == -1) break; outPath.push_back(ids[u]); }
    for(auto& s : steps) for(int idx : outPath) if(s.nodeIndex == idx) { s.isFinal = true; break; }
    return steps;
}
