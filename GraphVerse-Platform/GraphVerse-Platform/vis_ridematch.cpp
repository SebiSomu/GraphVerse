#include "vis_ridematch.h"
#include <QHBoxLayout>
#include <QLineF>
#include <QMouseEvent>
#include <QPainter>
#include <QRandomGenerator>
#include <QResizeEvent>
#include <queue>

VisRideMatch::VisRideMatch(QWidget *parent)
    : QWidget(parent), m_toolbar(nullptr), m_statusLabel(new QLabel(this)),
      m_hintLabel(new QLabel(this)), m_btnPassenger(new QPushButton(this)),
      m_btnDriver(new QPushButton(this)), m_btnSample(new QPushButton(this)),
      m_btnMatch(new QPushButton(this)), m_btnClear(new QPushButton(this)),
      m_mode(Mode::None), m_passengers(), m_drivers(), m_candidateEdges(),
      m_matchedPairs(), m_nextPassengerId(1), m_nextDriverId(1),
      m_matchDistance(230) {
  setMinimumSize(1200, 720);
  setupUi();
  updateStatus();
}

void VisRideMatch::setupUi() {
  m_toolbar = new QWidget(this);
  auto *hbox = new QHBoxLayout(m_toolbar);
  hbox->setContentsMargins(8, 8, 8, 4);

  m_btnPassenger->setText("Add Passenger");
  m_btnDriver->setText("Add Driver");
  m_btnSample->setText("Sample Data");
  m_btnMatch->setText("Run Matching");
  m_btnClear->setText("Clear");

  for (auto *b :
       {m_btnPassenger, m_btnDriver, m_btnSample, m_btnMatch, m_btnClear}) {
    b->setFixedHeight(34);
    b->setMinimumWidth(120);
  }

  QString base = "QPushButton{border-radius:6px;font-weight:bold;font-size:12px;}";
  m_btnPassenger->setStyleSheet(base + "QPushButton{background:#1e90ff;color:white;}QPushButton:hover{background:#1070d0;}");
  m_btnDriver->setStyleSheet(base + "QPushButton{background:#16a34a;color:white;}QPushButton:hover{background:#15803d;}");
  m_btnSample->setStyleSheet(base + "QPushButton{background:#7c3aed;color:white;}QPushButton:hover{background:#6d28d9;}");
  m_btnMatch->setStyleSheet(base + "QPushButton{background:#f59e0b;color:white;}QPushButton:hover{background:#d97706;}");
  m_btnClear->setStyleSheet(base + "QPushButton{background:#ef4444;color:white;}QPushButton:hover{background:#dc2626;}");

  m_statusLabel->setStyleSheet("color:#60a5fa;font-size:12px;font-weight:bold;");
  m_statusLabel->setAlignment(Qt::AlignCenter);
  m_statusLabel->setMinimumWidth(460);

  m_hintLabel->setText("Click canvas to place nodes. Blue=Passenger, Green=Driver, Gold=Match");
  m_hintLabel->setStyleSheet("color:#9ca3af;font-size:11px;");

  hbox->addWidget(m_btnPassenger);
  hbox->addWidget(m_btnDriver);
  hbox->addWidget(m_btnSample);
  hbox->addWidget(m_btnMatch);
  hbox->addWidget(m_btnClear);
  hbox->addStretch(1);
  hbox->addWidget(m_statusLabel, 2);
  hbox->addStretch(1);
  hbox->addWidget(m_hintLabel);

  connect(m_btnPassenger, &QPushButton::clicked, this, &VisRideMatch::onModePassenger);
  connect(m_btnDriver, &QPushButton::clicked, this, &VisRideMatch::onModeDriver);
  connect(m_btnSample, &QPushButton::clicked, this, &VisRideMatch::onGenerateSample);
  connect(m_btnMatch, &QPushButton::clicked, this, &VisRideMatch::onRunMatching);
  connect(m_btnClear, &QPushButton::clicked, this, &VisRideMatch::onClear);
}

void VisRideMatch::resizeEvent(QResizeEvent *e) {
  QWidget::resizeEvent(e);
  if (m_toolbar)
    m_toolbar->setGeometry(0, 0, width(), 50);
}

QRect VisRideMatch::canvasRect() const { return QRect(0, 56, width(), height() - 56); }

void VisRideMatch::mousePressEvent(QMouseEvent *event) {
  if (!canvasRect().contains(event->pos()))
    return;
  if (m_mode == Mode::AddPassenger) {
    m_passengers.push_back(Passenger{event->pos(), m_nextPassengerId++});
  } else if (m_mode == Mode::AddDriver) {
    m_drivers.push_back(Driver{event->pos(), m_nextDriverId++});
  } else {
    return;
  }
  recomputeCandidates();
  m_matchedPairs.clear();
  updateStatus();
  update();
}

void VisRideMatch::onModePassenger() {
  m_mode = Mode::AddPassenger;
  updateStatus();
}

void VisRideMatch::onModeDriver() {
  m_mode = Mode::AddDriver;
  updateStatus();
}

void VisRideMatch::onGenerateSample() {
  m_passengers.clear();
  m_drivers.clear();
  m_matchedPairs.clear();
  m_nextPassengerId = 1;
  m_nextDriverId = 1;
  QRect area = canvasRect().adjusted(40, 20, -40, -20);
  int pCount = 9;
  int dCount = 7;
  for (int i = 0; i < pCount; ++i) {
    int x = QRandomGenerator::global()->bounded(area.left(), area.center().x() - 20);
    int y = QRandomGenerator::global()->bounded(area.top(), area.bottom());
    m_passengers.push_back(Passenger{QPoint(x, y), m_nextPassengerId++});
  }
  for (int i = 0; i < dCount; ++i) {
    int x = QRandomGenerator::global()->bounded(area.center().x() + 20, area.right());
    int y = QRandomGenerator::global()->bounded(area.top(), area.bottom());
    m_drivers.push_back(Driver{QPoint(x, y), m_nextDriverId++});
  }
  recomputeCandidates();
  updateStatus();
  update();
}

