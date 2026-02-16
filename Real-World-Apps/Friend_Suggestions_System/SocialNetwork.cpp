#include "socialnetwork.h"
#include <algorithm>
#include <iostream>
#include <iomanip>

void SocialNetwork::addUser(const std::string& name) {
    if (!m_graph.findByName(name))
        m_graph.addNode(name);
}

void SocialNetwork::addFriendship(const std::string& a, const std::string& b) {
    addUser(a);
    addUser(b);
    Node* nodeA = m_graph.findByName(a);
    Node* nodeB = m_graph.findByName(b);
    if (nodeA && nodeB)
        m_graph.addEdge(*nodeA, *nodeB);
}

int SocialNetwork::countMutual(const std::string& a, const std::string& b) const {
    BFSResult ra = m_graph.bfs(a, 1);
    BFSResult rb = m_graph.bfs(b, 1);
    const Node* nodeA = m_graph.findByName(a);
    const Node* nodeB = m_graph.findByName(b);
    if (!nodeA || !nodeB) return 0;
    int count = 0;
    for (const auto& [idx, dist] : ra.dist) {
        if (idx == nodeA->getIndex()) continue;
        if (dist == 1 && rb.dist.count(idx) && rb.dist.at(idx) == 1)
            count++;
    }
    return count;
}

std::vector<Suggestion> SocialNetwork::suggestFriends(const std::string& user, int maxDepth) const {
    const Node* userNode = m_graph.findByName(user);
    if (!userNode) return {};

    BFSResult result = m_graph.bfs(user, maxDepth);
    int userIdx = userNode->getIndex();

    std::vector<Suggestion> suggestions;
    for (const auto& [idx, dist] : result.dist) {
        if (idx == userIdx) continue;
        if (dist == 1) continue;

        const Node* candidate = m_graph.findByIndex(idx);
        if (!candidate) continue;

        Suggestion s;
        s.name = candidate->getName();
        s.distance = dist;
        s.mutualCount = countMutual(user, s.name);
        suggestions.push_back(s);
    }

    std::sort(suggestions.begin(), suggestions.end(),
        [](const Suggestion& a, const Suggestion& b) {
            if (a.distance != b.distance) return a.distance < b.distance;
            return a.mutualCount > b.mutualCount;
        });

    return suggestions;
}

void SocialNetwork::printGraph() const {
    auto adj = m_graph.buildAdjList();
    std::cout << "\n=== Social Network Graph ===\n";
    for (const auto& n : m_graph.getNodes()) {
        std::cout << "  " << std::setw(12) << std::left << n.getName() << " -> ";
        bool first = true;
        for (int neighborIdx : adj[n.getIndex()]) {
            const Node* nb = m_graph.findByIndex(neighborIdx);
            if (nb) {
                if (!first) std::cout << ", ";
                std::cout << nb->getName();
                first = false;
            }
        }
        std::cout << "\n";
    }
}

void SocialNetwork::printSuggestions(const std::string& user, int maxDepth) const {
    std::cout << "\n=== Friend Suggestions for \"" << user
        << "\" (BFS depth=" << maxDepth << ") ===\n";

    auto suggestions = suggestFriends(user, maxDepth);

    if (suggestions.empty()) {
        std::cout << "  No suggestions found.\n";
        return;
    }

    std::cout << std::left
        << std::setw(16) << "Name"
        << std::setw(12) << "Distance"
        << std::setw(16) << "Common friends"
        << "\n"
        << std::string(44, '-') << "\n";

    for (const auto& s : suggestions) {
        std::cout << std::left
            << std::setw(16) << s.name
            << std::setw(12) << s.distance
            << std::setw(16) << s.mutualCount
            << "\n";
    }
}

void SocialNetwork::printSuggestionsAll(int maxDepth) const {
    for (const auto& node : m_graph.getNodes())
        printSuggestions(node.getName(), maxDepth);
}