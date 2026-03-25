#include "vis_tsp.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stack>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

VisTSP::VisTSP(QWidget* parent) : QWidget(parent) {
    setupUi();
    onLoadGraph(); // Load by default
}

VisTSP::~VisTSP() = default;

void VisTSP::setupUi() {
    m_toolbar = new QWidget(this);
    m_toolbar->setFixedHeight(64);
    m_toolbar->setStyleSheet("background: rgba(15, 23, 42, 0.9); border-bottom: 1px solid rgba(51, 65, 85, 0.5);");

    auto createBtn = [&](const QString& text, const QString& color) {
        auto* btn = new QPushButton(text, m_toolbar);
        btn->setFixedSize(130, 36);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QString(
            "QPushButton { background: %1; color: white; border-radius: 6px; font-weight: 600; border: none; }"
            "QPushButton:hover { background: %1; opacity: 0.8; }"
            "QPushButton:disabled { background: #334155; color: #64748b; }"
        ).arg(color));
        return btn;
    };

    m_btnLoad = createBtn("1. Floyd-Warshall", "#8B5CF6");
    m_btnFloyd = createBtn("2. Generate Kn", "#EC4899");
    m_btnKn = createBtn("3. Calculate MST", "#10B981");
    m_btnMST = createBtn("4. TSP Circuit", "#F59E0B");
    m_btnTSP = createBtn("Reset", "#64748B");
    m_btnReset = m_btnTSP; // Reset is now the 5th button

    m_statusLabel = new QLabel("Ready", m_toolbar);
    m_statusLabel->setStyleSheet("color: #E2E8F0; font-weight: 500; font-size: 13px;");

    auto* layout = new QHBoxLayout(m_toolbar);
    layout->setContentsMargins(20, 0, 20, 0);
    layout->setSpacing(10);
    layout->addWidget(m_btnLoad);
    layout->addWidget(m_btnFloyd);
    layout->addWidget(m_btnKn);
    layout->addWidget(m_btnMST);
    layout->addWidget(m_btnTSP);
    layout->addSpacing(20);
    layout->addStretch();
    layout->addWidget(m_statusLabel);

    connect(m_btnLoad, &QPushButton::clicked, this, &VisTSP::onFloydWarshall);
    connect(m_btnFloyd, &QPushButton::clicked, this, &VisTSP::onGenerateKn);
    connect(m_btnKn, &QPushButton::clicked, this, &VisTSP::onCalculateMST);
    connect(m_btnMST, &QPushButton::clicked, this, &VisTSP::onGenerateTSP);
    connect(m_btnTSP, &QPushButton::clicked, this, &VisTSP::onReset);

    // Floyd-Warshall is available immediately since graph loads automatically
    // Other buttons enabled progressively through the workflow
    m_btnFloyd->setEnabled(false);
    m_btnKn->setEnabled(false);
    m_btnMST->setEnabled(false);
}

void VisTSP::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    m_toolbar->setFixedWidth(width());
}

bool VisTSP::loadFromFile(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    m_graph.clear();
    QTextStream in(&file);
    int numNodes = 0;
    in >> numNodes;
    if (in.status() != QTextStream::Ok) return false;

    for (int i = 0; i < numNodes; ++i) {
        QString name;
        double x, y;
        in >> name >> x >> y;
        m_graph.addNode(QPoint(static_cast<int>(x), static_cast<int>(y)));
        m_graph.getNodes().back().setName(name);
    }

    int numEdges = 0;
    in >> numEdges;
    if (in.status() != QTextStream::Ok) return true;
    
    std::vector<Node*> nodePtrs;
    for (auto& n : m_graph.getNodes()) nodePtrs.push_back(&n);

    for (int i = 0; i < numEdges; ++i) {
        int u, v, w;
        in >> u >> v >> w;
        if (in.status() != QTextStream::Ok) break;
        if (u >= 1 && u <= (int)nodePtrs.size() && v >= 1 && v <= (int)nodePtrs.size()) {
            m_graph.addEdge(*nodePtrs[u-1], *nodePtrs[v-1], w);
        }
    }

    return true;
}

