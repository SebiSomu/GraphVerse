#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QPaintEvent>
#include <QMessageBox>
#include "tspgraph.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

enum class DisplayMode {
    InitialGraph,
    CompleteGraph,
    MST,
    TSPCircuit
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onLoadGraph();
    void onFloydWarshall();
    void onGenerateKn();
    void onCalculateMST();
    void onGenerateTSP();

private:
    Ui::MainWindow *ui;
    TSPGraph m_graph;
    DisplayMode m_displayMode;
    bool m_graphLoaded;
    bool m_floydDone;
    bool m_knGenerated;
    bool m_mstDone;
    bool m_tspDone;
    
    void scaleCoordinates();
};

#endif // MAINWINDOW_H
