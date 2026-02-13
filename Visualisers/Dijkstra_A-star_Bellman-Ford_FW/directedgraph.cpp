#include "directedgraph.h"
#include <queue>
#include <algorithm>
#include <cmath>
#include <unordered_set>

DirectedGraph::DirectedGraph() {}

void DirectedGraph::addEdge(Node &f, Node &s, int cost)
{
    Edge newEdge(&f, &s, cost);
    m_edges.push_back(newEdge);
}

void DirectedGraph::drawEdge(QPainter &p) const
{
    const double ar = 12.0, nr = 10.0;
    for(const auto& ed : m_edges) {
        QPointF a(ed.getFirst().getX(),  ed.getFirst().getY());
        QPointF b(ed.getSecond().getX(), ed.getSecond().getY());
        QLineF  ln(a, b);
        double  angle = std::atan2(ln.dy(), ln.dx());
        QPointF as = a + QPointF(nr*cos(angle), nr*sin(angle));
        QPointF ae = b - QPointF(nr*cos(angle), nr*sin(angle));
        p.drawLine(as, ae);
        p.drawLine(ae, ae - QPointF(ar*cos(angle-M_PI/6), ar*sin(angle-M_PI/6)));
        p.drawLine(ae, ae - QPointF(ar*cos(angle+M_PI/6), ar*sin(angle+M_PI/6)));
    }
    for(const auto& n : m_nodes) {
        QRect r(n.getX()-10, n.getY()-10, 20, 20);
        p.drawEllipse(r);
        p.drawText(r, Qt::AlignCenter, QString::number(n.getIndex()));
    }
}

std::string DirectedGraph::getGraphType() const { return "Directed"; }

std::unordered_map<int, std::vector<std::pair<int,int>>> DirectedGraph::buildAdjList() const
{
    std::unordered_map<int, std::vector<std::pair<int,int>>> adj;
    for(const auto& n  : m_nodes) adj[n.getIndex()] = {};
    for(const auto& ed : m_edges)
        adj[ed.getFirst().getIndex()].push_back({ed.getSecond().getIndex(), ed.getCost()});
    return adj;
}

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

std::vector<PathStep> DirectedGraph::dijkstra(int startIdx, int endIdx,
                                              std::vector<int>& outPath) const
{
    auto adj = buildAdjList();
    std::unordered_map<int,int> dist, parent;
    std::unordered_set<int> settled;
    std::vector<PathStep> steps;

    for(const auto& n : m_nodes) dist[n.getIndex()] = INF;
    dist[startIdx] = 0;
    parent[startIdx] = -1;

    using T = std::pair<int,int>;
    std::priority_queue<T, std::vector<T>, std::greater<T>> pq;
    pq.push({0, startIdx});

    while(!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if(settled.count(u)) continue;
        settled.insert(u);

        steps.push_back({u, parent.count(u) ? parent[u] : -1, d, false});
        if(u == endIdx) break;

        for(auto& [v, w] : adj[u]) {
            if(dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                parent[v] = u;
                pq.push({dist[v], v});
            }
        }
    }

    outPath = reconstruct(endIdx, parent);
    for(auto& s : steps)
        for(int idx : outPath)
            if(s.nodeIndex == idx) {
                s.isFinal = true;
                break;
            }
    return steps;
}

