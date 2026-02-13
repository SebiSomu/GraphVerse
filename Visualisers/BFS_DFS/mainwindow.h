#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QTimer>
#include <QQueue>
#include <QStack>
#include <vector>
#include <unordered_map>
#include "directedgraph.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Maze cell
struct Cell {
    bool walls[4] = {true, true, true, true}; // N E S W
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void paintEvent(QPaintEvent* e) override;

private slots:
    void onBfsClicked();
    void onDfsClicked();
    void onResetClicked();
    void onAnimationTick();

private:
    Ui::MainWindow *ui;

    static const int COLS = 35;
    static const int ROWS = 25;
    static const int CELL = 22;

    std::vector<std::vector<Cell>> m_maze;

    // DirectedGraph holds the maze — BFS/DFS are called on it
    DirectedGraph* m_graph;

    QTimer* m_timer;
    std::vector<std::pair<int,int>> m_visitOrder;
    std::vector<std::pair<int,int>> m_finalPath;
    int  m_animStep;
    bool m_animDone;
    bool m_showingPath;
    int  m_pathStep;

    enum class AlgoType { None, BFS, DFS };
    AlgoType m_currentAlgo;

    std::vector<std::vector<bool>> m_visited;
    std::vector<std::pair<int,int>> m_pathHighlight;

    void generateMaze();
    std::vector<std::pair<int,int>> runBFS(std::vector<std::pair<int,int>>& path);
    std::vector<std::pair<int,int>> runDFS(std::vector<std::pair<int,int>>& path);
    void startAnimation(AlgoType algo);

    int offsetX() const { return 10; }
    int offsetY() const { return 60; }
};
#endif // MAINWINDOW_H
