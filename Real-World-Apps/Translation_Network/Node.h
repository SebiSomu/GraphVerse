#pragma once
#include <string>

class Node
{
    int m_index;
    std::string m_language;
public:
    Node(int index = 0, const std::string& language = "");
    void setIndex(int index);
    void setLanguage(const std::string& language);
    int getIndex() const;
    const std::string& getLanguage() const;
};