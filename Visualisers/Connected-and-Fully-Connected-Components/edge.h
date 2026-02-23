#ifndef EDGE_H
#define EDGE_H
#include "node.h"

class Edge
{
    Node *m_first, *m_second;
public:
    Edge(Node* f, Node* s);
    Node& getFirst() const;
    Node& getSecond() const;
};

#endif // EDGE_H
