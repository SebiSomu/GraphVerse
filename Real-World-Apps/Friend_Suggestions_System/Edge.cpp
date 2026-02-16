#include "Edge.h"

Edge::Edge(Node* f, Node* s)
    : m_first(f), m_second(s) {
}

Node& Edge::getFirst() const { return *m_first; }
Node& Edge::getSecond() const { return *m_second; }