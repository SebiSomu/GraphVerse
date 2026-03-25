#include "node.h"

Node::Node(int index, QPointF p, QString name) : m_index(index), m_coord(p), m_name(name)
{
}

void Node::setIndex(int index) {
    m_index = index;
}

void Node::setCoord(QPointF p) {
    m_coord = p;
}

void Node::setName(const QString& name) {
    m_name = name;
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

QString Node::getName() const {
    return m_name;
}
