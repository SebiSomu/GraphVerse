#include "translationnetwork.h"
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <utility>

void TranslationNetwork::addLanguage(const std::string& language) {
    if (!m_graph.findByLanguage(language))
        m_graph.addNode(language);
}

void TranslationNetwork::addTranslationPair(const std::string& lang1, const std::string& lang2, int difficulty) {
    addLanguage(lang1);
    addLanguage(lang2);
    Node* n1 = m_graph.findByLanguage(lang1);
    Node* n2 = m_graph.findByLanguage(lang2);
    if (n1 && n2) {
        m_graph.addEdge(*n1, *n2, difficulty);
        m_graph.addEdge(*n2, *n1, difficulty);
    }
}

void TranslationNetwork::printFullGraph() const {
    std::cout << "\n=== Translation Network - Full Graph ===\n";
    std::cout << "Total languages: " << m_graph.getNodes().size() << "\n";
    std::cout << "Total translation pairs: " << m_graph.getEdges().size() / 2 << "\n\n";

    std::cout << std::left << std::setw(20) << "Language 1"
        << std::setw(20) << "Language 2"
        << std::setw(12) << "Difficulty"
        << "\n" << std::string(52, '-') << "\n";

    for (size_t i = 0; i < m_graph.getEdges().size(); i++) {
        const auto& e = m_graph.getEdges()[i];
        if (e.getFirst().getIndex() > e.getSecond().getIndex()) continue;

        std::cout << std::left
            << std::setw(20) << e.getFirst().getLanguage()
            << std::setw(20) << e.getSecond().getLanguage()
            << std::setw(12) << e.getCost()
            << "\n";
    }
}

void TranslationNetwork::computeAndPrintMST() const {
    auto mst = m_graph.kruskalMST();

    std::cout << "\n=== Minimum Spanning Tree (Kruskal) ===\n";
    std::cout << "Minimum translation pairs needed: " << mst.size() << "\n";

    int totalCost = 0;
    for (const auto& e : mst)
        totalCost += e.getCost();

    std::cout << "Total training effort: " << totalCost << "\n\n";

    std::cout << std::left << std::setw(20) << "Language 1"
        << std::setw(20) << "Language 2"
        << std::setw(12) << "Difficulty"
        << "\n" << std::string(52, '-') << "\n";

    for (const auto& e : mst) {
        std::cout << std::left
            << std::setw(20) << e.getFirst().getLanguage()
            << std::setw(20) << e.getSecond().getLanguage()
            << std::setw(12) << e.getCost()
            << "\n";
    }
}

void TranslationNetwork::printMSTAsTree(const std::string& rootLanguage) const {
    auto mstAdj = buildMSTAdjacencyList();

    if (mstAdj.find(rootLanguage) == mstAdj.end() && !m_graph.findByLanguage(rootLanguage)) {
        std::cout << "\n[Error] Language " << rootLanguage << " not found.\n";
        return;
    }

    std::cout << "\n=== MST as Tree (BFS traversal from " << rootLanguage << ") ===\n\n";

    std::unordered_set<std::string> visited;
    std::queue<std::pair<std::string, int>> q;

    q.push({ rootLanguage, 0 });
    visited.insert(rootLanguage);

    int edgeCount = 0;
    while (!q.empty()) {
        auto [current, depth] = q.front();
        q.pop();

        if (depth == 0) {
            std::cout << "ROOT: " << current << "\n";
        }

        for (const auto& [neighbor, cost] : mstAdj[current]) {
            if (!visited.count(neighbor)) {
                visited.insert(neighbor);
                q.push({ neighbor, depth + 1 });

                edgeCount++;
                std::string childIndent(depth * 2, ' ');
                std::cout << childIndent << " +-- [Edge #" << edgeCount
                    << ", Cost=" << cost << "] " << neighbor << "\n";
            }
        }
    }

    std::cout << "\nTotal edges in MST: " << edgeCount << "\n";
}

TranslationNetwork::AdjacencyList TranslationNetwork::buildMSTAdjacencyList() const {
    auto mst = m_graph.kruskalMST();
    AdjacencyList adj;

    for (const auto& e : mst) {
        std::string lang1 = e.getFirst().getLanguage();
        std::string lang2 = e.getSecond().getLanguage();
        int cost = e.getCost();

        adj[lang1].push_back({ lang2, cost });
        adj[lang2].push_back({ lang1, cost });
    }
    return adj;
}