#ifndef VIS_TRANSLATION_H
#define VIS_TRANSLATION_H

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <string>
#include <unordered_map>
#include <vector>
#include <QString>
#include "undirectedgraph.h"
#include "algorithms/kruskal_solver.h"

struct TranslationEdge {
  QString u, v;
  int cost;
  bool operator<(const TranslationEdge &o) const { return cost < o.cost; }
  bool operator==(const TranslationEdge &o) const { return cost == o.cost && u == o.u && v == o.v; }
};

class VisTranslation : public QWidget {
  Q_OBJECT
public:
  explicit VisTranslation(QWidget *parent = nullptr);
  ~VisTranslation();

private slots:
  void onBuildTreeClicked();

private:
  void setupUi();
  void buildNetwork();
  void addPair(const QString &a, const QString &b, int cost);
  std::vector<TranslationEdge> kruskalMST() const;

  // Data
  UndirectedGraph m_graph;
  std::unordered_map<QString, int> m_nameToIndex;

  // UI Elements
  QLineEdit *m_searchInput;
  QPushButton *m_buildBtn;
  QWidget *m_resultsContainer;
  QVBoxLayout *m_resultsLayout;
  QScrollArea *m_scrollArea;
};

#endif // VIS_TRANSLATION_H