void VisTSP::onLoadGraph() {
    if (loadFromFile("romania-cities.txt")) {
        m_graphLoaded = true;
        m_displayMode = DisplayMode::InitialGraph;
        m_btnLoad->setEnabled(true);  // Enable Floyd-Warshall button
        m_statusLabel->setText(QString("Loaded %1 cities from Romania").arg(m_graph.getNodes().size()));
        update();
    } else {
        m_statusLabel->setText("Error: Could not load romania-cities.txt");
    }
}

void VisTSP::onFloydWarshall() {
    int n = m_graph.getNodes().size();
    m_distMatrix.assign(n, std::vector<int>(n, 1e9));
    for (int i = 0; i < n; ++i) m_distMatrix[i][i] = 0;
    
    std::vector<Node*> nodePtrs;
    for (auto& node : m_graph.getNodes()) nodePtrs.push_back(&node);
    
    for (const auto& e : m_graph.getEdges()) {
        int u = -1, v = -1;
        for(int i=0; i<n; ++i) {
            if (nodePtrs[i]->getIndex() == e.getFirst().getIndex()) u = i;
            if (nodePtrs[i]->getIndex() == e.getSecond().getIndex()) v = i;
        }
        if (u != -1 && v != -1) {
            m_distMatrix[u][v] = e.getCost();
            m_distMatrix[v][u] = e.getCost();
        }
    }

    for (int k = 0; k < n; ++k) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (m_distMatrix[i][k] + m_distMatrix[k][j] < m_distMatrix[i][j]) {
                    m_distMatrix[i][j] = m_distMatrix[i][k] + m_distMatrix[k][j];
                }
            }
        }
    }

    m_floydDone = true;
    m_btnFloyd->setEnabled(true);  // Enable Generate Kn button
    m_statusLabel->setText("Floyd-Warshall complete: Shortest paths calculated");
    update();
}

void VisTSP::onGenerateKn() {
    m_knGenerated = true;
    m_displayMode = DisplayMode::CompleteGraph;
    m_btnKn->setEnabled(true);  // Enable Calculate MST button
    int n = m_graph.getNodes().size();
    m_statusLabel->setText(QString("Complete Graph Kn generated (%1 Edges)").arg(n * (n - 1) / 2));
    update();
}


void VisTSP::onCalculateMST() {
    KruskalSolver solver;
    // We need a complete graph for TSP, not the initial graph.
    // Let's build a temporary complete graph based on distMatrix.
    UndirectedGraph kn;
    std::vector<Node*> nodePtrs;
    for (auto& node : m_graph.getNodes()) {
        kn.addNode(node.getPos());
    }
    
    std::vector<Node*> knPtrs;
    for (auto& node : kn.getNodes()) knPtrs.push_back(&node);
    
    int n = knPtrs.size();
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (m_distMatrix[i][j] < 1e9) {
                kn.addEdge(*knPtrs[i], *knPtrs[j], m_distMatrix[i][j]);
            }
        }
    }

    m_mstSteps = solver.solve(kn);
    
    m_mstDone = true;
    m_displayMode = DisplayMode::MST;
    m_btnMST->setEnabled(true);  // Enable TSP Circuit button
    
    int mstEdgeCount = 0;
    for(const auto& s : m_mstSteps) if(s.accepted) mstEdgeCount++;

    m_statusLabel->setText(QString("MST calculated using KruskalSolver (%1 edges)").arg(mstEdgeCount));
    update();
}

void VisTSP::dfsPreorder(int u, std::vector<bool>& visited, 
                         const std::vector<std::vector<int>>& adj, 
                         std::vector<int>& circuit) {
    visited[u] = true;
    circuit.push_back(u + 1);
    for (int v : adj[u]) {
        if (!visited[v]) {
            dfsPreorder(v, visited, adj, circuit);
        }
    }
}

