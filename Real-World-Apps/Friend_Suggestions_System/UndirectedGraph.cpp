#include "undirectedgraph.h"
#include <queue>

void UndirectedGraph::addEdge(Node& f, Node& s) {
    m_edges.push_back(Edge(&f, &s));
    m_edges.push_back(Edge(&s, &f));
}

std::unordered_map<int, std::vector<int>> UndirectedGraph::buildAdjList() const {
    std::unordered_map<int, std::vector<int>> adj;
    for (const auto& n : m_nodes)
        adj[n.getIndex()] = {};
    for (const auto& e : m_edges)
        adj[e.getFirst().getIndex()].push_back(e.getSecond().getIndex());
    return adj;
}

BFSResult UndirectedGraph::bfs(const std::string& startName,
    int maxDepth,
    std::function<void(const Node&, int)> onVisit) const
{
    BFSResult result;
    const Node* startNode = findByName(startName);
    if (!startNode) return result;

    auto adj = buildAdjList();
    std::queue<int> q;

    int startIdx = startNode->getIndex();
    result.dist[startIdx] = 0;
    result.parent[startIdx] = -1;
    q.push(startIdx);

    while (!q.empty()) {
        int curIdx = q.front(); q.pop();
        int curDist = result.dist[curIdx];

        if (onVisit) {
            const Node* curNode = findByIndex(curIdx);
            if (curNode) onVisit(*curNode, curDist);
        }

        if (maxDepth >= 0 && curDist >= maxDepth) continue;

        for (int neighborIdx : adj[curIdx]) {
            if (result.dist.find(neighborIdx) == result.dist.end()) {
                result.dist[neighborIdx] = curDist + 1;
                result.parent[neighborIdx] = curIdx;
                q.push(neighborIdx);
            }
        }
    }
    return result;
}

std::string UndirectedGraph::getGraphType() const { return "Undirected"; }