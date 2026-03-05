#include "vis_friends.h"
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <algorithm>
#include <queue>
#include <unordered_set>


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

VisFriends::~VisFriends() {}

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

void VisFriends::addFriendship(const std::string &a, const std::string &b) {
  m_adj[a].push_back(b);
  m_adj[b].push_back(a);
}

void VisFriends::buildNetwork() {
  // Generate an enormous list of international friend connections
  // Over 100 names, numerous edges
  std::vector<std::string> names = {
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

  // Ensure all names exist in map at least
  for (const auto &n : names)
    m_adj[n] = {};

  // Group formations
  auto makeClique = [&](int start, int end) {
    for (int i = start; i < end; i++)
      for (int j = i + 1; j < end; j++)
        addFriendship(names[i], names[j]);
  };

  makeClique(0, 5);  // English bros
  makeClique(5, 10); // English sis
  addFriendship("John", "Emma");
  addFriendship("Oliver", "Olivia");
  addFriendship("Oscar", "Lily");

  // Romanian network
  makeClique(20, 26);
  makeClique(26, 32);
  addFriendship("Sebi", "Cezar");
  addFriendship("Sebi", "Andrei");
  addFriendship("Andrei", "Mihai");
  addFriendship("Andrei", "Elena");
  addFriendship("Andrei", "Radu");
  addFriendship("Mihai", "Elena");
  addFriendship("Mihai", "Bogdan");
  addFriendship("Elena", "Ioana");
  addFriendship("Ioana", "Alexandra");
  addFriendship("Ioana", "Vlad");
  addFriendship("Vlad", "Marius");
  addFriendship("Teodora", "Marius");
  addFriendship("Cristina", "Teodora");
  addFriendship("Bogdan", "Vlad");
  addFriendship("Florin", "Dragos");

  // Global bridges
  addFriendship("Sebi", "John");
  addFriendship("Oliver", "Arthur");
  addFriendship("Harry", "Matteo");
  addFriendship("Mia", "Sofia");
  addFriendship("Sophia", "Ali");
  addFriendship("Omar", "Wei");
  addFriendship("Li", "Hiroshi");
  addFriendship("Yuki", "Alejandro");
  addFriendship("Daniel", "Alessandro");

  // Add random links
  srand(42);
  for (int i = 0; i < 60; ++i) {
    int u = rand() % names.size();
    int v = rand() % names.size();
    if (u != v)
      addFriendship(names[u], names[v]);
  }

  // Clean up duplicates
  for (auto &pair : m_adj) {
    std::sort(pair.second.begin(), pair.second.end());
    pair.second.erase(std::unique(pair.second.begin(), pair.second.end()),
                      pair.second.end());
  }
}

int VisFriends::countMutual(const std::string &a, const std::string &b) const {
  if (m_adj.find(a) == m_adj.end() || m_adj.find(b) == m_adj.end())
    return 0;

  std::unordered_set<std::string> friendsA(m_adj.at(a).begin(),
                                           m_adj.at(a).end());
  int mutual = 0;
  for (const auto &f : m_adj.at(b)) {
    if (friendsA.count(f))
      mutual++;
  }
  return mutual;
}

std::vector<Suggestion>
VisFriends::getSuggestions(const std::string &root) const {
  if (m_adj.find(root) == m_adj.end())
    return {};

  std::unordered_map<std::string, int> dist;
  std::queue<std::pair<std::string, int>> q;
  q.push({root, 0});
  dist[root] = 0;

  std::vector<Suggestion> results;

  while (!q.empty()) {
    auto [u, d] = q.front();
    q.pop();

    if (d > 0) {
      Suggestion s;
      s.name = u;
      s.distance = d;
      s.mutualCount = countMutual(root, u);
      if (d == 2)
        results.push_back(s); // Only suggest 2nd degree friends
    }

    if (d < 2) { // up to depth 2
      for (const auto &v : m_adj.at(u)) {
        if (dist.find(v) == dist.end()) {
          dist[v] = d + 1;
          q.push({v, d + 1});
        }
      }
    }
  }

  std::sort(results.begin(), results.end(),
            [](const Suggestion &a, const Suggestion &b) {
              if (a.distance != b.distance)
                return a.distance < b.distance;
              return a.mutualCount > b.mutualCount;
            });

  return results;
}

void VisFriends::onSearchClicked() {
  QString qname = m_searchInput->text().trimmed();
  if (qname.isEmpty())
    return;

  std::string name = qname.toStdString();

  // Clear old layout
  QLayoutItem *child;
  while ((child = m_resultsLayout->takeAt(0)) != nullptr) {
    if (child->widget())
      child->widget()->deleteLater();
    delete child;
  }

  if (m_adj.find(name) == m_adj.end()) {
    auto *err = new QLabel(QString("User '%1' not found in the network. Try "
                                   "'Sebi', 'John', 'Li', etc.")
                               .arg(qname));
    err->setStyleSheet("color: #F87171; font-size: 14px;");
    m_resultsLayout->addWidget(err);
    m_resultsLayout->addStretch();
    return;
  }

  // Direct Friends
  const auto &friends = m_adj[name];

  auto *lblDirect = new QLabel(
      QString("Current Network (%1 direct connections)").arg(friends.size()));
  lblDirect->setStyleSheet(
      "font: 700 16px 'Segoe UI'; color: #8C94BE; margin-top: 10px;");
  m_resultsLayout->addWidget(lblDirect);

  auto *gridFriends = new QGridLayout;
  gridFriends->setSpacing(12);
  int row = 0, col = 0;
  for (size_t i = 0; i < std::min(friends.size(), size_t(24));
       ++i) { // Show up to 24 direct
    auto *card =
        new PersonCard(QString::fromStdString(friends[i]), "1st Degree", false);
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
  auto suggestions = getSuggestions(name);

  auto *lblSugg = new QLabel(
      QString("Suggested Friends (%1 found)").arg(suggestions.size()));
  lblSugg->setStyleSheet(
      "font: 700 16px 'Segoe UI'; color: #14B8A6; margin-top: 24px;");
  m_resultsLayout->addWidget(lblSugg);

  auto *gridSugg = new QGridLayout;
  gridSugg->setSpacing(12);
  row = 0;
  col = 0;
  for (size_t i = 0; i < std::min(suggestions.size(), size_t(40));
       ++i) { // Up to 40 suggestions
    const auto &s = suggestions[i];
    QString det = QString("%1 Mutual Friend%2")
                      .arg(s.mutualCount)
                      .arg(s.mutualCount == 1 ? "" : "s");
    auto *card = new PersonCard(QString::fromStdString(s.name), det, true);
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
