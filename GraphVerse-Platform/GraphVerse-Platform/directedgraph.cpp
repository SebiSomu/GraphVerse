#include "directedgraph.h"
#include "algorithms/kosaraju_solver.h"
#include <algorithm>
#include <cmath>
#include <string>
#include "directedgraph.h"
#include <algorithm>

DirectedGraph::DirectedGraph() : Graph(), m_showCondensedGraph(false) {}

void DirectedGraph::addEdge(Node &f, Node &s, int cost) {
    m_edges.emplace_back(&f, &s, cost);
    m_numComponents = 0;
}


std::string DirectedGraph::getGraphType() const { return "Directed"; }

void DirectedGraph::toggleCondensedGraph() {
    m_showCondensedGraph = !m_showCondensedGraph;
}

bool DirectedGraph::isShowingCondensedGraph() const {
    return m_showCondensedGraph;
}

void DirectedGraph::buildCondensedGraph() {
    if (m_numComponents <= 0) return;
    
    m_nodeToComponent.clear();
    for (const auto& n : m_nodes) {
        m_nodeToComponent[n.getIndex()] = getComponentColor(n.getIndex());
    }

    m_condensed = CondensedGraphSolver{}.solve(*this, m_numComponents, m_nodeToComponent);
}
