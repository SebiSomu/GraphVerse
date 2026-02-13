#ifndef DIRECTEDGRAPH_H
#define DIRECTEDGRAPH_H
#include "graph.h"
#include <QLineF>
#include <QtMath>
#include <vector>
#include <functional>
#include <unordered_map>

class DirectedGraph : public Graph
{
public:
    DirectedGraph();
    void addEdge(Node &f, Node &s) override;
    void drawEdge(QPainter& p) const override;
    std::string getGraphType() const override;
    std::unordered_map<int, std::vector<int>> buildAdjacencyList() const;

    std::vector<int> bfs(int startIndex,
                         int stopAt = -1,
                         std::function<void(int)>      onVisit = nullptr,
                         std::function<void(int,int)>  onEdge  = nullptr) const;

    std::vector<int> dfs(int startIndex,
                         int stopAt = -1,
                         std::function<void(int)>      onVisit = nullptr,
                         std::function<void(int,int)>  onEdge  = nullptr) const;

};

#endif // DIRECTEDGRAPH_H
