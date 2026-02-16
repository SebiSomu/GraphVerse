#pragma once
#include <string>

class Node
{
    int m_index;
    std::string m_name;
public:
    Node(int index = 0, const std::string& name = "");
    void setIndex(int index);
    void setName(const std::string& name);
    int getIndex() const;
    const std::string& getName() const;
};