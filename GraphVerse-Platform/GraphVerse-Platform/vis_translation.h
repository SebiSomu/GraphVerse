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

struct TranslationEdge {
  std::string u, v;
  int cost;
  bool operator<(const TranslationEdge &o) const { return cost < o.cost; }
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
  void addPair(const std::string &a, const std::string &b, int cost);
  std::vector<TranslationEdge> kruskalMST() const;

  // Disjoint Set utility for Kruskal
  struct DSU {
    std::unordered_map<std::string, std::string> parent;
    std::unordered_map<std::string, int> rank;
    void makeSet(const std::string &s) {
      parent[s] = s;
      rank[s] = 0;
    }
    std::string findSet(const std::string &s) {
      if (parent[s] == s)
        return s;
      return parent[s] = findSet(parent[s]);
    }
    void unionSet(const std::string &a, const std::string &b) {
      std::string pa = findSet(a), pb = findSet(b);
      if (pa != pb) {
        if (rank[pa] < rank[pb])
          std::swap(pa, pb);
        parent[pb] = pa;
        if (rank[pa] == rank[pb])
          rank[pa]++;
      }
    }
  };

  // Data
  std::vector<TranslationEdge> m_edges;
  std::vector<std::string> m_allLanguages;

  // UI Elements
  QLineEdit *m_searchInput;
  QPushButton *m_buildBtn;
  QWidget *m_resultsContainer;
  QVBoxLayout *m_resultsLayout;
  QScrollArea *m_scrollArea;
};

#endif // VIS_TRANSLATION_H
