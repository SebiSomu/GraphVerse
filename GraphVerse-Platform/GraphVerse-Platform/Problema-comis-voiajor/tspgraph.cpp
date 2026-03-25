#include "tspgraph.h"
#include <cmath>
#include <QDebug>
#include <stack>

const int INF = std::numeric_limits<int>::max() / 2;

TSPGraph::TSPGraph() {}

int TSPGraph::find(int x) {
    int root = x;
    while (m_parent[root] != root) {
        root = m_parent[root];
    }
    
    int current = x;
    while (current != root) {
        int next = m_parent[current];
        m_parent[current] = root;
        current = next;
    }
    
    return root;
}

void TSPGraph::unite(int x, int y) {
    int rootX = find(x);
    int rootY = find(y);
    
    if (rootX != rootY) {
        if (m_rank[rootX] < m_rank[rootY]) {
            m_parent[rootX] = rootY;
        } else if (m_rank[rootX] > m_rank[rootY]) {
            m_parent[rootY] = rootX;
        } else {
            m_parent[rootY] = rootX;
            m_rank[rootX]++;
        }
    }
}

void TSPGraph::dfsPreorder(int startNode, std::vector<bool>& visited, 
                            const std::vector<std::vector<int>>& mstAdj) {
    std::stack<int> s;
    s.push(startNode);
    
    while (!s.empty()) {
        int node = s.top();
        s.pop();
        
        if (!visited[node]) {
            visited[node] = true;
            m_tspCircuit.push_back(node + 1);
            
            const auto& neighbors = mstAdj[node];
            for (auto it = neighbors.rbegin(); it != neighbors.rend(); it++)
                if (!visited[*it])
                    s.push(*it);
        }
    }
}

bool TSPGraph::loadFromFile(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open file:" << filename;
        return false;
    }
    
    QTextStream in(&file);
    
    int n;
    in >> n;
    
    for (int i = 0; i < n; i++) {
        QString name;
        double x, y;
        in >> name >> x >> y;
        
        Node node(i + 1, QPointF(x, y), name);
        m_nodes.push_back(node);
    }
    
    m_distMatrix.resize(n, std::vector<int>(n, INF));
    for (int i = 0; i < n; i++) {
        m_distMatrix[i][i] = 0;
    }
    
    int edgeCount;
    in >> edgeCount;
    
    for (int i = 0; i < edgeCount; i++) {
        int u, v, cost;
        in >> u >> v >> cost;
        u--;
        v--;
        
        m_distMatrix[u][v] = cost;
        m_distMatrix[v][u] = cost;
        
        Edge edge(&m_nodes[u], &m_nodes[v], cost);
        m_edges.push_back(edge);
    }
    
    file.close();
    qDebug() << "Loaded" << n << "cities and" << edgeCount << "edges";
    return true;
}

void TSPGraph::floydWarshall() {
    int n = static_cast<int>(m_nodes.size());
    
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (m_distMatrix[i][k] + m_distMatrix[k][j] < m_distMatrix[i][j])
                    m_distMatrix[i][j] = m_distMatrix[i][k] + m_distMatrix[k][j];
            }
        }
    }
    
    qDebug() << "Floyd-Warshall completed";
}

void TSPGraph::generateCompleteGraph() {
    int n = static_cast<int>(m_nodes.size());
    
    m_edges.clear();
    
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            Edge edge(&m_nodes[i], &m_nodes[j], m_distMatrix[i][j]);
            m_edges.push_back(edge);
        }
    }
    
    qDebug() << "Generated complete graph with" << m_edges.size() << "edges";
}

void TSPGraph::kruskalMST() {
    int n = static_cast<int>(m_nodes.size());
    
    m_parent.resize(n);
    m_rank.resize(n, 0);
    for (int i = 0; i < n; i++) {
        m_parent[i] = i;
    }
    
    std::vector<Edge> sortedEdges = m_edges;
    std::sort(sortedEdges.begin(), sortedEdges.end(), 
              [](const Edge& a, const Edge& b) {
                  return a.getCost() < b.getCost();
              });
    
    m_mstEdges.clear();
    
    for (const Edge& edge : sortedEdges) {
        int u = edge.getFirst().getIndex() - 1;
        int v = edge.getSecond().getIndex() - 1;
        
        if (find(u) != find(v)) {
            m_mstEdges.push_back(edge);
            unite(u, v);
            
            if (static_cast<int>(m_mstEdges.size()) == n - 1)
                break;
        }
    }
    
    qDebug() << "Kruskal MST completed with" << m_mstEdges.size() << "edges";
}

