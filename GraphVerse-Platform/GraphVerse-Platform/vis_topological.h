#ifndef VIS_TOPOLOGICAL_H
#define VIS_TOPOLOGICAL_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QHideEvent>
#include "directedgraph.h"
#include "rendering/mutable_graph_renderer.h"
#include <memory>
#include <vector>

class VisTopologicalSort : public QWidget
{
    Q_OBJECT
public:
    explicit VisTopologicalSort(QWidget* parent = nullptr);
    ~VisTopologicalSort();
protected:
    void paintEvent(QPaintEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void hideEvent(QHideEvent* event) override;
private slots:
    void onSortClicked();
    void onClearClicked();
private:
    std::unique_ptr<DirectedGraph> m_graph;
    Node* m_firstNode;
    Node* m_draggedNode;
    bool m_dragging = false;
    bool m_hasMoved = false;
    QPoint m_pressPosition;
    QPushButton* m_btnSort;
    QPushButton* m_btnClear;
    QLabel* m_resultLabel;
    
    // Result data
    std::vector<int> m_sortedOrder;
    
    // Renderer for mutable graphs
    std::unique_ptr<MutableGraphRenderer> m_renderer;
    MutableRenderSettings m_renderSettings;
    AnimationState m_animState;
};

#endif // VIS_TOPOLOGICAL_H
