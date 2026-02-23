#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPushButton>
#include "graph.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void mouseReleaseEvent(QMouseEvent* m) override;
    void mousePressEvent(QMouseEvent* m) override;
    void mouseMoveEvent(QMouseEvent* m ) override;
    void paintEvent(QPaintEvent* e ) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onCheckBoxStateChanged(int state);
    void onFindComponents();
    void onToggleCondensedView();

private:
    Ui::MainWindow *ui;
    Graph* m_graph;
    Node* m_firstNode;
    Node* m_draggedNode;
    bool m_dragging = false;
    bool m_hasMoved = false;
    QPoint m_pressPosition;
    QPushButton* m_btnComponents;
    QPushButton* m_btnToggleCondensed;

    void recreateGraph(bool directed);
    void updateButtonText();
};
#endif // MAINWINDOW_H