void TSPGraph::generateTSPCircuit() {
    int n = static_cast<int>(m_nodes.size());
    
    std::vector<std::vector<int>> mstAdj(n);
    for (const Edge& edge : m_mstEdges) {
        int u = edge.getFirst().getIndex() - 1;
        int v = edge.getSecond().getIndex() - 1;
        mstAdj[u].push_back(v);
        mstAdj[v].push_back(u);
    }
    
    m_tspCircuit.clear();
    std::vector<bool> visited(n, false);
    dfsPreorder(0, visited, mstAdj);
    
    m_tspCircuit.push_back(1);
    
    int totalCost = 0;
    for (size_t i = 0; i < m_tspCircuit.size() - 1; i++) {
        int u = m_tspCircuit[i] - 1;
        int v = m_tspCircuit[i + 1] - 1;
        totalCost += m_distMatrix[u][v];
    }
    
    qDebug() << "TSP Circuit generated:";
    QString circuit;
    for (int city : m_tspCircuit) {
        circuit += QString::number(city) + " -> ";
    }
    circuit.chop(4);
    qDebug() << circuit;
    qDebug() << "Total cost:" << totalCost;
}

const std::vector<std::vector<int>>& TSPGraph::getDistMatrix() const {
    return m_distMatrix;
}

const std::vector<Edge>& TSPGraph::getMSTEdges() const {
    return m_mstEdges;
}

const std::vector<int>& TSPGraph::getTSPCircuit() const {
    return m_tspCircuit;
}

void TSPGraph::drawGraph(QPainter& p, bool showMST, bool showTSP) const {
    const double nodeRadius = 15.0;
    
    if (!showMST && !showTSP) {
        p.setPen(QPen(Qt::gray, 1));
        for (const auto& ed : m_edges) {
            QPointF start(ed.getFirst().getX(), ed.getFirst().getY());
            QPointF end(ed.getSecond().getX(), ed.getSecond().getY());
            
            QLineF line(start, end);
            double angle = std::atan2(line.dy(), line.dx());
            
            QPointF adjustedStart = start + QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));
            QPointF adjustedEnd = end - QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));
            p.drawLine(adjustedStart, adjustedEnd);
            
            QPointF midPoint = (start + end) / 2.0;
            QPointF offset(-12 * sin(angle), 12 * cos(angle));
            QPointF textPos = midPoint + offset;
            
            p.save();
            p.setPen(Qt::darkGray);
            QFont font = p.font();
            font.setPointSize(8);
            p.setFont(font);
            p.drawText(textPos, QString::number(ed.getCost()));
            p.restore();
        }
    }
    
    if (showMST && !showTSP) {
        p.setPen(QPen(Qt::blue, 3));
        for (const auto& ed : m_mstEdges) {
            QPointF start(ed.getFirst().getX(), ed.getFirst().getY());
            QPointF end(ed.getSecond().getX(), ed.getSecond().getY());
            
            QLineF line(start, end);
            double angle = std::atan2(line.dy(), line.dx());
            
            QPointF adjustedStart = start + QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));
            QPointF adjustedEnd = end - QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));
            p.drawLine(adjustedStart, adjustedEnd);
            
            QPointF midPoint = (start + end) / 2.0;
            QPointF offset(-15 * sin(angle), 15 * cos(angle));
            QPointF textPos = midPoint + offset;
            
            p.save();
            p.setPen(Qt::blue);
            QFont font = p.font();
            font.setBold(true);
            font.setPointSize(10);
            p.setFont(font);
            p.drawText(textPos, QString::number(ed.getCost()));
            p.restore();
        }
    }
    
    if (showTSP && m_tspCircuit.size() > 1) {
        p.setPen(QPen(Qt::red, 3));
        
        for (size_t i = 0; i < m_tspCircuit.size() - 1; i++) {
            int u = m_tspCircuit[i] - 1;
            int v = m_tspCircuit[i + 1] - 1;
            
            QPointF start(m_nodes[u].getX(), m_nodes[u].getY());
            QPointF end(m_nodes[v].getX(), m_nodes[v].getY());
            
            QLineF line(start, end);
            double angle = std::atan2(line.dy(), line.dx());
            
            QPointF adjustedStart = start + QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));
            QPointF adjustedEnd = end - QPointF(nodeRadius * cos(angle), nodeRadius * sin(angle));
            p.drawLine(adjustedStart, adjustedEnd);
            
            QPointF midPoint = (start + end) / 2.0;
            QPointF offset(-15 * sin(angle), 15 * cos(angle));
            QPointF textPos = midPoint + offset;

            p.save();
            p.setPen(Qt::red);
            QFont font = p.font();
            font.setBold(true);
            font.setPointSize(10);
            p.setFont(font);
            p.drawText(textPos, QString::number(i + 1));
            p.restore();
        }
    }
    
    for (const auto& n : m_nodes) {
        QRect r(n.getX() - nodeRadius, n.getY() - nodeRadius, nodeRadius * 2, nodeRadius * 2);
        
        p.setBrush(Qt::white);
        p.setPen(QPen(Qt::black, 2));
        p.drawEllipse(r);
        
        QString indexStr = QString::number(n.getIndex());
        p.drawText(r, Qt::AlignCenter, indexStr);
        
        p.save();
        QFont font = p.font();
        font.setPointSize(9);
        font.setBold(true);
        p.setFont(font);
        p.setPen(Qt::darkBlue);
        QRect nameRect(n.getX() - 50, n.getY() + nodeRadius + 2, 100, 20);
        p.drawText(nameRect, Qt::AlignCenter, n.getName());
        p.restore();
    }
}
