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


struct Suggestion {
  std::string name;
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
  void addFriendship(const std::string &a, const std::string &b);
  std::vector<Suggestion> getSuggestions(const std::string &rootUser) const;
  int countMutual(const std::string &a, const std::string &b) const;

  // Data
  std::unordered_map<std::string, std::vector<std::string>> m_adj;

  // UI Elements
  QLineEdit *m_searchInput;
  QPushButton *m_searchBtn;
  QWidget *m_resultsContainer;
  QVBoxLayout *m_resultsLayout;
  QScrollArea *m_scrollArea;
};

#endif // VIS_FRIENDS_H
