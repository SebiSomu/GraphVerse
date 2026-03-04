#include "node.h"

Node::Node(int index, QPoint p) : m_index(index), m_coord(p)
{
}

void Node::setIndex(int index) {
    m_index = index;
}

void Node::setCoord(QPoint p) {
    m_coord = p;
}

int Node::getIndex() const {
    return m_index;
}

QPoint& Node::getCoord() {
    return m_coord;
}

const QPoint& Node::getCoord() const
{
    return m_coord;
}

int Node::getX() const {
    return m_coord.x();
}

int Node::getY() const {
    return m_coord.y();
}
