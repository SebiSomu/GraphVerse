#include "vis_maze.h"
#include "algorithms/bfs_traversal.h"
#include "algorithms/dfs_traversal.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QLabel>
#include <ctime>
#include <random>
#include <stack>
#include <algorithm>

static constexpr int DR[4]  = {-1,  0,  1,  0};
static constexpr int DC[4]  = { 0,  1,  0, -1};
static constexpr int OPP[4] = { 2,  3,  0,  1};
static inline int cellToIndex(int r, int c, int cols) { return r * cols + c + 1; }
static inline int indexToRow(int idx, int cols) { return (idx - 1) / cols; }
static inline int indexToCol(int idx, int cols) { return (idx - 1) % cols; }

VisMaze::VisMaze(QWidget* parent)
    : QWidget(parent), m_timer(new QTimer(this)), m_animStep(0), m_animDone(false),
      m_showingPath(false), m_pathStep(0), m_currentAlgo(AlgoType::None), m_graph(nullptr), m_toolbar(nullptr)
{
    setMinimumSize(COLS * CELL_SIZE + 20, ROWS * CELL_SIZE + 140);

    auto* btnBfs = new QPushButton("BFS", this);
    auto* btnDfs = new QPushButton("DFS", this);
    auto* btnReset = new QPushButton("Reset", this);
    m_pauseBtn = new QPushButton("Pause", this);
    m_statusLabel = new QLabel(this);
    m_hintLabel = new QLabel("Start: top-left  |  Exit: bottom-right", this);
    m_speedLabel = new QLabel("Speed", this);
    m_speedSlider = new QSlider(Qt::Horizontal, this);
    m_pauseBtn->setEnabled(false);
    m_paused = false;
    m_tickInterval = 18;
    m_speedSlider->setRange(1, 100);
    m_speedSlider->setValue(55);
    m_speedSlider->setFixedWidth(220);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_hintLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_speedLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_statusLabel->setMinimumWidth(420);
    m_hintLabel->setMinimumWidth(240);

    for(auto* btn : {btnBfs, btnDfs, btnReset, m_pauseBtn}) { btn->setFixedHeight(34); btn->setFixedWidth(96); }
    m_hintLabel->setStyleSheet("color: #aaaaaa; font-size: 11px;");
    m_statusLabel->setStyleSheet("color:#5fa8ff;font-size:12px;font-weight:bold;");
    m_speedLabel->setStyleSheet("color:#cbd5e1;font-size:11px;");
    m_speedSlider->setStyleSheet(
        "QSlider::groove:horizontal { height: 4px; background: #374151; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: #60a5fa; width: 12px; margin: -5px 0; border-radius: 6px; }");

    QString base = "QPushButton { border-radius:6px; font-weight:bold; font-size:13px; }";
    btnBfs->setStyleSheet(base + "QPushButton { background:#1e90ff; color:white; } QPushButton:hover { background:#1070d0; }");
    btnDfs->setStyleSheet(base + "QPushButton { background:#27ae60; color:white; } QPushButton:hover { background:#1e8449; }");
    btnReset->setStyleSheet(base + "QPushButton { background:#e74c3c; color:white; } QPushButton:hover { background:#c0392b; }");
    m_pauseBtn->setStyleSheet(base + "QPushButton { background:#6b7280; color:white; } QPushButton:hover { background:#4b5563; }");

    m_toolbar = new QWidget(this);
    auto* hbox = new QHBoxLayout(m_toolbar);
    hbox->addWidget(btnBfs);
    hbox->addWidget(btnDfs);
    hbox->addWidget(btnReset);
    hbox->addWidget(m_pauseBtn);
    hbox->addSpacing(8);
    hbox->addWidget(m_speedLabel);
    hbox->addWidget(m_speedSlider);
    hbox->addStretch(1);
    hbox->addWidget(m_statusLabel, 2);
    hbox->addStretch(1);
    hbox->addWidget(m_hintLabel);
    hbox->setContentsMargins(8, 8, 8, 4);
    m_toolbar->setGeometry(0, 0, width(), 52);

    connect(btnBfs, &QPushButton::clicked, this, &VisMaze::onBfsClicked);
    connect(btnDfs, &QPushButton::clicked, this, &VisMaze::onDfsClicked);
    connect(btnReset, &QPushButton::clicked, this, &VisMaze::onResetClicked);
    connect(m_pauseBtn, &QPushButton::clicked, this, &VisMaze::onPauseClicked);
    connect(m_speedSlider, &QSlider::valueChanged, this, &VisMaze::onSpeedChanged);
    connect(m_timer, &QTimer::timeout, this, &VisMaze::onAnimationTick);

    updateStatusLabel();
    srand(static_cast<unsigned>(time(nullptr)));
    generateMaze();
}

VisMaze::~VisMaze() { delete m_graph; }

void VisMaze::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    if (m_toolbar)
        m_toolbar->setGeometry(0, 0, width(), 52);
}