void VisTSP::onGenerateTSP() {
    int n = m_graph.getNodes().size();
    if (n == 0) return;

    std::vector<std::vector<int>> adj(n);
    std::vector<Node*> nodePtrs;
    for (auto& node : m_graph.getNodes()) nodePtrs.push_back(&node);

    for (const auto& step : m_mstSteps) {
        if (step.accepted) {
            int u = -1, v = -1;
            for(int i=0; i<n; ++i) {
                if (nodePtrs[i]->getIndex() == step.fromIndex) u = i;
                if (nodePtrs[i]->getIndex() == step.toIndex) v = i;
            }
            if (u != -1 && v != -1) {
                adj[u].push_back(v);
                adj[v].push_back(u);
            }
        }
    }

    std::vector<bool> visited(n, false);
    m_tspCircuit.clear();
    
    // DFS Preorder
    std::stack<int> s;
    s.push(0);
    while(!s.empty()) {
        int u = s.top();
        s.pop();
        if(!visited[u]) {
            visited[u] = true;
            m_tspCircuit.push_back(nodePtrs[u]->getIndex());
            for(int v : adj[u]) if(!visited[v]) s.push(v);
        }
    }
    
    if(!m_tspCircuit.empty())
        m_tspCircuit.push_back(m_tspCircuit[0]); // Return to start

    m_tspDone = true;
    m_displayMode = DisplayMode::TSPCircuit;

    int totalCost = 0;
    for (size_t i = 0; i < m_tspCircuit.size() - 1; ++i) {
        // Find indices in nodePtrs to use distMatrix
        int idx1 = -1, idx2 = -1;
        for(int j=0; j<n; ++j) {
            if(nodePtrs[j]->getIndex() == m_tspCircuit[i]) idx1 = j;
            if(nodePtrs[j]->getIndex() == m_tspCircuit[i+1]) idx2 = j;
        }
        if(idx1 != -1 && idx2 != -1)
            totalCost += m_distMatrix[idx1][idx2];
    }

    m_statusLabel->setText(QString("TSP Circuit Complete! Total Cost: %1 km").arg(totalCost));
    update();
}

void VisTSP::onReset() {
    m_graph.clear();

    m_distMatrix.clear();
    m_tspCircuit.clear();
    m_mstSteps.clear();
    m_graphLoaded = false;
    m_floydDone = false;
    m_knGenerated = false;
    m_mstDone = false;
    m_tspDone = false;
    m_btnLoad->setEnabled(true);   // Floyd-Warshall
    m_btnFloyd->setEnabled(false);
    m_btnKn->setEnabled(false);
    m_btnMST->setEnabled(false);
    m_displayMode = DisplayMode::InitialGraph;
    m_statusLabel->setText("Reset complete. Click '1. Floyd-Warshall' to start.");
    update();
}

