#ifndef FLOWNETWORK_H
#define FLOWNETWORK_H

#include <vector>
#include <queue>
#include <map>
#include <set>
#include <climits>
#include <QPointF>
#include <QString>

struct IterationState {
    int iterationNumber;
    std::vector<std::vector<int>> residualCapacity;
    std::vector<int> augmentingPath;
    int pathFlow;
    int totalFlow;
    QString description;
};

class FlowNetwork
{
public:
    FlowNetwork();
    void addNode(const QPointF& pos);
    void addEdge(int from, int to, int capacity);
    void setCapacity(int from, int to, int capacity);
    void setEdgeCost(int from, int to, int cost);
    int getEdgeCost(int from, int to) const;
    void clear();
    void resetFlow();
    int getNumNodes() const;
    const std::vector<QPointF>& getNodePositions() const;
    int getCapacity(int from, int to) const;
    int getFlow(int from, int to) const;
    int getResidual(int from, int to) const;
    void setSource(int s);
    void setSink(int t);
    int getSource() const;
    int getSink() const;
    int runFullAlgorithm();
    void initializeAlgorithm();
    bool performStep();
    void finalizeMinCut();
    void addFinalState();
    int eliminateNegativeCycles();
    const std::vector<IterationState>& getIterations() const;
    const std::set<std::pair<int,int>>& getMinCutEdges() const;
    bool isMinCutEdge(int from, int to) const;

private:
    int m_source;
    int m_sink;
    std::vector<std::vector<int>> m_capacity;
    std::vector<std::vector<int>> m_flow;
    std::vector<std::vector<int>> m_cost;
    std::vector<QPointF> m_nodePositions;
    std::vector<IterationState> m_iterations;
    std::vector<std::vector<int>> m_currentResidual;
    int m_currentTotalFlow;
    std::set<std::pair<int,int>> m_minCutEdges;
    bool bfs(const std::vector<std::vector<int>>& rGraph, int s, int t, std::vector<int>& parent);
    std::vector<int> findNegativeCycle();
};

#endif // FLOWNETWORK_H
