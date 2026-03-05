#include "vis_translation.h"
#include <QDir>
#include <QFile>
#include <QLabel>
#include <algorithm>
#include <queue>
#include <unordered_set>


class TranslationNodeItem : public QWidget {
public:
  TranslationNodeItem(const QString &lang, int costFromParent, int depth,
                      QWidget *parent = nullptr)
      : QWidget(parent) {
    auto *lay = new QHBoxLayout(this);

    // Depth indent + standard margin
    int indent = depth * 32 + 12;

    // If not root, maybe add an elbow connector visually
    if (depth > 0) {
      auto *elbow = new QLabel("↳");
      elbow->setStyleSheet(
          QString(
              "font: 400 16px 'Segoe UI'; color: #8C94BE; margin-left: %1px;")
              .arg(indent - 20));
      lay->addWidget(elbow);
      lay->setContentsMargins(4, 4, 12, 4);
    } else {
      lay->setContentsMargins(indent, 4, 12, 4);
    }

    QString flagPath = QDir::currentPath() + "/flags/" + lang + ".png";
    auto *flagLbl = new QLabel;
    if (QFile::exists(flagPath)) {
      QPixmap pix(flagPath);
      flagLbl->setPixmap(
          pix.scaled(28, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
      flagLbl->setText("🌐"); // fallback
      flagLbl->setStyleSheet("font-size: 16px; color: #8C94BE;");
    }
    flagLbl->setFixedSize(30, 22);

    auto *nameLbl = new QLabel(lang);
    nameLbl->setStyleSheet("font: 600 14px 'Segoe UI'; color: #EBEDFF;");

    lay->addWidget(flagLbl);
    lay->addSpacing(8);
    lay->addWidget(nameLbl);

    if (costFromParent > 0) {
      auto *costLbl = new QLabel(QString("Difficulty: %1").arg(costFromParent));
      costLbl->setStyleSheet(
          "font: 600 11px 'Segoe UI'; color: #F59E0B; margin-left: 12px; "
          "padding: 2px 8px; background: rgba(245, 158, 11, 0.15); border: 1px "
          "solid rgba(245, 158, 11, 0.4); border-radius: 4px;");
      lay->addWidget(costLbl);
    }

    lay->addStretch();
  }
};

VisTranslation::VisTranslation(QWidget *parent) : QWidget(parent) {
  setupUi();
  buildNetwork();
}

VisTranslation::~VisTranslation() {}

void VisTranslation::setupUi() {
  auto *mainLay = new QVBoxLayout(this);
  mainLay->setContentsMargins(48, 24, 48, 48);
  mainLay->setSpacing(24);

  // Header Area
  auto *header = new QWidget;
  auto *hlay = new QHBoxLayout(header);
  hlay->setContentsMargins(0, 0, 0, 0);

  auto *title = new QLabel("Translation MST Network");
  title->setStyleSheet("font: 700 24px 'Segoe UI'; color: #14B8A6;");

  m_searchInput = new QLineEdit;
  m_searchInput->setPlaceholderText(
      "Enter Root Language (e.g. Spanish, French, English)...");
  m_searchInput->setMinimumHeight(44);
  m_searchInput->setMinimumWidth(320);
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

  m_buildBtn = new QPushButton("Build Translation Tree");
  m_buildBtn->setMinimumHeight(44);
  m_buildBtn->setStyleSheet(R"(
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
  m_buildBtn->setCursor(Qt::PointingHandCursor);

  connect(m_buildBtn, &QPushButton::clicked, this,
          &VisTranslation::onBuildTreeClicked);
  connect(m_searchInput, &QLineEdit::returnPressed, this,
          &VisTranslation::onBuildTreeClicked);

  hlay->addWidget(title);
  hlay->addStretch();
  hlay->addWidget(m_searchInput);
  hlay->addWidget(m_buildBtn);
  mainLay->addWidget(header);

  // Results Scroll Area
  m_scrollArea = new QScrollArea;
  m_scrollArea->setWidgetResizable(true);
  m_scrollArea->setStyleSheet("background: transparent; border: none;");

  m_resultsContainer = new QWidget;
  m_resultsContainer->setStyleSheet("background: transparent;");
  m_resultsLayout = new QVBoxLayout(m_resultsContainer);
  m_resultsLayout->setContentsMargins(0, 0, 0, 0);
  m_resultsLayout->setSpacing(8);
  m_resultsLayout->addStretch();

  m_scrollArea->setWidget(m_resultsContainer);
  mainLay->addWidget(m_scrollArea, 1);
}

void VisTranslation::addPair(const std::string &langs1,
                             const std::string &langs2, int cost) {
  m_edges.push_back({langs1, langs2, cost});
  if (std::find(m_allLanguages.begin(), m_allLanguages.end(), langs1) ==
      m_allLanguages.end())
    m_allLanguages.push_back(langs1);
  if (std::find(m_allLanguages.begin(), m_allLanguages.end(), langs2) ==
      m_allLanguages.end())
    m_allLanguages.push_back(langs2);
}

void VisTranslation::buildNetwork() {
  std::vector<std::tuple<std::string, std::string, int>> connections = {
      {"Spanish", "French", 2},       {"Spanish", "Portuguese", 1},
      {"Spanish", "Italian", 2},      {"Spanish", "Romanian", 2},
      {"Spanish", "Catalan", 2},      {"Spanish", "Galician", 1},
      {"French", "Portuguese", 2},    {"French", "Italian", 1},
      {"French", "Romanian", 2},      {"French", "Catalan", 2},
      {"French", "Occitan", 1},       {"Portuguese", "Italian", 2},
      {"Portuguese", "Romanian", 2},  {"Portuguese", "Catalan", 1},
      {"Italian", "Romanian", 1},     {"Italian", "Catalan", 2},
      {"Italian", "Occitan", 2},      {"Catalan", "Galician", 1},
      {"English", "German", 3},       {"English", "Dutch", 3},
      {"English", "Swedish", 3},      {"English", "Danish", 3},
      {"English", "Norwegian", 3},    {"English", "Afrikaans", 3},
      {"German", "Dutch", 2},         {"German", "Swedish", 3},
      {"German", "Danish", 3},        {"German", "Norwegian", 3},
      {"Swedish", "Danish", 2},       {"Swedish", "Norwegian", 2},
      {"Norwegian", "Danish", 1},     {"Danish", "Afrikaans", 3},
      {"Dutch", "Afrikaans", 3},      {"Russian", "Ukrainian", 2},
      {"Russian", "Polish", 3},       {"Russian", "Czech", 3},
      {"Russian", "Bulgarian", 3},    {"Russian", "Serbian", 2},
      {"Ukrainian", "Polish", 3},     {"Ukrainian", "Bulgarian", 2},
      {"Ukrainian", "Belarusian", 2}, {"Polish", "Czech", 2},
      {"Polish", "Slovak", 2},        {"Polish", "Croatian", 3},
      {"Czech", "Slovak", 1},         {"Czech", "Serbian", 3},
      {"Bulgarian", "Serbian", 2},    {"Serbian", "Croatian", 1},
      {"Serbian", "Slovenian", 2},    {"Croatian", "Slovenian", 2},
      {"Belarusian", "Polish", 3},    {"Mandarin", "Cantonese", 4},
      {"Mandarin", "Wu", 5},          {"Mandarin", "Burmese", 6},
      {"Mandarin", "Tibetan", 5},     {"Cantonese", "Wu", 4},
      {"Hindi", "Urdu", 1},           {"Hindi", "Bengali", 4},
      {"Hindi", "Punjabi", 3},        {"Hindi", "Marathi", 3},
      {"Urdu", "Punjabi", 2},         {"Urdu", "Persian", 3},
      {"Bengali", "Punjabi", 3},      {"Persian", "Kurdish", 3},
      {"Persian", "Pashto", 4},       {"Kurdish", "Pashto", 3},
      {"Finnish", "Estonian", 5},     {"Finnish", "Hungarian", 6},
      {"Estonian", "Hungarian", 6},   {"Arabic", "Hebrew", 4},
      {"Arabic", "Amharic", 5},       {"Arabic", "Tigrinya", 4},
      {"Arabic", "Maltese", 4},       {"Hebrew", "Amharic", 5},
      {"Hebrew", "Tigrinya", 5},      {"Japanese", "Korean", 8},
      {"Tamil", "Telugu", 3},         {"Tamil", "Kannada", 3},
      {"Tamil", "Malayalam", 3},      {"Telugu", "Kannada", 2},
      {"Telugu", "Malayalam", 3},     {"Swahili", "Hausa", 6},
      {"Swahili", "Yoruba", 6},       {"Swahili", "Zulu", 6},
      {"Swahili", "Igbo", 7},         {"Hausa", "Yoruba", 5},
      {"Hausa", "Fula", 5},           {"Yoruba", "Igbo", 6},
      {"Zulu", "Shona", 5},           {"Indonesian", "Malay", 1},
      {"Indonesian", "Tagalog", 3},   {"Indonesian", "Javanese", 2},
      {"Malay", "Tagalog", 3},        {"Vietnamese", "Thai", 6},
      {"Turkish", "Azerbaijani", 2},  {"Turkish", "Uzbek", 3},
      {"Turkish", "Kazakh", 3},       {"Azerbaijani", "Uzbek", 2},
      {"Uzbek", "Kazakh", 2},         {"Greek", "Armenian", 7},
      {"Greek", "Georgian", 8},       {"Greek", "Albanian", 7},
      {"Armenian", "Georgian", 6},    {"Georgian", "Albanian", 7},
      {"Basque", "Spanish", 8},       {"English", "Spanish", 4},
      {"English", "French", 3},       {"English", "Portuguese", 4},
      {"English", "Italian", 3},      {"English", "Romanian", 5},
      {"English", "Russian", 6},      {"English", "Ukrainian", 6},
      {"English", "Polish", 5},       {"English", "Czech", 5},
      {"English", "Bulgarian", 6},    {"English", "Serbian", 6},
      {"English", "Arabic", 7},       {"English", "Hebrew", 6},
      {"English", "Hindi", 5},        {"English", "Bengali", 6},
      {"English", "Mandarin", 8},     {"English", "Japanese", 9},
      {"English", "Korean", 8},       {"English", "Turkish", 6},
      {"English", "Swahili", 5},      {"English", "Indonesian", 6},
      {"English", "Vietnamese", 7},   {"English", "Thai", 8},
      {"English", "Finnish", 7},      {"English", "Hungarian", 8},
      {"English", "Basque", 10},      {"Mandarin", "Japanese", 8},
      {"Mandarin", "Korean", 7},      {"Mandarin", "Hindi", 8},
      {"Mandarin", "Bengali", 9},     {"Mandarin", "Russian", 8},
      {"Mandarin", "Vietnamese", 6},  {"Mandarin", "Thai", 7},
      {"French", "Swahili", 6},       {"French", "Hausa", 5},
      {"French", "Yoruba", 6},        {"French", "Fula", 5},
      {"Arabic", "Turkish", 6},       {"Arabic", "Persian", 5},
      {"Arabic", "Urdu", 6},          {"Arabic", "Swahili", 6},
      {"Arabic", "Hausa", 5},         {"Arabic", "Yoruba", 7},
      {"Hindi", "Tamil", 6},          {"Hindi", "Telugu", 5},
      {"Hindi", "Kannada", 5},        {"Russian", "Finnish", 7},
      {"Russian", "Estonian", 7},     {"Russian", "Hungarian", 8},
      {"Russian", "Turkish", 6},      {"Russian", "Azerbaijani", 6},
      {"Russian", "Kazakh", 6}};

  for (const auto &[lang1, lang2, cost] : connections) {
    addPair(lang1, lang2, cost);
  }
}

std::vector<TranslationEdge> VisTranslation::kruskalMST() const {
  std::vector<TranslationEdge> mst;
  std::vector<TranslationEdge> sortedEdges = m_edges;
  std::sort(sortedEdges.begin(), sortedEdges.end());

  DSU dsu;
  for (const auto &l : m_allLanguages)
    dsu.makeSet(l);

  for (const auto &e : sortedEdges) {
    if (dsu.findSet(e.u) != dsu.findSet(e.v)) {
      dsu.unionSet(e.u, e.v);
      mst.push_back(e);
    }
  }
  return mst;
}

void VisTranslation::onBuildTreeClicked() {
  QString qroot = m_searchInput->text().trimmed();
  if (qroot.isEmpty())
    return;

  std::string root = qroot.toStdString();

  QLayoutItem *child;
  while ((child = m_resultsLayout->takeAt(0)) != nullptr) {
    if (child->widget())
      child->widget()->deleteLater();
    delete child;
  }

  if (std::find(m_allLanguages.begin(), m_allLanguages.end(), root) ==
      m_allLanguages.end()) {
    auto *err = new QLabel(QString("Language '%1' not found in the network. "
                                   "Try 'Spanish', 'French', etc.")
                               .arg(qroot));
    err->setStyleSheet("color: #F87171; font-size: 14px;");
    m_resultsLayout->addWidget(err);
    m_resultsLayout->addStretch();
    return;
  }

  auto mst = kruskalMST();

  // Build Adjacency List from MST
  std::unordered_map<std::string, std::vector<std::pair<std::string, int>>> adj;
  int totalCost = 0;
  for (const auto &e : mst) {
    adj[e.u].push_back({e.v, e.cost});
    adj[e.v].push_back({e.u, e.cost});
    totalCost += e.cost;
  }

  auto *statLbl = new QLabel(
      QString("MST Total Difficulty: %1 (Min Effort Required)").arg(totalCost));
  statLbl->setStyleSheet("font: 700 16px 'Segoe UI'; color: #8C94BE; "
                         "margin-bottom: 16px; margin-top: 10px;");
  m_resultsLayout->addWidget(statLbl);

  // BFS to render Tree Hierarchy
  std::unordered_set<std::string> visited;

  // Instead of simple Queue, we can use a Queue to layout DFS-like or BFS-like
  // Wait, DFS visually makes more hierarchical sense for a Tree list.
  // Let's do an explicit iterative DFS tree print so children appear exactly
  // under parents.

  std::vector<std::tuple<std::string, int, int>>
      stackList; // lang, depth, costToParent
  auto dfs = [&](auto &self, const std::string &curr, int depth,
                 int costPr) -> void {
    visited.insert(curr);
    stackList.push_back({curr, depth, costPr});

    // Sort children for deterministic beautiful layout
    auto children = adj[curr];
    std::sort(children.begin(), children.end(),
              [](const auto &a, const auto &b) {
                if (a.second != b.second)
                  return a.second < b.second;
                return a.first < b.first;
              });

    for (const auto &[nxt, cost] : children) {
      if (!visited.count(nxt)) {
        self(self, nxt, depth + 1, cost);
      }
    }
  };

  dfs(dfs, root, 0, 0);

  // Build the visual lines
  for (const auto &[lang, depth, cost] : stackList) {
    auto *item =
        new TranslationNodeItem(QString::fromStdString(lang), cost, depth);
    m_resultsLayout->addWidget(item);
  }

  m_resultsLayout->addStretch();
}
