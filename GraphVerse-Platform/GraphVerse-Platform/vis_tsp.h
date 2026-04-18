#ifndef VIS_TSP_H
#define VIS_TSP_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <vector>
#include <QHideEvent>
#include <QString>
#include <QPointF>
#include <unordered_map>
#include "undirectedgraph.h"
#include "algorithms/graph_algorithm_types.h"
#include "algorithms/kruskal_solver.h"
#include "algorithms/floyd_warshall_solver.h"
#include "GraphRenderer.h"

class VisTSP : public QWidget {
    Q_OBJECT
public:
    explicit VisTSP(QWidget* parent = nullptr);
    ~VisTSP();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private slots:
    void onLoadGraph();
    void onFloydWarshall();
    void onGenerateKn();
    void onCalculateMST();
    void onGenerateTSP();
    void onReset();

private:

    enum class DisplayMode {
        InitialGraph,
        CompleteGraph,
        MST,
        TSPCircuit
    };

    UndirectedGraph m_graph;
    std::vector<std::vector<int>> m_distMatrix;
    std::vector<int> m_tspCircuit;
    std::vector<MSTStep> m_mstSteps; 

    DisplayMode m_displayMode = DisplayMode::InitialGraph;
    bool m_graphLoaded = false;
    bool m_floydDone = false;
    bool m_knGenerated = false;
    bool m_mstDone = false;
    bool m_tspDone = false;

    // UI Components
    QWidget* m_toolbar;
    QLabel* m_statusLabel;
    QPushButton* m_btnLoad;
    QPushButton* m_btnFloyd;
    QPushButton* m_btnKn;
    QPushButton* m_btnMST;
    QPushButton* m_btnTSP;
    QPushButton* m_btnReset;

    void setupUi();
    bool loadFromFile(const QString& filename);
    void scaleCoordinates();
    
    void dfsPreorder(int node, std::vector<bool>& visited, 
                     const std::vector<std::vector<int>>& mstAdj, 
                     std::vector<int>& circuit);

    // Helpers
    QPointF getCanvasCoord(const QPointF& coord) const;
};

#endif // VIS_TSP_H