void VisMaze::generateMaze()
{
    m_timer->stop();
    m_maze.assign(ROWS, std::vector<Cell>(COLS));
    m_visited.assign(ROWS, std::vector<bool>(COLS, false));
    m_visitOrder.clear(); m_finalPath.clear(); m_pathHighlight.clear();
    m_animStep = 0; m_animDone = false; m_showingPath = false; m_pathStep = 0;
    m_currentAlgo = AlgoType::None;
    m_paused = false;
    m_tickInterval = 18;
    m_pauseBtn->setText("Pause");
    m_pauseBtn->setEnabled(false);
    updateStatusLabel();

    std::mt19937 rng(static_cast<unsigned>(time(nullptr)));
    std::vector<std::vector<bool>> visited(ROWS, std::vector<bool>(COLS, false));
    std::stack<std::pair<int,int>> stk;
    stk.emplace(0, 0); visited[0][0] = true;
    while(!stk.empty()) {
        auto [r, c] = stk.top();
        std::vector<int> neighbors;
        for(int d = 0; d < 4; d++) {
            int nr = r+DR[d], nc = c+DC[d];
            if(nr>=0 && nr<ROWS && nc>=0 && nc<COLS && !visited[nr][nc]) neighbors.push_back(d);
        }
        if(neighbors.empty()) { stk.pop(); }
        else {
            std::uniform_int_distribution<int> dist(0, static_cast<int>(neighbors.size())-1);
            int d = neighbors[dist(rng)]; int nr = r+DR[d], nc = c+DC[d];
            m_maze[r][c].walls[d] = false; m_maze[nr][nc].walls[OPP[d]] = false;
            visited[nr][nc] = true; stk.emplace(nr, nc);
        }
    }

    delete m_graph; m_graph = new DirectedGraph();
    for(int r = 0; r < ROWS; r++)
        for(int c = 0; c < COLS; c++)
            m_graph->addNode(QPoint(offsetX() + c*CELL_SIZE + CELL_SIZE/2, offsetY() + r*CELL_SIZE + CELL_SIZE/2));
    
    std::vector<Node*> nodes;
    for(auto& n : m_graph->getNodes()) nodes.push_back(&n);
    
    for(int r = 0; r < ROWS; r++)
        for(int c = 0; c < COLS; c++)
            for(int d = 0; d < 4; d++) {
                if(m_maze[r][c].walls[d]) continue;
                int nr = r+DR[d], nc = c+DC[d];
                m_graph->addEdge(*nodes[cellToIndex(r,c,COLS)-1], *nodes[cellToIndex(nr,nc,COLS)-1]);
            }
    update();
}

static void reconstructPath(int endIdx, int cols, const std::unordered_map<int,int>& parent,
                            std::vector<std::pair<int,int>>& outPath) {
    outPath.clear();
    for(int cur = endIdx; cur != -1; cur = parent.count(cur) ? parent.at(cur) : -1)
        outPath.emplace_back(indexToRow(cur, cols), indexToCol(cur, cols));
    std::reverse(outPath.begin(), outPath.end());
}

std::vector<std::pair<int,int>> VisMaze::runBFS(std::vector<std::pair<int,int>>& outPath) {
    const int startIdx = cellToIndex(0, 0, COLS), endIdx = cellToIndex(ROWS-1, COLS-1, COLS);
    std::unordered_map<int,int> parent; parent[startIdx] = -1;
    BFSTraversal bfs;
    std::vector<int> order = bfs.solve(*m_graph, startIdx, endIdx, nullptr, [&](int par, int child){ parent[child] = par; });
    std::vector<std::pair<int,int>> result;
    for(int idx : order) result.emplace_back(indexToRow(idx, COLS), indexToCol(idx, COLS));
    reconstructPath(endIdx, COLS, parent, outPath);
    return result;
}

std::vector<std::pair<int,int>> VisMaze::runDFS(std::vector<std::pair<int,int>>& outPath) {
    const int startIdx = cellToIndex(0, 0, COLS), endIdx = cellToIndex(ROWS-1, COLS-1, COLS);
    std::unordered_map<int,int> parent; parent[startIdx] = -1;
    DFSTraversal dfs;
    std::vector<int> order = dfs.solve(*m_graph, startIdx, endIdx, nullptr, [&](int par, int child){ parent[child] = par; });
    std::vector<std::pair<int,int>> result;
    for(int idx : order) result.emplace_back(indexToRow(idx, COLS), indexToCol(idx, COLS));
    reconstructPath(endIdx, COLS, parent, outPath);
    return result;
}

void VisMaze::startAnimation(AlgoType algo) {
    m_timer->stop(); m_currentAlgo = algo;
    m_paused = false;
    m_pauseBtn->setText("Pause");
    m_pauseBtn->setEnabled(true);
    m_visited.assign(ROWS, std::vector<bool>(COLS, false));
    m_pathHighlight.clear(); m_animStep = 0; m_animDone = false; m_showingPath = false; m_pathStep = 0;
    if(algo == AlgoType::BFS) m_visitOrder = runBFS(m_finalPath);
    else m_visitOrder = runDFS(m_finalPath);
    m_tickInterval = currentInterval();
    updateStatusLabel();
    m_timer->start(m_tickInterval);
}

