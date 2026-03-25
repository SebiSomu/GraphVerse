#ifndef NODE_H
#define NODE_H
#include <QPointF>
#include <QString>

class Node
{
    int m_index;
    QPointF m_coord;
    QString m_name;
public:
    Node(int index = 0, QPointF p = QPointF(0,0), QString name = "");
    void setIndex(int index);
    void setCoord(QPointF p);
    void setName(const QString& name);
    int getIndex() const;
    QPointF& getCoord();
    const QPointF& getCoord() const;
    double getX() const;
    double getY() const;
    QString getName() const;
};

#endif // NODE_H
