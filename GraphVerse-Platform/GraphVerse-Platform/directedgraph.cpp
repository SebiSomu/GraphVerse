#include "directedgraph.h"
#include "algorithms/kosaraju_solver.h"
#include <algorithm>
#include <cmath>
#include <string>
#include "directedgraph.h"
#include <algorithm>

DirectedGraph::DirectedGraph() : Graph() {}

void DirectedGraph::addEdge(Node &f, Node &s, int cost) {
    m_edges.emplace_back(&f, &s, cost);
}

std::string_view DirectedGraph::getGraphType() const { return "Directed"; }
