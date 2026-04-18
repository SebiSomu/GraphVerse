#include "astar_solver.h"
#include "graph_utils.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <limits>
#include <QLineF>

std::vector<PathStep> AStarSolver::solve(const IGraphData& graph, int startIdx, int endIdx,
                                          std::vector<int>& outPath) const
{
    auto adj = GraphUtils::buildWeightedAdjList(graph);
    std::vector<PathStep> steps;

    std::unordered_map<int, QPoint> pos;
    for (const auto& n : graph.getNodes()) pos[n.getIndex()] = n.getCoord();

    // Compute minimum cost/distance ratio for admissible heuristic
    double minRatio = std::numeric_limits<double>::infinity();
    for (const auto& ed : graph.getEdges()) {
        QPointF A(ed.getFirst().getX(), ed.getFirst().getY());
        QPointF B(ed.getSecond().getX(), ed.getSecond().getY());
        double L = QLineF(A, B).length();
        if (L > 1e-6) {
            double r = static_cast<double>(ed.getCost()) / L;
            if (r < minRatio) minRatio = r;
        }
    }
    if (!std::isfinite(minRatio) || minRatio <= 0.0) minRatio = 0.0;

    auto heuristic = [&](int a, int b) -> int {
        if (minRatio == 0.0) return 0;
        double dx = static_cast<double>(pos[a].x() - pos[b].x());
        double dy = static_cast<double>(pos[a].y() - pos[b].y());
        double eu = std::sqrt(dx * dx + dy * dy);
        double h = eu * minRatio;
        if (h < 0) h = 0;
        return static_cast<int>(h);
    };

    std::unordered_map<int, int> gCost, parent;
    std::unordered_set<int> closed;
    for (const auto& n : graph.getNodes()) gCost[n.getIndex()] = INF_COST;
    gCost[startIdx] = 0; parent[startIdx] = -1;

    using T = std::pair<int, int>;
    std::priority_queue<T, std::vector<T>, std::greater<>> open;
    open.emplace(heuristic(startIdx, endIdx), startIdx);

    while (!open.empty()) {
        auto [f, u] = open.top(); open.pop();
        if (closed.count(u)) continue;
        closed.insert(u);
        steps.push_back({u, parent.count(u) ? parent[u] : -1, gCost[u], false});
        if (u == endIdx) break;
        for (auto& [v, w] : adj[u]) {
            if (int ng = gCost[u] + w; ng < gCost[v]) {
                gCost[v] = ng;
                parent[v] = u;
                open.emplace(ng + heuristic(v, endIdx), v);
            }
        }
    }

    outPath = GraphUtils::reconstructPath(endIdx, parent);
    GraphUtils::markFinalPath(steps, outPath);
    return steps;
}
