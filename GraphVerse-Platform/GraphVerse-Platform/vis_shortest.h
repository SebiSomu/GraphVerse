#ifndef VIS_SHORTEST_H
#define VIS_SHORTEST_H
#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QElapsedTimer>
#include <QHideEvent>
#include <unordered_map>
#include "algorithms/graph_algorithm_types.h"
#include "directedgraph.h"

class VisShortest : public QWidget {
    Q_OBJECT
public:
    explicit VisShortest(QWidget* parent = nullptr);
    ~VisShortest();
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void hideEvent(QHideEvent* event) override;
private slots:
    void onDijkstraClicked();
    void onAStarClicked();
    void onBellmanFordClicked();
    void onFloydWarshallClicked();
    void onResetClicked();
    void onAnimationTick();
    void onPauseClicked();
    void onSpeedChanged(int value);
private:
    static constexpr int COLS=20, ROWS=9, SPACING=65, NODE_R=8;
    constexpr int offsetX() const { return 60; }
    constexpr int offsetY() const { return 70; }
    enum class AlgoType { None, Dijkstra, AStar, BellmanFord, FloydWarshall };
    std::unique_ptr<DirectedGraph> m_graph;
    QTimer* m_timer;
    QLabel* m_statusLabel;
    QLabel* m_timerLabel;
    QLabel* m_speedLabel;
    QPushButton* m_pauseBtn;
    QSlider* m_speedSlider;
    QWidget* m_toolbar;
    int m_startIdx, m_endIdx;
    std::vector<PathStep> m_steps;
    std::unordered_map<int,int> m_bestCost, m_parent;
    std::vector<int> m_finalPath;
    int m_stepIdx; bool m_animDone; int m_currentNode; bool m_hasCurrent;
    AlgoType m_currentAlgo;
    bool m_paused;
    qint64 m_elapsedMs;
    QElapsedTimer m_elapsedClock;
    void buildGraph();
    void resetAnimation();
    void updateLabel();
    void updateTimerLabel();
    int currentInterval() const;
    int nodeAt(QPoint pos) const;
    void startAnimation(AlgoType algo);
};
#endif
