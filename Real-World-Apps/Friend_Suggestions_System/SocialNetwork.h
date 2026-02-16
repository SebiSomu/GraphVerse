#pragma once
#include "undirectedgraph.h"
#include <string>
#include <vector>

struct Suggestion {
    std::string name;
    int distance;
    int mutualCount;
};

class SocialNetwork
{
public:
    void addUser(const std::string& name);
    void addFriendship(const std::string& a, const std::string& b);
    std::vector<Suggestion> suggestFriends(const std::string& user, int maxDepth = 2) const;

    void printGraph() const;
    void printSuggestions(const std::string& user, int maxDepth = 2) const;
    void printSuggestionsAll(int maxDepth = 2) const;

private:
    UndirectedGraph m_graph;

    int countMutual(const std::string& a, const std::string& b) const;
};