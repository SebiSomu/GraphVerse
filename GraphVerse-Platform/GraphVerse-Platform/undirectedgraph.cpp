#include "undirectedgraph.h"
#include <cmath>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <limits>

UndirectedGraph::UndirectedGraph() {}

void UndirectedGraph::addEdge(Node &f, Node &s, int cost)
{
    m_edges.emplace_back(&f, &s, cost);
    m_componentsColors.clear();
    m_numComponents = 0;
}

void UndirectedGraph::drawEdge(QPainter &p) const
{
    const double nodeRadius = 10.0;
    bool componentsFound = (m_numComponents > 0);

    for(const auto& ed : m_edges) {
        QPointF start(ed.getFirst().getX(), ed.getFirst().getY());
        QPointF end(ed.getSecond().getX(), ed.getSecond().getY());
        QLineF line(start, end);
        double angle = std::atan2(line.dy(), line.dx());
        QPointF adjStart = start + QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));
        QPointF adjEnd = end - QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));

        if(componentsFound) {
            int comp = getComponentColor(ed.getFirst().getIndex());
            p.setPen(QPen(getColorForComponent(comp), 2));
        } else {
            p.setPen(QPen(Qt::white, 1));
        }
        p.drawLine(adjStart, adjEnd);

        /* Cost label - hidden for CC view
        QPointF mid = (adjStart + adjEnd) / 2.0;
        QPointF offset(-12 * sin(angle), 12 * cos(angle));
        p.save();
        p.setPen(Qt::green);
        QFont f = p.font(); f.setBold(true); f.setPointSize(10);
        p.setFont(f);
        p.drawText(mid + offset, QString::number(ed.getCost()));
        p.restore();
        */
    }

    for(const auto& n : m_nodes) {
        QRect r(n.getX() - (int)nodeRadius, n.getY() - (int)nodeRadius,
                (int)nodeRadius*2, (int)nodeRadius*2);
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

std::string UndirectedGraph::getGraphType() const { return "Undirected"; }

// ─── Union-Find ───

struct UnionFind {
    std::unordered_map<int,int> parent, rank_;
    void init(const std::vector<int>& ids) {
        for(int id : ids) { parent[id] = id; rank_[id] = 0; }
    }
    int find(int x) {
        if(parent[x] != x) parent[x] = find(parent[x]);
        return parent[x];
    }
    bool unite(int a, int b) {
        int ra = find(a), rb = find(b);
        if(ra == rb) return false;
        if(rank_[ra] < rank_[rb]) std::swap(ra, rb);
        parent[rb] = ra;
        if(rank_[ra] == rank_[rb]) rank_[ra]++;
        return true;
    }
    bool connected(int a, int b) { return find(a) == find(b); }
};

// ─── MST Algorithms ───

std::vector<MSTStep> UndirectedGraph::kruskal() const
{
    std::vector<MSTStep> steps;
    if(m_nodes.empty()) return steps;
    std::vector<const Edge*> sorted;
    for(const auto& ed : m_edges) sorted.push_back(&ed);
    std::sort(sorted.begin(), sorted.end(),
              [](const Edge* a, const Edge* b){ return a->getCost() < b->getCost(); });
    UnionFind uf;
    std::vector<int> ids;
    for(const auto& n : m_nodes) ids.push_back(n.getIndex());
    uf.init(ids);
    for(const Edge* ed : sorted) {
        int u = ed->getFirst().getIndex(), v = ed->getSecond().getIndex();
        bool ok = uf.unite(u, v);
        steps.push_back({u, v, ed->getCost(), ok});
    }
    return steps;
}

std::vector<MSTStep> UndirectedGraph::prim() const
{
    std::vector<MSTStep> steps;
    if(m_nodes.empty()) return steps;
    std::unordered_map<int, std::vector<std::pair<int,int>>> adj;
    for(const auto& n : m_nodes) adj[n.getIndex()] = {};
    for(const auto& ed : m_edges) {
        int u = ed.getFirst().getIndex(), v = ed.getSecond().getIndex(), c = ed.getCost();
        adj[u].push_back({v, c}); adj[v].push_back({u, c});
    }
    std::unordered_set<int> inMST;
    using T = std::tuple<int,int,int>;
    std::priority_queue<T, std::vector<T>, std::greater<T>> pq;
    int start = m_nodes.front().getIndex();
    inMST.insert(start);
    for(auto& [nb, c] : adj[start]) pq.push({c, start, nb});
    while(!pq.empty()) {
        auto [cost, u, v] = pq.top(); pq.pop();
        if(inMST.count(v)) { steps.push_back({u, v, cost, false}); continue; }
        inMST.insert(v);
        steps.push_back({u, v, cost, true});
        for(auto& [nb, c] : adj[v]) if(!inMST.count(nb)) pq.push({c, v, nb});
    }
    return steps;
}

std::vector<MSTStep> UndirectedGraph::boruvka() const
{
    std::vector<MSTStep> steps;
    if(m_nodes.empty()) return steps;
    std::vector<int> ids;
    for(const auto& n : m_nodes) ids.push_back(n.getIndex());
    UnionFind uf; uf.init(ids);
    int n = (int)m_nodes.size(), mstEdges = 0;
    while(mstEdges < n - 1) {
        std::unordered_map<int, const Edge*> cheapest;
        for(const auto& ed : m_edges) {
            int u = ed.getFirst().getIndex(), v = ed.getSecond().getIndex();
            int ru = uf.find(u), rv = uf.find(v);
            if(ru == rv) continue;
            if(!cheapest.count(ru) || ed.getCost() < cheapest[ru]->getCost()) cheapest[ru] = &ed;
            if(!cheapest.count(rv) || ed.getCost() < cheapest[rv]->getCost()) cheapest[rv] = &ed;
        }
        if(cheapest.empty()) break;
        std::unordered_set<const Edge*> added;
        for(auto& [comp, ed] : cheapest) {
            if(added.count(ed)) continue;
            int u = ed->getFirst().getIndex(), v = ed->getSecond().getIndex();
            if(uf.unite(u, v)) { steps.push_back({u, v, ed->getCost(), true}); added.insert(ed); mstEdges++; }
        }
        for(const auto& ed : m_edges) {
            int u = ed.getFirst().getIndex(), v = ed.getSecond().getIndex();
            if(uf.connected(u, v) && !added.count(&ed)) {
                bool inMST = false;
                for(auto& s : steps)
                    if(s.accepted && ((s.fromIndex==u && s.toIndex==v) || (s.fromIndex==v && s.toIndex==u)))
                    { inMST = true; break; }
                if(!inMST) steps.push_back({u, v, ed.getCost(), false});
            }
        }
    }
    return steps;
}
