#ifndef NODE_H
#define NODE_H
#include <QPointF>

class Node
{
    int m_index;
    QPointF m_coord;
public:
    Node(int index = 0, QPointF p = QPointF(0,0));
    void setIndex(int index);
    void setCoord(QPointF p);
    int getIndex() const;
    QPointF& getCoord();
    const QPointF& getCoord() const;
    double getX() const;
    double getY() const;
};

#endif // NODE_H
