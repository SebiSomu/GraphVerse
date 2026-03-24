#include "condensed_graph_solver.h"
#include <unordered_set>
#include <string>

CondensedGraph CondensedGraphSolver::solve(const Graph& graph, int numComponents, const std::unordered_map<int, int>& nodeToComponent) const {
    CondensedGraph condensed;
    if (numComponents <= 0) return condensed;

    std::unordered_map<int, QPoint> componentPositions;
    std::unordered_map<int, int> nodeCount;

    for (const auto& n : graph.getNodes()) {
        int comp = nodeToComponent.at(n.getIndex());
        componentPositions[comp] += QPoint(n.getX(), n.getY());
        nodeCount[comp]++;
    }

    for (int i = 0; i < numComponents; i++) {
        if (nodeCount[i] > 0) {
            componentPositions[i] /= nodeCount[i];
            Node cn;
            cn.setCoord(componentPositions[i]);
            cn.setIndex(i + 1);
            condensed.nodes.push_back(cn);
        }
    }

    std::unordered_set<std::string> addedEdges;
    for (const auto& ed : graph.getEdges()) {
        int uComp = nodeToComponent.at(ed.getFirst().getIndex());
        int vComp = nodeToComponent.at(ed.getSecond().getIndex());
        if (uComp != vComp) {
            std::string key = std::to_string(uComp) + "->" + std::to_string(vComp);
            if (addedEdges.find(key) == addedEdges.end()) {
                Node* uNode = nullptr; Node* vNode = nullptr;
                for (auto& n : condensed.nodes) {
                    if (n.getIndex() == uComp + 1) uNode = &n;
                    if (n.getIndex() == vComp + 1) vNode = &n;
                }
                if (uNode && vNode) {
                    condensed.edges.emplace_back(uNode, vNode);
                    addedEdges.insert(key);
                }
            }
        }
    }

    return condensed;
}
