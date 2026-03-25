#ifndef EDGE_H
#define EDGE_H
#include "node.h"

class Edge
{
    Node *m_first, *m_second;
    int m_cost;
public:
    Edge(Node* f, Node* s, int cost = 1);
    Node& getFirst() const;
    Node& getSecond() const;
    int getCost() const;
    void setCost(int cost);
};

#endif // EDGE_H