std::vector<PathStep> DirectedGraph::aStar(int startIdx, int endIdx,
                                           std::vector<int>& outPath) const
{
    auto adj = buildAdjList();
    std::vector<PathStep> steps;

    std::unordered_map<int, QPoint> pos;
    for(const auto& n : m_nodes) pos[n.getIndex()] = n.getCoord();

    auto heuristic = [&](int a, int b) -> int {
        int dx = pos[a].x() - pos[b].x();
        int dy = pos[a].y() - pos[b].y();
        return static_cast<int>(std::sqrt(dx*dx + dy*dy) * 0.5);
    };

    std::unordered_map<int,int> gCost, parent;
    std::unordered_set<int> closed;

    for(const auto& n : m_nodes) gCost[n.getIndex()] = INF;
    gCost[startIdx] = 0;
    parent[startIdx] = -1;

    using T = std::pair<int,int>;
    std::priority_queue<T, std::vector<T>, std::greater<T>> open;
    open.push({heuristic(startIdx, endIdx), startIdx});

    while(!open.empty()) {
        auto [f, u] = open.top(); open.pop();
        if(closed.count(u)) continue;
        closed.insert(u);

        steps.push_back({u, parent.count(u) ? parent[u] : -1, gCost[u], false});
        if(u == endIdx) break;

        for(auto& [v, w] : adj[u]) {
            int ng = gCost[u] + w;
            if(ng < gCost[v]) {
                gCost[v] = ng;
                parent[v] = u;
                open.push({ng + heuristic(v, endIdx), v});
            }
        }
    }

    outPath = reconstruct(endIdx, parent);
    for(auto& s : steps)
        for(int idx : outPath)
            if(s.nodeIndex == idx) {
                s.isFinal = true;
                break;
            }
    return steps;
}

std::vector<PathStep> DirectedGraph::bellmanFord(int startIdx, int endIdx,
                                                 std::vector<int>& outPath) const
{
    std::unordered_map<int,int> dist, parent;
    std::vector<PathStep> steps;

    for(const auto& n : m_nodes) dist[n.getIndex()] = INF;
    dist[startIdx] = 0;
    parent[startIdx] = -1;

    int N = (int)m_nodes.size();

    for(int round = 0; round < N - 1; ++round) {
        bool updated = false;
        for(const auto& ed : m_edges) {
            int u = ed.getFirst().getIndex();
            int v = ed.getSecond().getIndex();
            int w = ed.getCost();
            if(dist[u] != INF && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                parent[v] = u;
                steps.push_back({v, u, dist[v], false});
                updated = true;
            }
        }
        if(!updated) break;
    }

    outPath = reconstruct(endIdx, parent);
    for(auto& s : steps)
        for(int idx : outPath)
            if(s.nodeIndex == idx) { s.isFinal = true; break; }
    return steps;
}

std::vector<PathStep> DirectedGraph::floydWarshall(int startIdx, int endIdx,
                                                   std::vector<int>& outPath) const
{
    std::vector<PathStep> steps;
    std::vector<int> ids;
    for(const auto& n : m_nodes) ids.push_back(n.getIndex());
    int N = (int)ids.size();

    std::unordered_map<int,int> pos;
    for(int i = 0; i < N; i++) pos[ids[i]] = i;

    std::vector<std::vector<int>> dist(N, std::vector<int>(N, INF));
    std::vector<std::vector<int>> next(N, std::vector<int>(N, -1));

    for(int i = 0; i < N; i++) dist[i][i] = 0;
    for(const auto& ed : m_edges) {
        int u = pos[ed.getFirst().getIndex()];
        int v = pos[ed.getSecond().getIndex()];
        int w = ed.getCost();
        if(w < dist[u][v]) {
            dist[u][v] = w;
            next[u][v] = pos[ed.getSecond().getIndex()];
        }
    }

    int si = pos[startIdx];
    for(int k = 0; k < N; k++) {
        for(int i = 0; i < N; i++) {
            for(int j = 0; j < N; j++) {
                if(dist[i][k] != INF && dist[k][j] != INF &&
                    dist[i][k] + dist[k][j] < dist[i][j])
                {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    next[i][j] = next[i][k];
                    if(i == si)
                        steps.push_back({ids[j], ids[k], dist[i][j], false});
                }
            }
        }
    }

    outPath.clear();
    int u = pos[startIdx], v = pos[endIdx];
    if(dist[u][v] == INF) { return steps; }
    outPath.push_back(startIdx);
    while(ids[u] != endIdx) {
        u = next[u][v];
        if(u == -1) break;
        outPath.push_back(ids[u]);
    }

    for(auto& s : steps)
        for(int idx : outPath)
            if(s.nodeIndex == idx) { s.isFinal = true; break; }
    return steps;
}
