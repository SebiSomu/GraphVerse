#pragma once
#include "weightedgraph.h"
#include <string>
#include <unordered_map>

class TranslationNetwork
{
public:
    void addLanguage(const std::string& language);
    void addTranslationPair(const std::string& lang1, const std::string& lang2, int difficulty);
    void computeAndPrintMST() const;
    void printFullGraph() const;
    void printMSTAsTree(const std::string& rootLanguage = "English") const;

private:
    WeightedGraph m_graph;

    using AdjacencyList = std::unordered_map<std::string, std::vector<std::pair<std::string, int>>>;
    AdjacencyList buildMSTAdjacencyList() const;
};
