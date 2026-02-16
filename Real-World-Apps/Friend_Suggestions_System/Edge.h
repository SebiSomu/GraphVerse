#pragma once
#include "Node.h"

class Edge
{
    Node* m_first;
    Node* m_second;
public:
    Edge(Node* f, Node* s);
    Node& getFirst() const;
    Node& getSecond() const;
};