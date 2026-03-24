#ifndef VIS_MST_H
#define VIS_MST_H
#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QElapsedTimer>
#include "algorithms/graph_algorithm_types.h"
#include "undirectedgraph.h"
#include <memory>

class VisMST : public QWidget {
    Q_OBJECT
public:
    explicit VisMST(QWidget* parent = nullptr);
    ~VisMST();
protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
private slots:
    void onKruskalClicked();
    void onPrimClicked();
    void onBoruvkaClicked();
    void onResetClicked();
    void onAnimationTick();
    void onPauseClicked();
    void onSpeedChanged(int value);
private:
    static const int COLS=20, ROWS=9, SPACING=65, NODE_R=8;
    int offsetX() const { return 60; }
    int offsetY() const { return 70; }
    enum class AlgoType { None, Kruskal, Prim, Boruvka };
    std::unique_ptr<UndirectedGraph> m_graph;
    QTimer* m_timer;
    QLabel* m_statusLabel;
    QLabel* m_timerLabel;
    QLabel* m_speedLabel;
    QPushButton* m_pauseBtn;
    QSlider* m_speedSlider;
    QWidget* m_toolbar;
    std::vector<MSTStep> m_steps, m_accepted, m_rejected;
    MSTStep m_current;
    int m_stepIdx; bool m_animDone; bool m_hasCurrent; AlgoType m_currentAlgo;
    bool m_paused;
    qint64 m_elapsedMs;
    QElapsedTimer m_elapsedClock;
    void buildGraph();
    void updateLabel();
    void updateTimerLabel();
    int currentInterval() const;
    void startAnimation(AlgoType algo);
};
#endif
