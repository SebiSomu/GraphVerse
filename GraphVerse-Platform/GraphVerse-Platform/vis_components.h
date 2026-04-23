#ifndef VIS_COMPONENTS_H
#define VIS_COMPONENTS_H
#include <QWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QHideEvent>
#include "graph.h"
#include "rendering/component_graph_renderer.h"
#include <memory>

class VisComponents : public QWidget
{
    Q_OBJECT
public:
    explicit VisComponents(QWidget* parent = nullptr);
    ~VisComponents();
protected:
    void paintEvent(QPaintEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void hideEvent(QHideEvent* event) override;
private slots:
    void onCheckBoxStateChanged(int state);
    void onFindComponents();
    void onToggleCondensedView();
private:
    std::unique_ptr<Graph> m_graph;
    Node* m_firstNode;
    Node* m_draggedNode;
    bool m_dragging = false;
    bool m_hasMoved = false;
    QPoint m_pressPosition;
    QPushButton* m_btnComponents;
    QPushButton* m_btnToggleCondensed;
    QCheckBox* m_checkBox;
    void recreateGraph(bool directed);
    void updateButtonText();
    ComponentResult m_compResult;
    CondensedResult m_condensedResult;
    
    // Renderer for component coloring (ISP: separated concern)
    std::unique_ptr<ComponentGraphRenderer> m_renderer;
};
#endif
