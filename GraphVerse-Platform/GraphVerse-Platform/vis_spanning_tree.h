#ifndef VIS_SPANNING_TREE_H
#define VIS_SPANNING_TREE_H

#include "algorithms/i_spanning_tree.h"
#include "undirectedgraph.h"
#include <QElapsedTimer>
#include <QHideEvent>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QWidget>
#include <memory>
#include <set>
#include <unordered_set>

class VisSpanningTree : public QWidget {
  Q_OBJECT
public:
  explicit VisSpanningTree(QWidget *parent = nullptr);
  ~VisSpanningTree();
  void paintEvent(QPaintEvent *) override;
  void resizeEvent(QResizeEvent *) override;
  void showEvent(QShowEvent *event) override;
  void hideEvent(QHideEvent *event) override;
private slots:
  void onDFSClicked();
  void onBFSClicked();
  void onResetClicked();
  void onAnimationTick();
  void onPauseClicked();
  void onSpeedChanged(int value);

private:
  std::unique_ptr<UndirectedGraph> m_graph;
  QTimer *m_timer;

  QLabel *m_statusLabel;
  QLabel *m_timerLabel;
  QLabel *m_speedLabel;
  QPushButton *m_pauseBtn;
  QSlider *m_speedSlider;
  QWidget *m_toolbar;

  enum class AlgoType { None, DFS, BFS };
  AlgoType m_currentAlgo;
  std::vector<SpanningTreeStep> m_steps;
  int m_stepIdx;
  bool m_animDone;
  bool m_paused;
  std::unordered_set<int> m_visitedNodes;
  std::unordered_set<int> m_completedNodes;
  std::set<std::pair<int, int>> m_treeEdges;
  std::set<std::pair<int, int>> m_backEdges;
  std::set<std::pair<int, int>> m_crossEdges;
  int m_currentNode;
  std::pair<int, int> m_currentEdge;
  SpanningTreeStep::Action m_currentAction;
  int m_rootNode;

  qint64 m_elapsedMs;
  QElapsedTimer m_elapsedClock;

  void buildGraph();
  void startAnimation(AlgoType algo);
  void updateLabel();
  void updateTimerLabel();
  int currentInterval() const;
};

#endif // VIS_SPANNING_TREE_H
