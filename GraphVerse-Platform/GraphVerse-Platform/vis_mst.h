#ifndef VIS_MST_H
#define VIS_MST_H
#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include <QLabel>
#include "undirectedgraph.h"

class VisMST : public QWidget {
    Q_OBJECT
public:
    explicit VisMST(QWidget* parent = nullptr);
    ~VisMST();
protected:
    void paintEvent(QPaintEvent*) override;
private slots:
    void onKruskalClicked();
    void onPrimClicked();
    void onBoruvkaClicked();
    void onResetClicked();
    void onAnimationTick();
private:
    static const int COLS=20, ROWS=9, SPACING=65, NODE_R=8;
    int offsetX() const { return 60; }
    int offsetY() const { return 70; }
    enum class AlgoType { None, Kruskal, Prim, Boruvka };
    UndirectedGraph* m_graph;
    QTimer* m_timer;
    QLabel* m_statusLabel;
    std::vector<MSTStep> m_steps, m_accepted, m_rejected;
    MSTStep m_current;
    int m_stepIdx; bool m_animDone; bool m_hasCurrent; AlgoType m_currentAlgo;
    void buildGraph();
    void updateLabel();
    void startAnimation(AlgoType algo);
};
#endif
