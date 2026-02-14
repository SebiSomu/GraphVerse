#include "node.h"

Node::Node(int index, QPointF p) : m_index(index), m_coord(p)
{
}

void Node::setIndex(int index) {
    m_index = index;
}

void Node::setCoord(QPointF p) {
    m_coord = p;
}

int Node::getIndex() const {
    return m_index;
}

QPointF& Node::getCoord() {
    return m_coord;
}

const QPointF &Node::getCoord() const
{
    return m_coord;
}

double Node::getX() const {
    return m_coord.x();
}

double Node::getY() const {
    return m_coord.y();
}