void VisTSP::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor(8, 10, 20)); // Deep background

    if (!m_graphLoaded) return;

    // Draw background subtle grid
    p.setPen(QColor(30, 41, 59, 50));
    for (int i = 0; i < width(); i += 50) p.drawLine(i, 0, i, height());
    for (int i = 0; i < height(); i += 50) p.drawLine(0, i, width(), i);

    // Calculate available canvas area (below toolbar)
    const int toolbarHeight = 64;
    const int margin = 60; // Margin around the graph
    const int availableWidth = width() - 2 * margin;
    const int availableHeight = height() - toolbarHeight - 2 * margin;
    const int canvasTop = toolbarHeight + margin;

    // Find graph bounds
    auto nodes = m_graph.getNodes();
    if (nodes.empty()) return;

    int minX = std::numeric_limits<int>::max(), maxX = std::numeric_limits<int>::min();
    int minY = std::numeric_limits<int>::max(), maxY = std::numeric_limits<int>::min();
    for (const auto& node : nodes) {
        QPoint pos = node.getCoord();
        minX = std::min(minX, pos.x());
        maxX = std::max(maxX, pos.x());
        minY = std::min(minY, pos.y());
        maxY = std::max(maxY, pos.y());
    }

    // Add padding to bounds
    const int nodeRadius = 22;
    minX -= nodeRadius; maxX += nodeRadius;
    minY -= nodeRadius; maxY += nodeRadius;

    int graphWidth = maxX - minX;
    int graphHeight = maxY - minY;
    if (graphWidth == 0) graphWidth = 1;
    if (graphHeight == 0) graphHeight = 1;

    // Calculate scale to fit within available area (maintain aspect ratio)
    double scaleX = static_cast<double>(availableWidth) / graphWidth;
    double scaleY = static_cast<double>(availableHeight) / graphHeight;
    double scale = std::min(scaleX, scaleY);

    // Calculate offset to center the graph
    double scaledWidth = graphWidth * scale;
    double scaledHeight = graphHeight * scale;
    double offsetX = margin + (availableWidth - scaledWidth) / 2;
    double offsetY = canvasTop + (availableHeight - scaledHeight) / 2;

    // Create a temporary scaled graph for rendering
    UndirectedGraph scaledGraph;
    std::unordered_map<int, int> indexMapping; // old index -> new index
    
    for (const auto& node : nodes) {
        QPoint pos = node.getCoord();
        double newX = offsetX + (pos.x() - minX) * scale;
        double newY = offsetY + (pos.y() - minY) * scale;
        scaledGraph.addNode(QPoint(static_cast<int>(newX), static_cast<int>(newY)));
        scaledGraph.getNodes().back().setName(node.getName());
        indexMapping[node.getIndex()] = scaledGraph.getNodes().back().getIndex();
    }

    // Copy edges with mapped indices
    for (const auto& edge : m_graph.getEdges()) {
        auto scaledNodes = scaledGraph.getNodes();
        Node* first = nullptr;
        Node* second = nullptr;
        for (auto& n : scaledNodes) {
            if (n.getName() == edge.getFirst().getName()) first = &n;
            if (n.getName() == edge.getSecond().getName()) second = &n;
        }
        if (first && second) {
            scaledGraph.addEdge(*first, *second, edge.getCost());
        }
    }

    RenderSettings settings;
    settings.nodeRadius = 18; // Slightly larger nodes
    settings.showEdgeCosts = (m_displayMode == DisplayMode::InitialGraph);

    if (m_displayMode == DisplayMode::InitialGraph) {
        GraphRenderer::drawGraph(p, scaledGraph, settings);
    } else if (m_displayMode == DisplayMode::CompleteGraph) {
        // Draw all-to-all lines (faded pink)
        settings.edgeColor = QColor(236, 72, 153, 30);
        auto scaledNodesList = scaledGraph.getNodes();
        std::vector<Node*> nodePtrs;
        for (auto& n : scaledNodesList) nodePtrs.push_back(&n);
        
        for (size_t i = 0; i < nodePtrs.size(); ++i) {
            for (size_t j = i + 1; j < nodePtrs.size(); ++j) {
                p.setPen(QPen(settings.edgeColor, 1));
                p.drawLine(nodePtrs[i]->getCoord(), nodePtrs[j]->getCoord());
            }
        }
        for (const auto& node : scaledNodesList) GraphRenderer::drawNode(p, node, settings);
    } else if (m_displayMode == DisplayMode::MST) {
        settings.highlightColor = QColor(16, 185, 129); // Emerald
        // Remap MST steps to scaled coordinates
        std::vector<MSTStep> scaledMstSteps;
        for (const auto& step : m_mstSteps) {
            scaledMstSteps.push_back(step);
        }
        GraphRenderer::drawMST(p, scaledGraph, scaledMstSteps, settings);
    } else if (m_displayMode == DisplayMode::TSPCircuit) {
        settings.highlightColor = QColor(245, 158, 11); // Amber
        GraphRenderer::drawPath(p, scaledGraph, m_tspCircuit, settings);
    }
}
