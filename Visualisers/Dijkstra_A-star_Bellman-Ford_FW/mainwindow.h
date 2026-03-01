#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QLabel>
#include <vector>
#include <unordered_map>
#include "directedgraph.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

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
    Ui::MainWindow* ui;

    static const int COLS    = 20;
    static const int ROWS    = 14;
    static const int SPACING = 52;
    static const int NODE_R  = 10;

    DirectedGraph* m_graph;

    int m_startIdx;
    int m_endIdx;

    QTimer* m_timer;

    std::vector<PathStep> m_steps;
    int m_stepIdx;
    bool m_animDone;

    std::unordered_map<int,int> m_bestCost;
    std::unordered_map<int,int> m_parent;
    std::vector<int> m_finalPath;
    int m_currentNode;
    bool m_hasCurrent;

    enum class AlgoType { None, Dijkstra, AStar, BellmanFord, FloydWarshall };
    AlgoType m_currentAlgo;

    void buildGraph();
    void startAnimation(AlgoType algo);
    void resetAnimation();

    int  nodeAt(QPoint pos) const;

    int offsetX() const { return 30; }
    int offsetY() const { return 62; }
};

#endif // MAINWINDOW_H
