#include "vis_maze.h"
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
      m_showingPath(false), m_pathStep(0), m_currentAlgo(AlgoType::None), m_graph(nullptr)
{
    setMinimumSize(COLS * CELL_SIZE + 20, ROWS * CELL_SIZE + 140);

    auto* btnBfs = new QPushButton("BFS", this);
    auto* btnDfs = new QPushButton("DFS", this);
    auto* btnReset = new QPushButton("Reset", this);
    auto* lblInfo = new QLabel("Start: top-left  |  Exit: bottom-right", this);

    for(auto* btn : {btnBfs, btnDfs, btnReset}) { btn->setFixedHeight(36); btn->setFixedWidth(110); }
    lblInfo->setAlignment(Qt::AlignCenter);
    lblInfo->setStyleSheet("color: #aaaaaa; font-size: 12px;");

    QString base = "QPushButton { border-radius:6px; font-weight:bold; font-size:13px; }";
    btnBfs->setStyleSheet(base + "QPushButton { background:#1e90ff; color:white; } QPushButton:hover { background:#1070d0; }");
    btnDfs->setStyleSheet(base + "QPushButton { background:#27ae60; color:white; } QPushButton:hover { background:#1e8449; }");
    btnReset->setStyleSheet(base + "QPushButton { background:#e74c3c; color:white; } QPushButton:hover { background:#c0392b; }");

    auto* toolbar = new QWidget(this);
    auto* hbox = new QHBoxLayout(toolbar);
    hbox->addWidget(btnBfs); hbox->addWidget(btnDfs); hbox->addSpacing(20);
    hbox->addWidget(btnReset); hbox->addStretch(); hbox->addWidget(lblInfo);
    hbox->setContentsMargins(8, 8, 8, 4);
    toolbar->setGeometry(0, 0, width(), 52);

    connect(btnBfs, &QPushButton::clicked, this, &VisMaze::onBfsClicked);
    connect(btnDfs, &QPushButton::clicked, this, &VisMaze::onDfsClicked);
    connect(btnReset, &QPushButton::clicked, this, &VisMaze::onResetClicked);
    connect(m_timer, &QTimer::timeout, this, &VisMaze::onAnimationTick);

    srand(static_cast<unsigned>(time(nullptr)));
    generateMaze();
}

VisMaze::~VisMaze() { delete m_graph; }

void VisMaze::generateMaze()
{
    m_timer->stop();
    m_maze.assign(ROWS, std::vector<Cell>(COLS));
    m_visited.assign(ROWS, std::vector<bool>(COLS, false));
    m_visitOrder.clear(); m_finalPath.clear(); m_pathHighlight.clear();
    m_animStep = 0; m_animDone = false; m_showingPath = false; m_pathStep = 0;
    m_currentAlgo = AlgoType::None;

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
    auto& nodes = m_graph->getNodes();
    for(int r = 0; r < ROWS; r++)
        for(int c = 0; c < COLS; c++)
            for(int d = 0; d < 4; d++) {
                if(m_maze[r][c].walls[d]) continue;
                int nr = r+DR[d], nc = c+DC[d];
                m_graph->addEdge(nodes[cellToIndex(r,c,COLS)-1], nodes[cellToIndex(nr,nc,COLS)-1]);
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
    std::vector<int> order = m_graph->bfs(startIdx, endIdx, nullptr, [&](int par, int child){ parent[child] = par; });
    std::vector<std::pair<int,int>> result;
    for(int idx : order) result.emplace_back(indexToRow(idx, COLS), indexToCol(idx, COLS));
    reconstructPath(endIdx, COLS, parent, outPath);
    return result;
}

std::vector<std::pair<int,int>> VisMaze::runDFS(std::vector<std::pair<int,int>>& outPath) {
    const int startIdx = cellToIndex(0, 0, COLS), endIdx = cellToIndex(ROWS-1, COLS-1, COLS);
    std::unordered_map<int,int> parent; parent[startIdx] = -1;
    std::vector<int> order = m_graph->dfs(startIdx, endIdx, nullptr, [&](int par, int child){ parent[child] = par; });
    std::vector<std::pair<int,int>> result;
    for(int idx : order) result.emplace_back(indexToRow(idx, COLS), indexToCol(idx, COLS));
    reconstructPath(endIdx, COLS, parent, outPath);
    return result;
}

void VisMaze::startAnimation(AlgoType algo) {
    m_timer->stop(); m_currentAlgo = algo;
    m_visited.assign(ROWS, std::vector<bool>(COLS, false));
    m_pathHighlight.clear(); m_animStep = 0; m_animDone = false; m_showingPath = false; m_pathStep = 0;
    if(algo == AlgoType::BFS) m_visitOrder = runBFS(m_finalPath);
    else m_visitOrder = runDFS(m_finalPath);
    m_timer->start(18);
}

void VisMaze::onAnimationTick() {
    if(!m_showingPath) {
        if(m_animStep < static_cast<int>(m_visitOrder.size())) {
            auto [r, c] = m_visitOrder[m_animStep++]; m_visited[r][c] = true; update();
        } else { m_showingPath = true; m_pathStep = 0; m_timer->setInterval(40); }
    } else {
        if(m_pathStep < static_cast<int>(m_finalPath.size())) { m_pathHighlight.push_back(m_finalPath[m_pathStep++]); update(); }
        else { m_timer->stop(); m_animDone = true; update(); }
    }
}

void VisMaze::onBfsClicked() { startAnimation(AlgoType::BFS); }
void VisMaze::onDfsClicked() { startAnimation(AlgoType::DFS); }
void VisMaze::onResetClicked() { m_timer->stop(); generateMaze(); }

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
    if(m_currentAlgo != AlgoType::None) {
        QString algoName = (m_currentAlgo == AlgoType::BFS) ? "BFS" : "DFS";
        QColor algoCol = (m_currentAlgo == AlgoType::BFS) ? QColor(80,160,255) : QColor(46,204,113);
        QFont f = p.font(); f.setPointSize(11); f.setBold(true); p.setFont(f); p.setPen(algoCol);
        QString status = m_animDone ? QString("%1 - done!  Path length: %2 steps").arg(algoName).arg(m_finalPath.size())
                                    : QString("%1 - searching for exit...").arg(algoName);
        p.drawText(QRect(ox, oy+ROWS*C+6, COLS*C, 26), Qt::AlignLeft|Qt::AlignVCenter, status);
    }
}
