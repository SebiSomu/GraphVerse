#include "vis_supermarket.h"
#include "Node.h"
#include "directedgraph.h"
#include "edge.h"
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <vector>

VisSupermarket::VisSupermarket(QWidget *parent)
    : QWidget(parent), m_graph(nullptr), m_nodeNames(), m_startIdx(-1),
      m_endIdx(-1), m_finalPath(), m_pathCost(0),
      m_statusLabel(new QLabel(this)), m_nameInput(new QLineEdit(this)),
      m_mode(InteractionMode::Normal) {
  setupUi();
  buildHipermarketLayout();
}

VisSupermarket::~VisSupermarket() { delete m_graph; }

void VisSupermarket::setupUi() {
  auto *toolbar = new QWidget(this);
  auto *vlay = new QVBoxLayout(toolbar);
  vlay->setContentsMargins(24, 12, 24, 12);
  vlay->setSpacing(8);

  auto *topRow = new QHBoxLayout();
  m_nameInput->setPlaceholderText("Enter new Section Name");
  m_nameInput->setFixedWidth(200);
  m_nameInput->setStyleSheet(
      "QLineEdit { background: #1E293B; color: white; border: 1px solid "
      "#475569; border-radius: 6px; padding: 4px 8px; }");

  auto *btnAdd = new QPushButton("➕ Add Section", this);
  auto *btnRemove = new QPushButton("🗑 Remove", this);
  auto *btnFind = new QPushButton("🛒 Find Route", this);
  auto *btnReset = new QPushButton("🔄 Reset", this);

  QString btnStyle = "QPushButton { border-radius: 8px; font-weight: bold; "
                     "font-size: 13px; padding: 6px 12px; color: white; }";
  btnAdd->setStyleSheet(btnStyle +
                        "QPushButton { background: #10B981; } "
                        "QPushButton:hover { background: #059669; }");
  btnRemove->setStyleSheet(btnStyle +
                           "QPushButton { background: #EF4444; } "
                           "QPushButton:hover { background: #DC2626; }");
  btnFind->setStyleSheet(btnStyle +
                         "QPushButton { background: #6366F1; } "
                         "QPushButton:hover { background: #4F46E5; }");
  btnReset->setStyleSheet(btnStyle +
                          "QPushButton { background: #374151; } "
                          "QPushButton:hover { background: #1F2937; }");

  topRow->addWidget(m_nameInput);
  topRow->addWidget(btnAdd);
  topRow->addWidget(btnRemove);
  topRow->addSpacing(24);
  topRow->addWidget(btnFind);
  topRow->addWidget(btnReset);
  topRow->addStretch(1);

  m_statusLabel->setStyleSheet(
      "color: #EBEDFF; font-size: 13px; font-weight: 500;");

  auto *hint = new QLabel("Click = Start · Shift+Click = End · Drag sections? "
                          "(feature coming soon)",
                          this);
  hint->setStyleSheet("color: #8C94BE; font-size: 11px;");

  vlay->addLayout(topRow);
  vlay->addWidget(m_statusLabel);
  vlay->addWidget(hint);

  toolbar->setGeometry(0, 0, 1000, 110);

  connect(btnAdd, &QPushButton::clicked, this,
          &VisSupermarket::onAddSectionClicked);
  connect(btnRemove, &QPushButton::clicked, this,
          &VisSupermarket::onRemoveSectionClicked);
  connect(btnFind, &QPushButton::clicked, this,
          &VisSupermarket::onFindPathClicked);
  connect(btnReset, &QPushButton::clicked, this,
          &VisSupermarket::onResetClicked);
}

void VisSupermarket::updateStatus(const QString &text) {
  m_statusLabel->setText(text);
}

