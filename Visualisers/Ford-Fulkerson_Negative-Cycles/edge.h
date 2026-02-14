#ifndef EDGE_H
#define EDGE_H
#include "node.h"

class Edge
{
    Node *m_first, *m_second;
    int m_capacity;
    int m_flow;
    bool m_isMinCut;
public:
    Edge(Node* f, Node* s, int capacity = 1);
    Node& getFirst() const;
    Node& getSecond() const;
    int getCapacity() const;
    void setCapacity(int capacity);
    int getFlow() const;
    void setFlow(int flow);
    void addFlow(int delta);
    int getResidualCapacity() const;
    bool isMinCut() const;
    void setMinCut(bool minCut);
};

#endif // EDGE_H
