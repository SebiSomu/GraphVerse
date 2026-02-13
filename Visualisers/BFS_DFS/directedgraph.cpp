#include "directedgraph.h"
#include <QQueue>
#include <QStack>
#include <unordered_set>

DirectedGraph::DirectedGraph() {}

void DirectedGraph::addEdge(Node &f, Node &s)
{
    Edge newEdge(&f, &s);
    m_edges.push_back(newEdge);
}

void DirectedGraph::drawEdge(QPainter &p) const
{
    const double arrowSize   = 15.0;
    const double nodeRadius  = 10.0;

    for(const auto &ed : m_edges) {
        QPointF start(ed.getFirst().getX(),  ed.getFirst().getY());
        QPointF end  (ed.getSecond().getX(), ed.getSecond().getY());
        QLineF  line(start, end);
        double  angle = std::atan2(line.dy(), line.dx());

        QPointF adjStart = start + QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));
        QPointF adjEnd   = end   - QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));

        p.drawLine(adjStart, adjEnd);
        p.drawLine(adjEnd, adjEnd - QPointF(arrowSize * cos(angle - M_PI/6), arrowSize * sin(angle - M_PI/6)));
        p.drawLine(adjEnd, adjEnd - QPointF(arrowSize * cos(angle + M_PI/6), arrowSize * sin(angle + M_PI/6)));
    }

    for(const auto &n : m_nodes) {
        QRect r(n.getX() - (int)nodeRadius, n.getY() - (int)nodeRadius,
                (int)nodeRadius*2, (int)nodeRadius*2);
        p.drawEllipse(r);
        p.drawText(r, Qt::AlignCenter, QString::number(n.getIndex()));
    }
}

std::string DirectedGraph::getGraphType() const { return "Directed"; }

std::unordered_map<int, std::vector<int>> DirectedGraph::buildAdjacencyList() const
{
    std::unordered_map<int, std::vector<int>> adj;
    for(const auto& n  : m_nodes) adj[n.getIndex()] = {};
    for(const auto& ed : m_edges)
        adj[ed.getFirst().getIndex()].push_back(ed.getSecond().getIndex());
    return adj;
}

std::vector<int> DirectedGraph::bfs(int startIndex,
                                    int stopAt,
                                    std::function<void(int)>     onVisit,
                                    std::function<void(int,int)> onEdge) const
{
    auto adj = buildAdjacencyList();
    std::unordered_set<int> visited;
    std::vector<int> order;
    QQueue<int> q;

    visited.insert(startIndex);
    q.enqueue(startIndex);

    while(!q.isEmpty()) {
        int cur = q.dequeue();
        order.push_back(cur);
        if(onVisit) onVisit(cur);
        if(cur == stopAt) break;

        for(int nb : adj[cur]) {
            if(!visited.count(nb)) {
                visited.insert(nb);
                if(onEdge) onEdge(cur, nb);
                q.enqueue(nb);
            }
        }
    }
    return order;
}

std::vector<int> DirectedGraph::dfs(int startIndex,
                                    int stopAt,
                                    std::function<void(int)>     onVisit,
                                    std::function<void(int,int)> onEdge) const
{
    auto adj = buildAdjacencyList();
    std::unordered_set<int> visited;
    std::vector<int> order;
    QStack<int> stk;

    QStack<std::pair<int,int>> stkWithParent;
    stkWithParent.push({startIndex, -1});
    visited.insert(startIndex);

    while(!stkWithParent.isEmpty()) {
        auto [cur, par] = stkWithParent.top(); stkWithParent.pop();
        order.push_back(cur);
        if(onVisit) onVisit(cur);
        if(onEdge && par != -1) onEdge(par, cur);
        if(cur == stopAt) break;

        auto& nbList = adj[cur];
        for(int i = (int)nbList.size() - 1; i >= 0; i--) {
            int nb = nbList[i];
            if(!visited.count(nb)) {
                visited.insert(nb);
                stkWithParent.push({nb, cur});
            }
        }
    }
    return order;
}