void VisRideMatch::onRunMatching() {
  recomputeCandidates();
  int maxMatch = runFordFulkerson();
  m_statusLabel->setText(
      QString("RideMatch (Ford-Fulkerson): %1/%2 passengers matched, %3 drivers")
          .arg(maxMatch)
          .arg(m_passengers.size())
          .arg(m_drivers.size()));
  update();
}

void VisRideMatch::onClear() {
  m_passengers.clear();
  m_drivers.clear();
  m_candidateEdges.clear();
  m_matchedPairs.clear();
  m_nextPassengerId = 1;
  m_nextDriverId = 1;
  m_mode = Mode::None;
  updateStatus();
  update();
}

void VisRideMatch::recomputeCandidates() {
  m_candidateEdges.clear();
  for (int p = 0; p < static_cast<int>(m_passengers.size()); ++p) {
    for (int d = 0; d < static_cast<int>(m_drivers.size()); ++d) {
      double dist = QLineF(m_passengers[p].pos, m_drivers[d].pos).length();
      if (dist <= m_matchDistance)
        m_candidateEdges.emplace_back(p, d);
    }
  }
}

int VisRideMatch::runFordFulkerson() {
  int P = static_cast<int>(m_passengers.size());
  int D = static_cast<int>(m_drivers.size());
  int N = 2 + P + D;
  int source = 0;
  int sink = N - 1;
  std::vector<std::vector<int>> capacity(N, std::vector<int>(N, 0));
  for (int p = 0; p < P; ++p)
    capacity[source][1 + p] = 1;
  for (const auto &edge : m_candidateEdges) {
    int p = edge.first;
    int d = edge.second;
    capacity[1 + p][1 + P + d] = 1;
  }
  for (int d = 0; d < D; ++d)
    capacity[1 + P + d][sink] = 1;

  std::vector<std::vector<int>> residual = capacity;
  std::vector<int> parent(N, -1);
  auto bfs = [&]() -> bool {
    std::fill(parent.begin(), parent.end(), -1);
    std::queue<int> q;
    q.push(source);
    parent[source] = source;
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      for (int v = 0; v < N; ++v) {
        if (parent[v] == -1 && residual[u][v] > 0) {
          parent[v] = u;
          if (v == sink)
            return true;
          q.push(v);
        }
      }
    }
    return false;
  };

  int maxFlow = 0;
  while (bfs()) {
    int pathFlow = 1;
    for (int v = sink; v != source; v = parent[v]) {
      int u = parent[v];
      pathFlow = std::min(pathFlow, residual[u][v]);
    }
    for (int v = sink; v != source; v = parent[v]) {
      int u = parent[v];
      residual[u][v] -= pathFlow;
      residual[v][u] += pathFlow;
    }
    maxFlow += pathFlow;
  }

  m_matchedPairs.clear();
  for (int p = 0; p < P; ++p) {
    int pNode = 1 + p;
    for (int d = 0; d < D; ++d) {
      int dNode = 1 + P + d;
      if (capacity[pNode][dNode] > 0 && residual[pNode][dNode] == 0)
        m_matchedPairs.emplace_back(p, d);
    }
  }

  return maxFlow;
}

void VisRideMatch::updateStatus() {
  QString modeTxt = "Mode: View";
  if (m_mode == Mode::AddPassenger)
    modeTxt = "Mode: Add Passenger";
  else if (m_mode == Mode::AddDriver)
    modeTxt = "Mode: Add Driver";
  m_statusLabel->setText(
      QString("%1 | Passengers: %2 | Drivers: %3 | Viable edges: %4")
          .arg(modeTxt)
          .arg(m_passengers.size())
          .arg(m_drivers.size())
          .arg(m_candidateEdges.size()));
}

void VisRideMatch::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  p.fillRect(rect(), Qt::black);
  QRect area = canvasRect();
  p.setPen(QPen(QColor(35, 43, 68), 1));
  p.drawRect(area.adjusted(0, 0, -1, -1));

  p.setPen(QPen(QColor(80, 80, 95), 1));
  for (const auto &edge : m_candidateEdges) {
    const auto &a = m_passengers[edge.first].pos;
    const auto &b = m_drivers[edge.second].pos;
    p.drawLine(a, b);
  }

  p.setPen(QPen(QColor(255, 215, 0), 3));
  for (const auto &edge : m_matchedPairs) {
    const auto &a = m_passengers[edge.first].pos;
    const auto &b = m_drivers[edge.second].pos;
    p.drawLine(a, b);
  }

  for (const auto &passenger : m_passengers) {
    QRect r(passenger.pos.x() - 11, passenger.pos.y() - 11, 22, 22);
    p.setBrush(QColor(30, 144, 255));
    p.setPen(QPen(Qt::white, 1));
    p.drawEllipse(r);
    p.setPen(Qt::white);
    p.setFont(QFont("Segoe UI", 7, QFont::Bold));
    p.drawText(r, Qt::AlignCenter, QString("P%1").arg(passenger.id));
  }

  for (const auto &driver : m_drivers) {
    QRect r(driver.pos.x() - 11, driver.pos.y() - 11, 22, 22);
    p.setBrush(QColor(22, 163, 74));
    p.setPen(QPen(Qt::white, 1));
    p.drawEllipse(r);
    p.setPen(Qt::white);
    p.setFont(QFont("Segoe UI", 7, QFont::Bold));
    p.drawText(r, Qt::AlignCenter, QString("D%1").arg(driver.id));
  }
}
