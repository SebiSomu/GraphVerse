#ifndef VIS_FRIENDS_H
#define VIS_FRIENDS_H

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
#include "algorithms/bfs_traversal.h"


struct Suggestion {
  QString name;
  int distance;
  int mutualCount;
  bool operator==(const Suggestion &o) const { return distance == o.distance && mutualCount == o.mutualCount && name == o.name; }
};

class VisFriends : public QWidget {
  Q_OBJECT
public:
  explicit VisFriends(QWidget *parent = nullptr);
  ~VisFriends();

private slots:
  void onSearchClicked();

private:
  void setupUi();
  void buildNetwork();
  void addFriendship(Node &a, Node &b);
  std::vector<Suggestion> getSuggestions(int rootNodeIdx) const;
  int countMutual(int nodeIdxA, int nodeIdxB) const;

  // Data
  UndirectedGraph m_graph;
  std::unordered_map<QString, int> m_nameToIndex;

  // UI Elements
  QLineEdit *m_searchInput;
  QPushButton *m_searchBtn;
  QWidget *m_resultsContainer;
  QVBoxLayout *m_resultsLayout;
  QScrollArea *m_scrollArea;
};

#endif // VIS_FRIENDS_H
