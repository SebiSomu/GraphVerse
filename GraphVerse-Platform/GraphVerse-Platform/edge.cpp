#include "edge.h"

Edge::Edge(Node* f, Node* s, int cost, int capacity)
    : m_first(f), m_second(s), m_cost(cost), m_capacity(capacity), m_flow(0), m_isMinCut(false) {}

Node& Edge::getFirst() const { return *m_first; }
Node& Edge::getSecond() const { return *m_second; }

int Edge::getCost() const { return m_cost; }
void Edge::setCost(int cost) { m_cost = cost; }

int Edge::getCapacity() const { return m_capacity; }
void Edge::setCapacity(int capacity) { m_capacity = capacity; }

int Edge::getFlow() const { return m_flow; }
void Edge::setFlow(int flow) { m_flow = flow; }
void Edge::addFlow(int delta) { m_flow += delta; }
int Edge::getResidualCapacity() const { return m_capacity - m_flow; }

bool Edge::isMinCut() const { return m_isMinCut; }
void Edge::setMinCut(bool minCut) { m_isMinCut = minCut; }
