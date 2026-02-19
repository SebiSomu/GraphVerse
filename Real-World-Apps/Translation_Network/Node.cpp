#include "Node.h"

Node::Node(int index, const std::string& language)
    : m_index(index), m_language(language) {
}

void Node::setIndex(int index) { m_index = index; }
void Node::setLanguage(const std::string& language) { m_language = language; }
int Node::getIndex() const { return m_index; }
const std::string& Node::getLanguage() const { return m_language; }