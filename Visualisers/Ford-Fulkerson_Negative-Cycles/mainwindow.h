#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QButtonGroup>
#include <QRadioButton>
#include "flownetwork.h"
#include "networkwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onModeChanged(int id);
    void onStartAlgorithm();
    void onNextStep();
    void onEliminateNegCycles();
    void onReset();
    void onClear();

private:
    Ui::MainWindow *ui;
    FlowNetwork* m_network;
    NetworkWidget* m_networkWidget;
    QButtonGroup* m_modeGroup;
    QPushButton* m_btnNextStep;
    QLabel* m_statusLabel;
    void setupUI();
};

#endif // MAINWINDOW_H
