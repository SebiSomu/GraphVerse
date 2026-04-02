#include "vis_friends.h"
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <algorithm>
#include <queue>
#include <unordered_set>
#include <ranges>
#include "algorithms/bfs_traversal.h"
#include "algorithms/graph_utils.h"


namespace PaletteApp {
const QColor BG_CARD{18, 22, 42};
const QColor ACCENT{20, 184, 166};
const QColor TEXT_PRI{235, 237, 255};
const QColor TEXT_SEC{140, 148, 190};
} // namespace PaletteApp

// Custom UI Card for a Person
class PersonCard : public QFrame {
public:
  PersonCard(const QString &name, const QString &details, bool isSuggested,
             QWidget *parent = nullptr)
      : QFrame(parent) {
    setFixedSize(160, 80);
    setStyleSheet(QString("background: rgba(18,22,42,0.8); border: 1px solid "
                          "rgba(20,184,166,%1); border-radius: 8px;")
                      .arg(isSuggested ? "0.6" : "0.2"));

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(12, 12, 12, 12);

    auto *nameLbl = new QLabel(name);
    nameLbl->setStyleSheet("font: 600 13px 'Segoe UI'; color: #EBEDFF; border: "
                           "none; background: transparent;");

    auto *detLbl = new QLabel(details);
    detLbl->setStyleSheet("font: 400 11px 'Segoe UI'; color: #8C94BE; border: "
                          "none; background: transparent;");

    lay->addWidget(nameLbl);
    lay->addWidget(detLbl);
    lay->addStretch();
  }
};

VisFriends::VisFriends(QWidget *parent) : QWidget(parent) {
  setupUi();
  buildNetwork();
}

VisFriends::~VisFriends() = default;

