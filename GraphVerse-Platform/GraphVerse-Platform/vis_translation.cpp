#include "vis_translation.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
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
  QString jsonPath = QDir::currentPath() + "/translations.json";
  std::ifstream file(jsonPath.toStdString());

  if (!file.is_open()) {
    addPair("English", "Spanish", 5);
    return;
  }

  // Read whole file into string for easy multiline parsing
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());

  size_t pos = 0;
  while (true) {
    size_t l1_label = content.find("\"lang1\":", pos);
    if (l1_label == std::string::npos)
      break;

    size_t l1_start = content.find("\"", l1_label + 8) + 1;
    size_t l1_end = content.find("\"", l1_start);
    std::string lang1 = content.substr(l1_start, l1_end - l1_start);

    size_t l2_label = content.find("\"lang2\":", l1_end);
    if (l2_label == std::string::npos)
      break;

    size_t l2_start = content.find("\"", l2_label + 8) + 1;
    size_t l2_end = content.find("\"", l2_start);
    std::string lang2 = content.substr(l2_start, l2_end - l2_start);

    size_t cost_label = content.find("\"cost\":", l2_end);
    if (cost_label == std::string::npos)
      break;

    size_t cost_start = content.find_first_of("0123456789", cost_label + 7);
    size_t cost_end = content.find_first_not_of("0123456789", cost_start);
    int cost = std::stoi(content.substr(cost_start, cost_end - cost_start));

    addPair(lang1, lang2, cost);
    pos = cost_end;
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

  // Clear previous results
  QLayoutItem *child;
  while ((child = m_resultsLayout->takeAt(0)) != nullptr) {
    if (child->widget())
      child->widget()->deleteLater();
    delete child;
  }

  // Show Loading message
  auto *loadingLbl = new QLabel("Building Translation Tree... Please wait.");
  loadingLbl->setStyleSheet(
      "font: 600 16px 'Segoe UI'; color: #14B8A6; padding: 20px;");
  m_resultsLayout->addWidget(loadingLbl);
  m_resultsLayout->addStretch();

  // Refresh UI to show the label immediately
  QCoreApplication::processEvents();

  if (std::find(m_allLanguages.begin(), m_allLanguages.end(), root) ==
      m_allLanguages.end()) {
    // Cleanup loading if error
    m_resultsLayout->removeWidget(loadingLbl);
    loadingLbl->deleteLater();

    auto *err = new QLabel(QString("Language '%1' not found in the network. "
                                   "Try 'Spanish', 'French', etc.")
                               .arg(qroot));
    err->setStyleSheet("color: #F87171; font-size: 14px;");
    m_resultsLayout->addWidget(err);
    m_resultsLayout->addStretch();
    return;
  }

  auto mst = kruskalMST();

  // Cleanup loading before building the tree results
  m_resultsLayout->removeWidget(loadingLbl);
  loadingLbl->deleteLater();

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
