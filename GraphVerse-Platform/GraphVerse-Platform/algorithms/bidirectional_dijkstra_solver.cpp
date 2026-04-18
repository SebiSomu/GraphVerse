#include "bidirectional_dijkstra_solver.h"
#include "graph_utils.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

std::vector<PathStep> BidirectionalDijkstraSolver::solve(const Graph& graph, int startIdx, int endIdx,
                                                         std::vector<int>& outPath) const
{
    std::unordered_map<int, std::vector<std::pair<int, int>>> adjF, adjB;
    for (const auto& n : graph.getNodes()) {
        adjF[n.getIndex()] = {};
        adjB[n.getIndex()] = {};
    }
    for (const auto& ed : graph.getEdges()) {
        int u = ed.getFirst().getIndex();
        int v = ed.getSecond().getIndex();
        int w = ed.getCost();
        adjF[u].emplace_back(v, w);
        adjB[v].emplace_back(u, w);
    }

    std::unordered_map<int, int> distF, distB, parentF, parentB;
    std::unordered_set<int> settledF, settledB;
    std::vector<PathStep> steps;

    for (const auto& n : graph.getNodes()) {
        distF[n.getIndex()] = INF_COST;
        distB[n.getIndex()] = INF_COST;
    }
    
    distF[startIdx] = 0; parentF[startIdx] = -1;
    distB[endIdx] = 0; parentB[endIdx] = -1;

    using T = std::pair<int, int>;
    std::priority_queue<T, std::vector<T>, std::greater<>> pqF, pqB;
    pqF.emplace(0, startIdx);
    pqB.emplace(0, endIdx);

    int mu = INF_COST;
    int meetingNode = -1;

    if (startIdx == endIdx) {
        outPath = {startIdx};
        steps.push_back({startIdx, -1, 0, true});
        return steps;
    }

    while (!pqF.empty() && !pqB.empty()) {
        int topF_dist = pqF.top().first;
        int topB_dist = pqB.top().first;

        if (topF_dist + topB_dist >= mu) {
            break;
        }

        bool isForward = (topF_dist <= topB_dist);

        if (isForward) {
            auto [d, u] = pqF.top(); pqF.pop();
            if (settledF.count(u)) continue;
            settledF.insert(u);
            steps.push_back({u, parentF.count(u) ? parentF[u] : -1, d, false});

            for (auto& [v, w] : adjF[u]) {
                if (distF[u] + w < distF[v]) {
                    distF[v] = distF[u] + w;
                    parentF[v] = u;
                    pqF.emplace(distF[v], v);
                    
                    if (distB.count(v) && distF[v] + distB[v] < mu) {
                        mu = distF[v] + distB[v];
                        meetingNode = v;
                    }
                }
            }
        } else {
            auto [d, u] = pqB.top(); pqB.pop();
            if (settledB.count(u)) continue;
            settledB.insert(u);
            steps.push_back({u, parentB.count(u) ? parentB[u] : -1, d, false});

            for (auto& [v, w] : adjB[u]) {
                if (distB[u] + w < distB[v]) {
                    distB[v] = distB[u] + w;
                    parentB[v] = u;
                    pqB.emplace(distB[v], v);

                    if (distF.count(v) && distF[v] + distB[v] < mu) {
                        mu = distF[v] + distB[v];
                        meetingNode = v;
                    }
                }
            }
        }
    }

    if (meetingNode != -1) {
        std::vector<int> pathF;
        for (int cur = meetingNode; cur != -1; cur = (parentF.count(cur) ? parentF[cur] : -1)) {
            pathF.push_back(cur);
        }
        std::reverse(pathF.begin(), pathF.end());

        std::vector<int> pathB;
        for (int cur = meetingNode; cur != -1; cur = (parentB.count(cur) ? parentB[cur] : -1)) {
            if (cur != meetingNode) pathB.push_back(cur);
        }

        outPath = pathF;
        outPath.insert(outPath.end(), pathB.begin(), pathB.end());
    } else {
        outPath = {};
    }

    GraphUtils::markFinalPath(steps, outPath);
    return steps;
}