void VisSupermarket::buildHipermarketLayout() {
  delete m_graph;
  m_graph = new DirectedGraph();
  m_nodeNames.clear();
  m_startIdx = -1;
  m_endIdx = -1;
  m_finalPath.clear();
  m_pathCost = 0;
  m_mode = InteractionMode::Normal;
  updateStatus("Hipermarket layout loaded. Add nodes or click to find paths.");

  struct Section {
    QString name;
    QPoint pos;
  };
  std::vector<Section> sections = {
      {"Entrance", {80, 400}},       {"Flower Shop", {180, 220}},
      {"Bakery", {180, 580}},        {"Fruits & Veg", {320, 200}},
      {"Vegetables", {320, 350}},    {"Meat Buffet", {320, 500}},
      {"Deli Area", {320, 650}},     {"Dairy Row", {500, 180}},
      {"Milk Shop", {500, 330}},     {"Cheese", {500, 480}},
      {"Yogurt", {500, 630}},        {"Frozen Meals", {680, 160}},
      {"Frozen Vegies", {680, 310}}, {"Ice Cream", {680, 460}},
      {"Frozen Pastry", {680, 610}}, {"Beverages", {840, 200}},
      {"Pet Care", {840, 400}},      {"Cleaning", {840, 600}},
      {"Self Service", {960, 300}},  {"Cashier 1", {960, 450}},
      {"Cashier 2", {960, 600}}};

  for (const auto &s : sections) {
    m_graph->addNode(s.pos);
    int idx = m_graph->getNodes().back().getIndex();
    m_nodeNames[idx] = s.name;
  }

  const std::vector<Node> &nodes = m_graph->getNodes();
  for (size_t i = 0; i < nodes.size(); ++i) {
    for (size_t j = i + 1; j < nodes.size(); ++j) {
      int dx = nodes[i].getX() - nodes[j].getX();
      int dy = nodes[i].getY() - nodes[j].getY();
      int dist = static_cast<int>(std::sqrt(dx * dx + dy * dy));
      if (dist < 250) {
        m_graph->addEdge(const_cast<Node &>(nodes[i]),
                         const_cast<Node &>(nodes[j]), dist);
        m_graph->addEdge(const_cast<Node &>(nodes[j]),
                         const_cast<Node &>(nodes[i]), dist);
      }
    }
  }
  update();
}

int VisSupermarket::nodeAt(QPoint pos) const {
  if (!m_graph || m_graph->getNodes().empty())
    return -1;

  const std::vector<Node> &nodes = m_graph->getNodes();
  int minX = nodes[0].getX(), maxX = nodes[0].getX();
  int minY = nodes[0].getY(), maxY = nodes[0].getY();
  for (const Node &n : nodes) {
    minX = std::min(minX, n.getX());
    maxX = std::max(maxX, n.getX());
    minY = std::min(minY, n.getY());
    maxY = std::max(maxY, n.getY());
  }

  double padX = 80, padY = 60, bottomPad = 120;
  double viewW = width();
  double viewH = height() - 100;
  double graphW = (maxX - minX) + padX * 2;
  double graphH = (maxY - minY) + padY + bottomPad;
  double scale = std::min(viewW / graphW, viewH / graphH);

  double offsetX = (viewW - graphW * scale) / 2.0;
  double offsetY = 100 + (viewH - graphH * scale) / 2.0;

  QPointF p = (QPointF(pos) - QPointF(offsetX, offsetY)) / scale +
              QPointF(minX - padX, minY - padY);

  for (const Node &n : nodes) {
    double dx = p.x() - n.getX();
    double dy = p.y() - n.getY();
    if (std::sqrt(dx * dx + dy * dy) <= (NODE_R) + 5)
      return n.getIndex();
  }
  return -1;
}

void VisSupermarket::onAddSectionClicked() {
  if (m_nameInput->text().trimmed().isEmpty()) {
    updateStatus("❌ Please enter a section name first!");
    return;
  }
  m_mode = InteractionMode::Adding;
  updateStatus("📍 Click on the map to place " + m_nameInput->text().trimmed());
}

void VisSupermarket::onRemoveSectionClicked() {
  m_mode = InteractionMode::Removing;
  updateStatus("🗑 Click a section to remove it.");
}

