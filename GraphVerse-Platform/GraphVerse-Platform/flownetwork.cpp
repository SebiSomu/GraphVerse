#include "flownetwork.h"
#include <algorithm>

FlowNetwork::FlowNetwork() : m_source(-1), m_sink(-1), m_currentTotalFlow(0) {}

void FlowNetwork::addNode(const QPointF& pos) {
    m_nodePositions.push_back(pos);
    int n = m_nodePositions.size();
    if (n == 0) return;
    for (auto& row : m_capacity) row.resize(n, 0);
    m_capacity.emplace_back(n, 0);
    for (auto& row : m_flow) row.resize(n, 0);
    m_flow.emplace_back(n, 0);
    for (auto& row : m_cost) row.resize(n, 0);
    m_cost.emplace_back(n, 0);
}

void FlowNetwork::addEdge(int from, int to, int capacity) {
    int u = from - 1, v = to - 1;
    if (u >= 0 && u < static_cast<int>(m_nodePositions.size()) && v >= 0 && v < static_cast<int>(m_nodePositions.size()))
        m_capacity[u][v] = capacity;
}

void FlowNetwork::setCapacity(int from, int to, int capacity) { addEdge(from, to, capacity); }

void FlowNetwork::setEdgeCost(int from, int to, int cost) {
    int u = from - 1, v = to - 1;
    if (u >= 0 && u < static_cast<int>(m_nodePositions.size()) && v >= 0 && v < static_cast<int>(m_nodePositions.size())) {
        m_cost[u][v] = cost; m_cost[v][u] = -cost;
    }
}

int FlowNetwork::getEdgeCost(int from, int to) const {
    int u = from - 1, v = to - 1;
    if (u >= 0 && u < static_cast<int>(m_cost.size()) && v >= 0 && v < static_cast<int>(m_cost.size())) return m_cost[u][v];
    return 0;
}

void FlowNetwork::clear() {
    m_nodePositions.clear(); m_capacity.clear(); m_flow.clear(); m_cost.clear();
    m_iterations.clear(); m_minCutEdges.clear(); m_source = -1; m_sink = -1; m_currentTotalFlow = 0;
}

void FlowNetwork::resetFlow() {
    int n = m_nodePositions.size();
    m_flow.assign(n, std::vector<int>(n, 0));
    m_iterations.clear(); m_minCutEdges.clear(); m_currentTotalFlow = 0;
}

void FlowNetwork::initializeAlgorithm() {
    resetFlow(); m_currentResidual = m_capacity;
    IterationState s; s.iterationNumber = 0; s.residualCapacity = m_currentResidual;
    s.pathFlow = 0; s.totalFlow = 0; s.description = "Initial state. Total flow: 0";
    m_iterations.push_back(s);
}

bool FlowNetwork::bfs(const std::vector<std::vector<int>>& rGraph, int s, int t, std::vector<int>& parent) {
    int n = m_nodePositions.size();
    std::fill(parent.begin(), parent.end(), -1);
    std::vector<bool> visited(n, false);
    std::queue<int> q; q.push(s); visited[s] = true;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v = 0; v < n; v++) {
            if (!visited[v] && rGraph[u][v] > 0) {
                if (v == t) { parent[v] = u; return true; }
                q.push(v); parent[v] = u; visited[v] = true;
            }
        }
    }
    return false;
}

bool FlowNetwork::performStep() {
    if (m_source == -1 || m_sink == -1) return false;
    int s = m_source - 1, t = m_sink - 1, n = m_nodePositions.size();
    std::vector<int> parent(n);
    if (bfs(m_currentResidual, s, t, parent)) {
        int pathFlow = INT_MAX, v = t;
        std::vector<int> path;
        while (v != s) { path.push_back(v + 1); int u = parent[v]; pathFlow = std::min(pathFlow, m_currentResidual[u][v]); v = u; }
        path.push_back(s + 1); std::reverse(path.begin(), path.end());
        v = t;
        while (v != s) { int u = parent[v]; m_currentResidual[u][v] -= pathFlow; m_currentResidual[v][u] += pathFlow; v = u; }
        m_currentTotalFlow += pathFlow;
        IterationState state; state.iterationNumber = m_iterations.size();
        state.residualCapacity = m_currentResidual; state.augmentingPath = path;
        state.pathFlow = pathFlow; state.totalFlow = m_currentTotalFlow;
        state.description = QString("Iteration %1: Augmenting path found, flow %2. Total flow: %3")
                                .arg(state.iterationNumber).arg(pathFlow).arg(m_currentTotalFlow);
        m_iterations.push_back(state);
        return true;
    }
    return false;
}

void FlowNetwork::finalizeMinCut() {
    if (m_source == -1 || m_sink == -1) return;
    int s = m_source - 1, n = m_nodePositions.size();
    std::vector<bool> visited(n, false);
    std::queue<int> q; q.push(s); visited[s] = true;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v = 0; v < n; v++)
            if (!visited[v] && m_currentResidual[u][v] > 0) { visited[v] = true; q.push(v); }
    }
    m_minCutEdges.clear();
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (visited[i] && !visited[j] && m_capacity[i][j] > 0) m_minCutEdges.insert({i+1, j+1});
}