void VisFriends::hideEvent(QHideEvent* event) {
    m_searchInput->clear();
    QLayoutItem *child;
    while ((child = m_resultsLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    QWidget::hideEvent(event);
}

void VisFriends::setupUi() {
  auto *mainLay = new QVBoxLayout(this);
  mainLay->setContentsMargins(48, 24, 48, 48);
  mainLay->setSpacing(24);

  // Header Area
  auto *header = new QWidget;
  auto *hlay = new QHBoxLayout(header);
  hlay->setContentsMargins(0, 0, 0, 0);

  auto *title = new QLabel("Friend Suggestion System");
  title->setStyleSheet("font: 700 24px 'Segoe UI'; color: #14B8A6;");

  m_searchInput = new QLineEdit;
  m_searchInput->setPlaceholderText(
      "Enter your name (e.g. John, Elena, Sebi, Oliver)...");
  m_searchInput->setMinimumHeight(44);
  m_searchInput->setMinimumWidth(300);
  m_searchInput->setStyleSheet(R"(
        QLineEdit {
            background: rgba(14,17,32,0.8);
            border: 1px solid rgba(140,148,190,0.3);
            border-radius: 6px;
            color: #EBEDFF;
            padding: 0 16px;
            font-size: 14px;
        }
        QLineEdit:focus { border: 1px solid #14B8A6; }
    )");

  m_searchBtn = new QPushButton("Suggest Friends");
  m_searchBtn->setMinimumHeight(44);
  m_searchBtn->setStyleSheet(R"(
        QPushButton {
            background: rgba(20,184,166,0.15);
            border: 1px solid rgba(20,184,166,0.5);
            border-radius: 6px;
            color: #EBEDFF;
            font: 600 13px 'Segoe UI';
            padding: 0 20px;
        }
        QPushButton:hover { background: rgba(20,184,166,0.25); }
    )");
  m_searchBtn->setCursor(Qt::PointingHandCursor);

  connect(m_searchBtn, &QPushButton::clicked, this,
          &VisFriends::onSearchClicked);
  connect(m_searchInput, &QLineEdit::returnPressed, this,
          &VisFriends::onSearchClicked);

  hlay->addWidget(title);
  hlay->addStretch();
  hlay->addWidget(m_searchInput);
  hlay->addWidget(m_searchBtn);
  mainLay->addWidget(header);

  // Results Scroll Area
  m_scrollArea = new QScrollArea;
  m_scrollArea->setWidgetResizable(true);
  m_scrollArea->setStyleSheet("background: transparent; border: none;");

  m_resultsContainer = new QWidget;
  m_resultsContainer->setStyleSheet("background: transparent;");
  m_resultsLayout = new QVBoxLayout(m_resultsContainer);
  m_resultsLayout->setContentsMargins(0, 0, 0, 0);
  m_resultsLayout->setSpacing(32);
  m_resultsLayout->addStretch();

  m_scrollArea->setWidget(m_resultsContainer);
  mainLay->addWidget(m_scrollArea, 1);
}

void VisFriends::addFriendship(Node &a, Node &b) {
  m_graph.addEdge(a, b);
}

void VisFriends::buildNetwork() {
  std::vector<QString> names = {
      "John",      "Oliver",     "Harry",     "Jack",    "Jacob",
      "Noah",      "Charlie",    "Thomas",    "George",  "Oscar",
      "Emma",      "Olivia",     "Amelia",    "Isla",    "Ava",
      "Mia",       "Isabella",   "Sophia",    "Grace",   "Lily",
      "Sebi",      "Andrei",     "Mihai",     "Elena",   "Radu",
      "Bogdan",    "Ioana",      "Alexandra", "Vlad",    "Teodora",
      "Marius",    "Cristina",   "Florin",    "Catalin", "Razvan",
      "Oana",      "Petra",      "Silviu",    "Dragos",  "Nicoleta",
      "Sorin",     "Cezar",      "Lucian",    "Matei",   "Diana",
      "Bianca",    "Victor",     "Stefan",    "Aisha",   "Fatima",
      "Mohammed",  "Ali",        "Omar",      "Zainab",  "Hassan",
      "Youssef",   "Mariam",     "Tariq",     "Huda",    "Nour",
      "Wei",       "Li",         "Wang",      "Zhang",   "Chen",
      "Yang",      "Zhao",       "Huang",     "Zhou",    "Wu",
      "Hiroshi",   "Kenji",      "Takumi",    "Sota",    "Yuki",
      "Sakura",    "Hina",       "Yui",       "Mei",     "Rin",
      "Arthur",    "Louis",      "Jules",     "Leo",     "Gabriel",
      "Alice",     "Chloe",      "Ines",      "Lea",     "Emma_FR",
      "Alejandro", "Daniel",     "Pablo",     "David",   "Adrian",
      "Lucia",     "Maria",      "Paula",     "Laura",   "Carmen",
      "Matteo",    "Alessandro", "Leonardo",  "Lorenzo", "Mattia",
      "Sofia",     "Giulia",     "Aurora",    "Ginevra", "Alice_IT"};

  m_graph.clear();
  m_nameToIndex.clear();

  std::vector<Node*> nodes;
  for (const auto &n : names) {
    m_graph.addNode(QPoint(0, 0));
    Node& node = m_graph.getNodes().back();
    node.setName(n);
    m_nameToIndex[n] = node.getIndex();
    nodes.push_back(&node);
  }

  auto makeClique = [&](int start, int end) {
    for (int i = start; i < end; i++)
      for (int j = i + 1; j < end; j++)
        addFriendship(*nodes[i], *nodes[j]);
  };

  makeClique(0, 5);  
  makeClique(5, 10); 
  addFriendship(*nodes[0], *nodes[10]); // John, Emma
  addFriendship(*nodes[1], *nodes[11]); // Oliver, Olivia
  addFriendship(*nodes[9], *nodes[19]); // Oscar, Lily

  // Romanian network
  makeClique(20, 26);
  makeClique(26, 32);
  addFriendship(*nodes[20], *nodes[41]); // Sebi, Cezar
  addFriendship(*nodes[20], *nodes[21]); // Sebi, Andrei
  addFriendship(*nodes[21], *nodes[22]); // Andrei, Mihai
  addFriendship(*nodes[21], *nodes[23]); 
  addFriendship(*nodes[21], *nodes[24]); 
  addFriendship(*nodes[22], *nodes[23]); 
  addFriendship(*nodes[22], *nodes[25]); 
  addFriendship(*nodes[23], *nodes[26]); 
  addFriendship(*nodes[26], *nodes[27]); 
  addFriendship(*nodes[26], *nodes[28]); 
  addFriendship(*nodes[28], *nodes[30]); // Vlad, Marius
  addFriendship(*nodes[29], *nodes[30]); // Teodora, Marius
  addFriendship(*nodes[31], *nodes[29]); // Cristina, Teodora
  addFriendship(*nodes[25], *nodes[28]); // Bogdan, Vlad
  addFriendship(*nodes[32], *nodes[38]); // Florin, Dragos

  // Random links
  srand(42);
  for (int i = 0; i < 60; ++i) {
    int u = rand() % names.size();
    int v = rand() % names.size();
    if (u != v)
      addFriendship(*nodes[u], *nodes[v]);
  }
}

int VisFriends::countMutual(int nodeIdxA, int nodeIdxB) const {
  std::unordered_set<int> friendsA;
  for (const auto &e : m_graph.getEdges()) {
    if (e.getFirst().getIndex() == nodeIdxA) friendsA.insert(e.getSecond().getIndex());
    else if (e.getSecond().getIndex() == nodeIdxA) friendsA.insert(e.getFirst().getIndex());
  }

  int mutual = 0;
  for (const auto &e : m_graph.getEdges()) {
    int other = -1;
    if (e.getFirst().getIndex() == nodeIdxB) other = e.getSecond().getIndex();
    else if (e.getSecond().getIndex() == nodeIdxB) other = e.getFirst().getIndex();
    
    if (other != -1 && friendsA.contains(other)) mutual++;
  }
  return mutual;
}

std::vector<Suggestion> VisFriends::getSuggestions(int rootNodeIdx) const {
  BFSTraversal bfs;
  auto steps = bfs.solve(m_graph, rootNodeIdx);

  std::vector<Suggestion> results;
  auto nodes = m_graph.getNodes();

  for (const auto& step : steps) {
    if (step.distance == 2) {
      Suggestion s;
      s.distance = 2;
      s.mutualCount = countMutual(rootNodeIdx, step.nodeIndex);
      
      // Find name
      for(const auto& n : nodes) {
          if(n.getIndex() == step.nodeIndex) {
              s.name = n.getName();
              break;
          }
      }
      results.push_back(s);
    }
  }

  std::ranges::sort(results, [](const Suggestion &a, const Suggestion &b) {
    return a.mutualCount > b.mutualCount;
  });

  return results;
}

void VisFriends::onSearchClicked() {
  QString name = m_searchInput->text().trimmed();
  if (name.isEmpty())
    return;

  // Clear old layout
  QLayoutItem *child;
  while ((child = m_resultsLayout->takeAt(0)) != nullptr) {
    if (child->widget())
      child->widget()->deleteLater();
    delete child;
  }

  if (!m_nameToIndex.contains(name)) {
    auto *err = new QLabel(QString("User '%1' not found in the network. Try "
                                   "'Sebi', 'John', 'Li', etc.")
                                .arg(name));
    err->setStyleSheet("color: #F87171; font-size: 14px;");
    m_resultsLayout->addWidget(err);
    m_resultsLayout->addStretch();
    return;
  }

  int rootIdx = m_nameToIndex[name];
  
  // Direct Friends
  std::vector<QString> friends;
  for(const auto& e : m_graph.getEdges()) {
      if(e.getFirst().getIndex() == rootIdx) friends.push_back(e.getSecond().getName());
      else if(e.getSecond().getIndex() == rootIdx) friends.push_back(e.getFirst().getName());
  }
  std::ranges::sort(friends);

  auto *lblDirect = new QLabel(
      QString("Current Network (%1 direct connections)").arg(friends.size()));
  lblDirect->setStyleSheet(
      "font: 700 16px 'Segoe UI'; color: #8C94BE; margin-top: 10px;");
  m_resultsLayout->addWidget(lblDirect);

  auto *gridFriends = new QGridLayout;
  gridFriends->setSpacing(12);
  int row = 0, col = 0;
  for (size_t i = 0; i < std::min(friends.size(), static_cast<size_t>(24));
       ++i) { 
    auto *card = new PersonCard(friends[i], "1st Degree", false);
    gridFriends->addWidget(card, row, col);
    if (++col >= 5) {
      col = 0;
      row++;
    }
  }
  auto *w1 = new QWidget;
  w1->setLayout(gridFriends);
  m_resultsLayout->addWidget(w1);

  // Suggestions
  auto suggestions = getSuggestions(rootIdx);

  auto *lblSugg = new QLabel(
      QString("Suggested Friends (%1 found)").arg(suggestions.size()));
  lblSugg->setStyleSheet(
      "font: 700 16px 'Segoe UI'; color: #14B8A6; margin-top: 24px;");
  m_resultsLayout->addWidget(lblSugg);

  auto *gridSugg = new QGridLayout;
  gridSugg->setSpacing(12);
  row = 0;
  col = 0;
  for (size_t i = 0; i < std::min(suggestions.size(), static_cast<size_t>(40));
       ++i) { 
    const auto &s = suggestions[i];
    QString det = QString("%1 Mutual Friend%2")
                      .arg(s.mutualCount)
                      .arg(s.mutualCount == 1 ? "" : "s");
    auto *card = new PersonCard(s.name, det, true);
    gridSugg->addWidget(card, row, col);
    if (++col >= 5) {
      col = 0;
      row++;
    }
  }
  auto *w2 = new QWidget;
  w2->setLayout(gridSugg);
  m_resultsLayout->addWidget(w2);

  m_resultsLayout->addStretch();
}
