#ifndef DIRECTEDGRAPH_H
#define DIRECTEDGRAPH_H
#include "graph.h"
#include <QLineF>
#include <QPolygonF>
#include <QtMath>
#include <vector>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <limits>

struct PathStep {
    int  nodeIndex;
    int  fromIndex;
    int  cost;
    bool isFinal;
};

static const int INF_COST = std::numeric_limits<int>::max() / 2;

class DirectedGraph : public Graph
{
private:
    // SCC support (Kosaraju)
    void fillOrder(int nodeIndex, std::unordered_set<int>& visited, std::stack<int>& Stack);
    void DFSUtil(int nodeIndex, std::unordered_set<int>& visited,
                 std::unordered_map<int, std::vector<int>>& transposeAdj,
                 int component);
    std::unordered_map<int, std::vector<int>> getTranspose();
    bool m_showCondensedGraph;
    std::vector<Node> m_condensedNodes;
    std::vector<Edge> m_condensedEdges;
    std::unordered_map<int, int> m_nodeToComponent;
    void buildCondensedGraph();
    void drawCondensedGraph(QPainter& p) const;

public:
    DirectedGraph();
    void addEdge(Node &f, Node &s, int cost = 1) override;
    void drawEdge(QPainter& p) const override;
    std::string getGraphType() const override;
    void buildAdjacencyList() override;
    void findConnectedComponents() override;
    void toggleCondensedGraph();
    bool isShowingCondensedGraph() const;

    // BFS/DFS
    std::unordered_map<int, std::vector<int>> buildSimpleAdjList() const;
    std::vector<int> bfs(int startIndex, int stopAt = -1,
                         std::function<void(int)> onVisit = nullptr,
                         std::function<void(int,int)> onEdge = nullptr) const;
    std::vector<int> dfs(int startIndex, int stopAt = -1,
                         std::function<void(int)> onVisit = nullptr,
                         std::function<void(int,int)> onEdge = nullptr) const;

    // Shortest paths
    std::unordered_map<int, std::vector<std::pair<int,int>>> buildWeightedAdjList() const;
    std::vector<PathStep> dijkstra    (int startIdx, int endIdx, std::vector<int>& outPath) const;
    std::vector<PathStep> aStar       (int startIdx, int endIdx, std::vector<int>& outPath) const;
    std::vector<PathStep> bellmanFord (int startIdx, int endIdx, std::vector<int>& outPath) const;
    std::vector<PathStep> floydWarshall(int startIdx, int endIdx, std::vector<int>& outPath) const;
};

#endif // DIRECTEDGRAPH_H