void VisSupermarket::mousePressEvent(QMouseEvent *e) {
  QPoint rawPos = e->pos();
  if (!m_graph)
    return;

  const std::vector<Node> &nodes = m_graph->getNodes();
  int minX = nodes.empty() ? 0 : nodes[0].getX(),
      maxX = nodes.empty() ? 1000 : nodes[0].getX();
  int minY = nodes.empty() ? 0 : nodes[0].getY(),
      maxY = nodes.empty() ? 700 : nodes[0].getY();
  for (const Node &n : nodes) {
    minX = std::min(minX, n.getX());
    maxX = std::max(maxX, n.getX());
    minY = std::min(minY, n.getY());
    maxY = std::max(maxY, n.getY());
  }

  double padX = 80, padY = 60, bottomPad = 120;
  double viewW = width();
  double viewH = height() - 100;
  double graphW = (maxX - minX) + padX * 2;
  double graphH = (maxY - minY) + padY + bottomPad;
  double scale = std::min(viewW / graphW, viewH / graphH);
  double offsetX = (viewW - graphW * scale) / 2.0;
  double offsetY = 100 + (viewH - graphH * scale) / 2.0;
  QPointF graphPos = (QPointF(rawPos) - QPointF(offsetX, offsetY)) / scale +
                     QPointF(minX - padX, minY - padY);

  int hit = nodeAt(rawPos);

  if (m_mode == InteractionMode::Adding) {
    QString name = m_nameInput->text().trimmed();
    m_graph->addNode(graphPos.toPoint()); // Use transformed coordinates
    int idx = m_graph->getNodes().back().getIndex();
    m_nodeNames[idx] = name;

    // Auto-connect to neighbors
    const std::vector<Node> &allNodes = m_graph->getNodes();
    Node &newNode = const_cast<Node &>(allNodes.back());
    for (size_t i = 0; i < allNodes.size() - 1; ++i) {
      int dx = newNode.getX() - allNodes[i].getX();
      int dy = newNode.getY() - allNodes[i].getY();
      int dist = static_cast<int>(std::sqrt(dx * dx + dy * dy));
      if (dist < 250) {
        m_graph->addEdge(newNode, const_cast<Node &>(allNodes[i]), dist);
        m_graph->addEdge(const_cast<Node &>(allNodes[i]), newNode, dist);
      }
    }
    m_nameInput->clear();
    m_mode = InteractionMode::Normal;
    updateStatus("✅ Added section: " + name);
    update();
    return;
  }

  if (m_mode == InteractionMode::Removing) {
    if (hit != -1) {
      QString name = m_nodeNames[hit];

      std::vector<Node> oldNodes = m_graph->getNodes();

      struct Ed {
        int u, v, cost;
      };
      std::vector<Ed> oldEdgesData;
      for (const auto &ed : m_graph->getEdges()) {
        oldEdgesData.push_back({ed.getFirst().getIndex(),
                                ed.getSecond().getIndex(), ed.getCost()});
      }
      auto oldNames = m_nodeNames;

      delete m_graph;
      m_graph = new DirectedGraph();
      m_nodeNames.clear();

      std::unordered_map<int, int> oldToNew;
      for (const auto &n : oldNodes) {
        if (n.getIndex() == hit)
          continue;
        m_graph->addNode(n.getCoord());
        int newIdx = m_graph->getNodes().back().getIndex();
        m_nodeNames[newIdx] = oldNames[n.getIndex()];
        oldToNew[n.getIndex()] = newIdx;
      }

      for (const auto &ed : oldEdgesData) {
        if (ed.u == hit || ed.v == hit)
          continue;

        auto &nodes = m_graph->getNodes();
        Node *f = nullptr;
        Node *s = nullptr;
        for (auto &n : nodes) {
          if (n.getIndex() == oldToNew[ed.u])
            f = &n;
          if (n.getIndex() == oldToNew[ed.v])
            s = &n;
        }
        if (f && s)
          m_graph->addEdge(*f, *s, ed.cost);
      }

      m_startIdx = m_endIdx = -1;
      m_finalPath.clear();
      m_mode = InteractionMode::Normal;
      updateStatus("🗑 Removed: " + name);
      update();
    }
    return;
  }

  if (hit == -1)
    return;

  if (e->modifiers() & Qt::ShiftModifier) {
    m_endIdx = hit;
  } else {
    m_startIdx = hit;
    if (m_startIdx == m_endIdx)
      m_endIdx = -1;
  }

  m_finalPath.clear();
  m_pathCost = 0;

  if (m_startIdx != -1 && m_endIdx != -1) {
    onFindPathClicked();
  } else {
    QString startName = (m_startIdx != -1) ? m_nodeNames[m_startIdx] : "...";
    QString endName = (m_endIdx != -1) ? m_nodeNames[m_endIdx] : "...";
    updateStatus(QString("Route: %1 → %2").arg(startName, endName));
  }
  update();
}

void VisSupermarket::onFindPathClicked() {
  if (m_startIdx == -1 || m_endIdx == -1) {
    updateStatus("❌ Please select both Start and End sections.");
    return;
  }
  std::vector<int> path;
  if (m_graph)
    m_graph->dijkstra(m_startIdx, m_endIdx, path);
  m_finalPath = path;
  if (m_finalPath.empty()) {
    updateStatus("⚠️ No route found between " + m_nodeNames[m_startIdx] +
                 " and " + m_nodeNames[m_endIdx]);
  } else {
    m_pathCost = 0;
    const std::vector<Edge> &edges = m_graph->getEdges();
    for (size_t i = 0; i + 1 < m_finalPath.size(); ++i) {
      for (const Edge &e : edges) {
        if (e.getFirst().getIndex() == m_finalPath[i] &&
            e.getSecond().getIndex() == m_finalPath[i + 1]) {
          m_pathCost += e.getCost();
          break;
        }
      }
    }
    updateStatus(QString("🎯 Found Shortest Route! Total distance: %1 meters")
                     .arg(m_pathCost));
  }
  update();
}

