#include "undirectedgraph.h"
#include <cmath>
#include <QtGui/QPainter>

UndirectedGraph::UndirectedGraph() = default;

void UndirectedGraph::addEdge(Node &f, Node &s, int cost)
{
    m_edges.emplace_back(&f, &s, cost);
    m_componentsColors.clear();
    m_numComponents = 0;
}

std::string_view UndirectedGraph::getGraphType() const { return "Undirected"; }

