#ifndef VIS_SUPERMARKET_H
#define VIS_SUPERMARKET_H

#include <QtCore/QPoint>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidget>
#include <unordered_map>
#include <vector>

class DirectedGraph;
class QPainter;
class QMouseEvent;
class QPaintEvent;

class VisSupermarket : public QWidget {
  Q_OBJECT
public:
  explicit VisSupermarket(QWidget *parent = nullptr);
  ~VisSupermarket();

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
  void onResetClicked();
  void onFindPathClicked();
  void onAddSectionClicked();
  void onRemoveSectionClicked();

private:
  enum class InteractionMode { Normal, Adding, Removing };

  void setupUi();
  void buildHipermarketLayout();
  int nodeAt(QPoint pos) const;
  void updateStatus(const QString &text);
  void updateGraphEdges();

  DirectedGraph *m_graph;
  std::unordered_map<int, QString> m_nodeNames;

  int m_startIdx;
  int m_endIdx;
  std::vector<int> m_finalPath;
  int m_pathCost;

  QLabel *m_statusLabel;
  QLineEdit *m_nameInput;
  InteractionMode m_mode;

  int m_draggedNodeIdx;
  bool m_isDragging;

  const int NODE_R = 25;
};

#endif // VIS_SUPERMARKET_H
