#ifndef VIS_SHORTEST_H
#define VIS_SHORTEST_H
#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include <QLabel>
#include <unordered_map>
#include "directedgraph.h"

class VisShortest : public QWidget {
    Q_OBJECT
public:
    explicit VisShortest(QWidget* parent = nullptr);
    ~VisShortest();
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
private slots:
    void onDijkstraClicked();
    void onAStarClicked();
    void onBellmanFordClicked();
    void onFloydWarshallClicked();
    void onResetClicked();
    void onAnimationTick();
private:
    static const int COLS=20, ROWS=9, SPACING=65, NODE_R=8;
    int offsetX() const { return 60; }
    int offsetY() const { return 70; }
    enum class AlgoType { None, Dijkstra, AStar, BellmanFord, FloydWarshall };
    DirectedGraph* m_graph;
    QTimer* m_timer;
    QLabel* m_statusLabel;
    int m_startIdx, m_endIdx;
    std::vector<PathStep> m_steps;
    std::unordered_map<int,int> m_bestCost, m_parent;
    std::vector<int> m_finalPath;
    int m_stepIdx; bool m_animDone; int m_currentNode; bool m_hasCurrent;
    AlgoType m_currentAlgo;
    void buildGraph();
    void resetAnimation();
    void updateLabel();
    int nodeAt(QPoint pos) const;
    void startAnimation(AlgoType algo);
};
#endif