void VisSupermarket::onResetClicked() { buildHipermarketLayout(); }

void VisSupermarket::paintEvent(QPaintEvent *) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  p.fillRect(rect(), QColor(8, 10, 20));

  if (!m_graph || m_graph->getNodes().empty())
    return;

  const std::vector<Node> &nodes = m_graph->getNodes();
  int minX = nodes[0].getX(), maxX = nodes[0].getX();
  int minY = nodes[0].getY(), maxY = nodes[0].getY();
  for (const Node &n : nodes) {
    minX = std::min(minX, n.getX());
    maxX = std::max(maxX, n.getX());
    minY = std::min(minY, n.getY());
    maxY = std::max(maxY, n.getY());
  }

  // Dynamic scaling based on absolute node bounds
  double padX = 80, padY = 60, bottomPad = 120;
  double viewW = width();
  double viewH = height() - 100;
  double graphW = (maxX - minX) + padX * 2;
  double graphH = (maxY - minY) + padY + bottomPad;
  double scale = std::min(viewW / graphW, viewH / graphH);

  double offsetX = (viewW - graphW * scale) / 2.0;
  double offsetY = 100 + (viewH - graphH * scale) / 2.0;

  p.save();
  p.translate(offsetX, offsetY);
  p.scale(scale, scale);
  p.translate(-(minX - padX), -(minY - padY));

  p.setPen(QPen(QColor(25, 30, 50), 1));
  for (int x = -1000; x < 2000; x += 50)
    p.drawLine(x, -1000, x, 2000);
  for (int y = -1000; y < 2000; y += 50)
    p.drawLine(-1000, y, 2000, y);

  p.setPen(QPen(QColor(45, 55, 100, 150), 2));
  for (const Edge &e : m_graph->getEdges()) {
    if (e.getFirst().getIndex() > e.getSecond().getIndex())
      continue;
    p.drawLine(QPoint(e.getFirst().getX(), e.getFirst().getY()),
               QPoint(e.getSecond().getX(), e.getSecond().getY()));
  }

  if (!m_finalPath.empty()) {
    p.setPen(QPen(QColor(99, 102, 241), 5, Qt::SolidLine, Qt::RoundCap,
                  Qt::RoundJoin));
    for (size_t i = 0; i + 1 < m_finalPath.size(); ++i) {
      QPoint p1, p2;
      for (const Node &n : m_graph->getNodes()) {
        if (n.getIndex() == m_finalPath[i])
          p1 = QPoint(n.getX(), n.getY());
        if (n.getIndex() == m_finalPath[i + 1])
          p2 = QPoint(n.getX(), n.getY());
      }
      p.drawLine(p1, p2);
    }
  }

  for (const Node &n : m_graph->getNodes()) {
    int idx = n.getIndex();
    bool isStart = (idx == m_startIdx);
    bool isEnd = (idx == m_endIdx);
    bool inPath = false;
    for (int pi : m_finalPath)
      if (pi == idx) {
        inPath = true;
        break;
      }

    QColor fill = isStart  ? QColor(34, 197, 94)
                  : isEnd  ? QColor(239, 68, 68)
                  : inPath ? QColor(99, 102, 241)
                           : QColor(30, 41, 59);

    QRect r(n.getX() - NODE_R, n.getY() - NODE_R, NODE_R * 2, NODE_R * 2);
    p.setBrush(fill);
    p.setPen(QPen(Qt::white, inPath ? 3 : 1));
    p.drawEllipse(r);

    p.setPen(Qt::white);
    p.setFont(QFont("Segoe UI", 8, QFont::Bold));
    QString initials = m_nodeNames[idx].split(" ").last().left(2).toUpper();
    p.drawText(r, Qt::AlignCenter, initials);

    p.setPen(QColor(140, 148, 190));
    p.setFont(QFont("Segoe UI", 9));
    p.drawText(QRect(n.getX() - 50, n.getY() + NODE_R + 10, 100, 20),
               Qt::AlignCenter, m_nodeNames[idx]);
  }
  p.restore();
}
