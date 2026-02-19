#include "weightedgraph.h"
#include <algorithm>
#include <unordered_map>

UnionFind::UnionFind(int n) : parent(n), rank(n, 0) {
    for (int i = 0; i < n; i++) parent[i] = i;
}

int UnionFind::find(int x) {
    if (parent[x] != x) parent[x] = find(parent[x]);
    return parent[x];
}

bool UnionFind::unite(int x, int y) {
    int px = find(x), py = find(y);
    if (px == py) return false;
    if (rank[px] < rank[py]) parent[px] = py;
    else if (rank[px] > rank[py]) parent[py] = px;
    else { parent[py] = px; rank[px]++; }
    return true;
}

WeightedGraph::WeightedGraph() {}

void WeightedGraph::addEdge(Node& f, Node& s, int cost) {
    m_edges.emplace_back(&f, &s, cost);
}

std::vector<Edge> WeightedGraph::kruskalMST() const {
    std::vector<Edge> mst;
    if (m_nodes.empty()) return mst;

    std::vector<Edge> sortedEdges = m_edges;
    std::sort(sortedEdges.begin(), sortedEdges.end(),
        [](const Edge& a, const Edge& b) { return a.getCost() < b.getCost(); });

    std::unordered_map<int, int> indexToUF;
    int ufIdx = 0;
    for (const auto& node : m_nodes)
        indexToUF[node.getIndex()] = ufIdx++;

    UnionFind uf(m_nodes.size());

    for (const auto& edge : sortedEdges) {
        int u = indexToUF[edge.getFirst().getIndex()];
        int v = indexToUF[edge.getSecond().getIndex()];
        if (uf.unite(u, v))
            mst.push_back(edge);
        if (mst.size() == m_nodes.size() - 1)
            break;
    }

    return mst;
}

std::string WeightedGraph::getGraphType() const { return "Weighted"; }