void VisMaze::onAnimationTick() {
    if(!m_showingPath) {
        if(m_animStep < static_cast<int>(m_visitOrder.size())) {
            auto [r, c] = m_visitOrder[m_animStep++]; m_visited[r][c] = true; update();
            updateStatusLabel();
        } else { m_showingPath = true; m_pathStep = 0; m_tickInterval = currentInterval(); m_timer->setInterval(m_tickInterval); updateStatusLabel(); }
    } else {
        if(m_pathStep < static_cast<int>(m_finalPath.size())) { m_pathHighlight.push_back(m_finalPath[m_pathStep++]); update(); updateStatusLabel(); }
        else { m_timer->stop(); m_animDone = true; m_pauseBtn->setEnabled(false); updateStatusLabel(); update(); }
    }
}

void VisMaze::onBfsClicked() { startAnimation(AlgoType::BFS); }
void VisMaze::onDfsClicked() { startAnimation(AlgoType::DFS); }
void VisMaze::onResetClicked() { m_timer->stop(); generateMaze(); }
void VisMaze::onPauseClicked() {
    if (m_currentAlgo == AlgoType::None || m_animDone)
        return;
    if (m_paused) {
        m_paused = false;
        m_pauseBtn->setText("Pause");
        updateStatusLabel();
        m_timer->start(m_tickInterval);
        return;
    }
    m_paused = true;
    m_pauseBtn->setText("Resume");
    updateStatusLabel();
    m_timer->stop();
}

void VisMaze::onSpeedChanged(int) {
    m_tickInterval = currentInterval();
    if (m_timer->isActive())
        m_timer->setInterval(m_tickInterval);
}

int VisMaze::currentInterval() const {
    int slider = m_speedSlider ? m_speedSlider->value() : 55;
    int maxMs = m_showingPath ? 180 : 120;
    int minMs = m_showingPath ? 6 : 3;
    int slow = 101 - slider;
    int range = maxMs - minMs;
    return minMs + (slow * slow * range) / 10000;
}

void VisMaze::updateStatusLabel() {
    if (m_currentAlgo == AlgoType::None) {
        m_statusLabel->setText("Ready. Choose BFS or DFS");
        m_statusLabel->setStyleSheet("color:#94a3b8;font-size:12px;font-weight:bold;");
        return;
    }
    QString algoName = (m_currentAlgo == AlgoType::BFS) ? "BFS" : "DFS";
    QColor algoCol = (m_currentAlgo == AlgoType::BFS) ? QColor(80,160,255) : QColor(46,204,113);
    QString text;
    if (m_animDone) {
        text = QString("%1 - done! Path length: %2 steps").arg(algoName).arg(m_finalPath.size());
    } else if (m_paused) {
        text = QString("%1 - paused").arg(algoName);
    } else if (!m_showingPath) {
        text = QString("%1 - exploring... %2/%3").arg(algoName).arg(m_animStep).arg(m_visitOrder.size());
    } else {
        text = QString("%1 - building path... %2/%3").arg(algoName).arg(m_pathStep).arg(m_finalPath.size());
    }
    m_statusLabel->setText(text);
    m_statusLabel->setStyleSheet(QString("color:%1;font-size:12px;font-weight:bold;").arg(algoCol.name()));
}

void VisMaze::paintEvent(QPaintEvent*) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing, false);
    const int ox = offsetX(), oy = offsetY(), C = CELL_SIZE;
    p.fillRect(rect(), Qt::black);
    QColor visitColor = (m_currentAlgo == AlgoType::BFS) ? QColor(30,100,200,180) : QColor(39,174,96,180);
    for(int r = 0; r < ROWS; r++) for(int c = 0; c < COLS; c++)
        if(m_visited[r][c]) p.fillRect(ox+c*C+1, oy+r*C+1, C-2, C-2, visitColor);
    for(auto& [r, c] : m_pathHighlight) p.fillRect(ox+c*C+3, oy+r*C+3, C-6, C-6, QColor(255,220,0));
    p.setPen(QPen(QColor(190,200,220), 2));
    for(int r = 0; r < ROWS; r++) for(int c = 0; c < COLS; c++) {
        int x = ox+c*C, y = oy+r*C;
        if(m_maze[r][c].walls[0]) p.drawLine(x, y, x+C, y);
        if(m_maze[r][c].walls[1]) p.drawLine(x+C, y, x+C, y+C);
        if(m_maze[r][c].walls[2]) p.drawLine(x, y+C, x+C, y+C);
        if(m_maze[r][c].walls[3]) p.drawLine(x, y, x, y+C);
    }
    p.fillRect(ox+4, oy+4, C-8, C-8, QColor(0,210,80));
    p.fillRect(ox+(COLS-1)*C+4, oy+(ROWS-1)*C+4, C-8, C-8, QColor(220,40,40));
}
