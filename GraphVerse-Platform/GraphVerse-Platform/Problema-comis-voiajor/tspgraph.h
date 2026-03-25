#ifndef TSPGRAPH_H
#define TSPGRAPH_H

#include "undirectedgraph.h"
#include <vector>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <algorithm>
#include <limits>

class TSPGraph : public UndirectedGraph
{
private:
    std::vector<std::vector<int>> m_distMatrix;
    std::vector<Edge> m_mstEdges;
    std::vector<int> m_tspCircuit;
    std::vector<std::vector<int>> m_adjacencyList;
    
    std::vector<int> m_parent;
    std::vector<int> m_rank;
    
    int find(int x);
    void unite(int x, int y);
    
    void dfsPreorder(int node, std::vector<bool>& visited, 
                     const std::vector<std::vector<int>>& mstAdj);

public:
    TSPGraph();
    
    bool loadFromFile(const QString& filename);
    void floydWarshall();
    void generateCompleteGraph();
    void kruskalMST();
    void generateTSPCircuit();
    
    const std::vector<std::vector<int>>& getDistMatrix() const;
    const std::vector<Edge>& getMSTEdges() const;
    const std::vector<int>& getTSPCircuit() const;
    
    void drawGraph(QPainter& p, bool showMST = false, bool showTSP = false) const;
};

#endif // TSPGRAPH_H