void FlowNetwork::addFinalState() {
    IterationState state; state.iterationNumber = m_iterations.size();
    state.residualCapacity = m_currentResidual; state.augmentingPath = {};
    state.pathFlow = 0; state.totalFlow = m_currentTotalFlow;
    state.description = QString("Final result. Max Flow: %1. Min-cut highlighted in red.").arg(m_currentTotalFlow);
    m_iterations.push_back(state);
}

int FlowNetwork::runFullAlgorithm() {
    initializeAlgorithm(); while (performStep()) {} finalizeMinCut(); addFinalState(); return m_currentTotalFlow;
}

std::vector<int> FlowNetwork::findNegativeCycle() {
    int n = m_nodePositions.size();
    std::vector<int> dist(n, 0), pred(n, -1);
    int cycleNode = -1;
    for (int iter = 0; iter < n; iter++) {
        cycleNode = -1;
        for (int u = 0; u < n; u++) {
            for (int v = 0; v < n; v++) {
                if (m_currentResidual[u][v] <= 0) continue;
                int w = m_cost[u][v];
                if (dist[u] + w < dist[v]) { dist[v] = dist[u] + w; pred[v] = u; if (iter == n-1) cycleNode = v; }
            }
        }
    }
    if (cycleNode == -1) return {};
    int x = cycleNode;
    for (int i = 0; i < n; i++) { if (pred[x] == -1) return {}; x = pred[x]; }
    std::vector<int> cycle;
    std::vector<bool> visited(n, false);
    int cur = x;
    while (!visited[cur]) { visited[cur] = true; cycle.push_back(cur + 1); cur = pred[cur]; if (cur == -1) return {}; }
    cycle.push_back(cur + 1);
    std::reverse(cycle.begin(), cycle.end());
    return cycle;
}

int FlowNetwork::eliminateNegativeCycles() {
    int n = m_nodePositions.size();
    if (n == 0) return 0;
    bool hasCosts = false;
    for (int i = 0; i < n && !hasCosts; i++) for (int j = 0; j < n && !hasCosts; j++) if (m_cost[i][j] != 0) hasCosts = true;
    if (!hasCosts) return 0;
    if (static_cast<int>(m_currentResidual.size()) != n) m_currentResidual = m_capacity;
    int eliminated = 0;
    for (int guard = 0; guard < 1000; guard++) {
        std::vector<int> cycle = findNegativeCycle();
        if (cycle.empty()) break;
        int r = INT_MAX;
        for (int i = 0; i+1 < static_cast<int>(cycle.size()); i++) { int u = cycle[i]-1, v = cycle[i+1]-1; r = std::min(r, m_currentResidual[u][v]); }
        if (r <= 0 || r == INT_MAX) break;
        for (int i = 0; i+1 < static_cast<int>(cycle.size()); i++) { int u = cycle[i]-1, v = cycle[i+1]-1; m_currentResidual[u][v] -= r; m_currentResidual[v][u] += r; }
        eliminated++;
    }
    if (eliminated > 0) {
        IterationState state; state.iterationNumber = m_iterations.size();
        state.residualCapacity = m_currentResidual; state.pathFlow = 0; state.totalFlow = m_currentTotalFlow;
        state.description = QString("Eliminated %1 negative cycle(s). Residual graph updated.").arg(eliminated);
        m_iterations.push_back(state);
    }
    return eliminated;
}

int FlowNetwork::getNumNodes() const { return m_nodePositions.size(); }
const std::vector<QPointF>& FlowNetwork::getNodePositions() const { return m_nodePositions; }
int FlowNetwork::getCapacity(int from, int to) const {
    int u = from-1, v = to-1;
    if (u >= 0 && u < static_cast<int>(m_capacity.size()) && v >= 0 && v < static_cast<int>(m_capacity.size())) return m_capacity[u][v];
    return 0;
}
int FlowNetwork::getResidual(int from, int to) const {
    int u = from-1, v = to-1;
    if (u >= 0 && u < static_cast<int>(m_currentResidual.size()) && v >= 0 && v < static_cast<int>(m_currentResidual.size())) return m_currentResidual[u][v];
    return 0;
}
int FlowNetwork::getFlow(int from, int to) const {
    int u = from-1, v = to-1;
    if (u < 0 || v < 0 || u >= static_cast<int>(m_capacity.size()) || v >= static_cast<int>(m_capacity.size())) return 0;
    if (m_capacity[u][v] > 0) return m_capacity[u][v] - m_currentResidual[u][v];
    return 0;
}
void FlowNetwork::setSource(int s) { m_source = s; }
void FlowNetwork::setSink(int t) { m_sink = t; }
int FlowNetwork::getSource() const { return m_source; }
int FlowNetwork::getSink() const { return m_sink; }
const std::vector<IterationState>& FlowNetwork::getIterations() const { return m_iterations; }
const std::set<std::pair<int,int>>& FlowNetwork::getMinCutEdges() const { return m_minCutEdges; }
bool FlowNetwork::isMinCutEdge(int from, int to) const { return m_minCutEdges.find({from, to}) != m_minCutEdges.end(); }
