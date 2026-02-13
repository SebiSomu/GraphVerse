#ifndef DIRECTEDGRAPH_H
#define DIRECTEDGRAPH_H
#include "graph.h"
#include <unordered_map>
#include <vector>
#include <limits>
#include <QLineF>
#include <QtMath>

struct PathStep {
    int  nodeIndex;
    int  fromIndex;  // where we came from (-1 = start)
    int  cost;       // best known cost so far
    bool isFinal;    // true = belongs to final shortest path
};

static const int INF = std::numeric_limits<int>::max() / 2;

class DirectedGraph : public Graph
{
public:
    DirectedGraph();
    void addEdge(Node &f, Node &s, int cost = 1) override;
    void drawEdge(QPainter& p) const override;
    std::string getGraphType() const override;

    // Pathfinding — each returns animation steps, outPath = final path node indices
    std::vector<PathStep> dijkstra    (int startIdx, int endIdx, std::vector<int>& outPath) const;
    std::vector<PathStep> aStar       (int startIdx, int endIdx, std::vector<int>& outPath) const;
    std::vector<PathStep> bellmanFord (int startIdx, int endIdx, std::vector<int>& outPath) const;
    std::vector<PathStep> floydWarshall(int startIdx, int endIdx, std::vector<int>& outPath) const;

    // Adjacency list: index -> list of (neighbour, cost)
    std::unordered_map<int, std::vector<std::pair<int,int>>> buildAdjList() const;
};

#endif // DIRECTEDGRAPH_H
