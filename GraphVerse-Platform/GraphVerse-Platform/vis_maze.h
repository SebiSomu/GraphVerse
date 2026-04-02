#ifndef VIS_MAZE_H
#define VIS_MAZE_H
#include <QWidget>
#include <QTimer>
#include <QEvent>
#include <QHideEvent>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <vector>
#include <unordered_map>
#include "directedgraph.h"
#include <memory>

struct Cell { bool walls[4] = {true,true,true,true}; };

class VisMaze : public QWidget
{
    Q_OBJECT
public:
    explicit VisMaze(QWidget* parent = nullptr);
    ~VisMaze();
protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void hideEvent(QHideEvent* event) override;
private slots:
    void onBfsClicked();
    void onDfsClicked();
    void onResetClicked();
    void onAnimationTick();
    void onPauseClicked();
    void onSpeedChanged(int value);
private:
    static constexpr int COLS = 35, ROWS = 25, CELL_SIZE = 22;
    std::vector<std::vector<Cell>> m_maze;
    std::unique_ptr<DirectedGraph> m_graph;
    QTimer* m_timer;
    std::vector<std::pair<int,int>> m_visitOrder;
    std::vector<std::pair<int,int>> m_finalPath;
    int m_animStep; bool m_animDone; bool m_showingPath; int m_pathStep;
    enum class AlgoType { None, BFS, DFS }; AlgoType m_currentAlgo;
    std::vector<std::vector<bool>> m_visited;
    std::vector<std::pair<int,int>> m_pathHighlight;
    QPushButton* m_pauseBtn;
    QLabel* m_statusLabel;
    QLabel* m_hintLabel;
    QLabel* m_speedLabel;
    QSlider* m_speedSlider;
    QWidget* m_toolbar;
    bool m_paused;
    int m_tickInterval;
    int currentInterval() const;
    void updateStatusLabel();
    void generateMaze();
    std::vector<std::pair<int,int>> runBFS(std::vector<std::pair<int,int>>& path);
    std::vector<std::pair<int,int>> runDFS(std::vector<std::pair<int,int>>& path);
    void startAnimation(AlgoType algo);
    constexpr int offsetX() const { return 10; }
    constexpr int offsetY() const { return 60; }
};
#endif
