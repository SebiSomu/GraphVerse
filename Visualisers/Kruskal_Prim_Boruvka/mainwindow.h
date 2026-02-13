#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPaintEvent>
#include <QTimer>
#include <vector>
#include "undirectedgraph.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void paintEvent(QPaintEvent* e) override;

private slots:
    void onKruskalClicked();
    void onPrimClicked();
    void onBoruvkaClicked();
    void onResetClicked();
    void onAnimationTick();

private:
    Ui::MainWindow *ui;

    // Grid dimensions
    static const int COLS    = 20;
    static const int ROWS    = 14;
    static const int SPACING = 52;
    static const int NODE_R  = 10;

    UndirectedGraph* m_graph;

    QTimer*               m_timer;
    std::vector<MSTStep>  m_steps;
    int                   m_stepIdx;
    bool                  m_animDone;

    std::vector<MSTStep>  m_accepted;
    std::vector<MSTStep>  m_rejected;
    MSTStep               m_current;
    bool                  m_hasCurrent;

    enum class AlgoType { None, Kruskal, Prim, Boruvka };
    AlgoType m_currentAlgo;

    void buildGraph();
    void startAnimation(AlgoType algo);

    int offsetX() const { return 30; }
    int offsetY() const { return 62; }
};

#endif // MAINWINDOW_H
