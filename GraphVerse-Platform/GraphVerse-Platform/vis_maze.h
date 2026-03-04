#ifndef VIS_MAZE_H
#define VIS_MAZE_H
#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include <vector>
#include <unordered_map>
#include "directedgraph.h"

struct Cell { bool walls[4] = {true,true,true,true}; };

class VisMaze : public QWidget
{
    Q_OBJECT
public:
    explicit VisMaze(QWidget* parent = nullptr);
    ~VisMaze();
protected:
    void paintEvent(QPaintEvent*) override;
private slots:
    void onBfsClicked();
    void onDfsClicked();
    void onResetClicked();
    void onAnimationTick();
private:
    static const int COLS = 35, ROWS = 25, CELL_SIZE = 22;
    std::vector<std::vector<Cell>> m_maze;
    DirectedGraph* m_graph;
    QTimer* m_timer;
    std::vector<std::pair<int,int>> m_visitOrder;
    std::vector<std::pair<int,int>> m_finalPath;
    int m_animStep; bool m_animDone; bool m_showingPath; int m_pathStep;
    enum class AlgoType { None, BFS, DFS }; AlgoType m_currentAlgo;
    std::vector<std::vector<bool>> m_visited;
    std::vector<std::pair<int,int>> m_pathHighlight;
    void generateMaze();
    std::vector<std::pair<int,int>> runBFS(std::vector<std::pair<int,int>>& path);
    std::vector<std::pair<int,int>> runDFS(std::vector<std::pair<int,int>>& path);
    void startAnimation(AlgoType algo);
    int offsetX() const { return 10; }
    int offsetY() const { return 60; }
};
#endif
