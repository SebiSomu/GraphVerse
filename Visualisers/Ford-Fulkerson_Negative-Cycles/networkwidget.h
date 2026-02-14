#ifndef NETWORKWIDGET_H
#define NETWORKWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QLabel>
#include <cmath>
#include "flownetwork.h"

class NetworkWidget : public QWidget
{
    Q_OBJECT

public:
    enum InteractionMode {
        Mode_AddNode,
        Mode_AddEdge,
        Mode_SetSource,
        Mode_SetSink,
        Mode_View
    };

private:
    FlowNetwork* m_network;
    InteractionMode m_mode;

    int m_iterationIndex;
    bool m_showFinalResult;

    int m_firstNode;
    QPoint m_tempMousePos;
    bool m_isDragging;

    static constexpr double NODE_RADIUS = 20.0;
    static constexpr double ARROW_SIZE = 12.0;

public:
    explicit NetworkWidget(QWidget *parent = nullptr);

    void setNetwork(FlowNetwork* network);
    void setMode(InteractionMode mode);
    void setIterationIndex(int index);

    int getIterationIndex() const { return m_iterationIndex; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void drawNode(QPainter& painter, int index, const QPointF& pos, bool isSource, bool isSink);
    void drawEdge(QPainter& painter, const QPointF& from, const QPointF& to,
                  int capacity, int residual, int cost, bool isMinCut, bool isOnPath,
                  bool isReverse = false);

    int getNodeAt(const QPoint& pos);
};

#endif // NETWORKWIDGET_H
