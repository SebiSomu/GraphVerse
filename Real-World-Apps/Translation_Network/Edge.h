#pragma once
#include "Node.h"

class Edge
{
    Node* m_first;
    Node* m_second;
    int m_cost;
public:
    Edge(Node* f, Node* s, int cost);
    Node& getFirst() const;
    Node& getSecond() const;
    int getCost() const;
    void setCost(int cost);
};