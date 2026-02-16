#include "Node.h"

Node::Node(int index, const std::string& name)
    : m_index(index), m_name(name) {
}

void Node::setIndex(int index) { m_index = index; }
void Node::setName(const std::string& name) { m_name = name; }
int Node::getIndex() const { return m_index; }
const std::string& Node::getName() const { return m_name; }