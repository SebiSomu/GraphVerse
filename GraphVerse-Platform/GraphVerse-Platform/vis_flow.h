#ifndef VIS_FLOW_H
#define VIS_FLOW_H
#include <QWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLabel>
#include <QHideEvent>
#include "flownetwork.h"

class NetworkWidget;

class VisFlow : public QWidget {
    Q_OBJECT
public:
    explicit VisFlow(QWidget* parent = nullptr);
    ~VisFlow();
protected:
    void hideEvent(QHideEvent* event) override;
private slots:
    void onModeChanged(int id);
    void onStartAlgorithm();
    void onNextStep();
    void onEliminateNegCycles();
    void onReset();
    void onClear();
private:
    FlowNetwork* m_network;
    NetworkWidget* m_networkWidget;
    QPushButton* m_btnNextStep;
    QButtonGroup* m_modeGroup;
    QLabel* m_statusLabel;
    void setupUI();
};

class NetworkWidget : public QWidget {
    Q_OBJECT
public:
    enum class InteractionMode { Mode_AddNode, Mode_AddEdge, Mode_SetSource, Mode_SetSink, Mode_View };
    explicit NetworkWidget(QWidget* parent = nullptr);
    void setNetwork(FlowNetwork* network);
    void setMode(InteractionMode mode);
    void setIterationIndex(int index);
    int getIterationIndex() const { return m_iterationIndex; }
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
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
    void drawNode(QPainter& painter, int index, const QPointF& pos, bool isSource, bool isSink);
    void drawEdge(QPainter& painter, const QPointF& from, const QPointF& to,
                  int capacity, int residual, int cost, bool isMinCut, bool isOnPath, bool isReverse = false);
    int getNodeAt(const QPoint& pos);
};

#endif
