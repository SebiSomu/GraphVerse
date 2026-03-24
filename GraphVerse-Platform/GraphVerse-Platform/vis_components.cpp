#include "vis_components.h"
#include "undirectedgraph.h"
#include "directedgraph.h"
#include "algorithms/connected_components_solver.h"
#include "algorithms/kosaraju_solver.h"
#include "rendering/graph_renderer_factory.h"
#include "rendering/component_colorizer.h"
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <cmath>

VisComponents::VisComponents(QWidget* parent)
    : QWidget(parent), m_graph(nullptr), m_firstNode(nullptr), m_draggedNode(nullptr)
{
    setMinimumSize(800, 600);
    m_graph = std::make_unique<UndirectedGraph>();

    m_checkBox = new QCheckBox("Directed", this);
    m_checkBox->setStyleSheet("color: white; font-size: 12px;");
    connect(m_checkBox, &QCheckBox::checkStateChanged, this, &VisComponents::onCheckBoxStateChanged);

    m_btnComponents = new QPushButton("Find connected components", this);
    m_btnComponents->setGeometry(10, 50, 250, 30);
    m_btnComponents->setStyleSheet("QPushButton{border-radius:6px;font-weight:bold;font-size:12px;background:#1e90ff;color:white;}"
                                   "QPushButton:hover{background:#1070d0;}");
    connect(m_btnComponents, &QPushButton::clicked, this, &VisComponents::onFindComponents);

    m_btnToggleCondensed = new QPushButton("Show condensed graph", this);
    m_btnToggleCondensed->setGeometry(270, 50, 250, 30);
    m_btnToggleCondensed->setEnabled(false);
    m_btnToggleCondensed->setStyleSheet("QPushButton{border-radius:6px;font-weight:bold;font-size:12px;background:#9b59b6;color:white;}"
                                        "QPushButton:hover{background:#7d3c98;}"
                                        "QPushButton:disabled{background:#555;color:#999;}");
    connect(m_btnToggleCondensed, &QPushButton::clicked, this, &VisComponents::onToggleCondensedView);

    m_checkBox->setGeometry(10, 15, 120, 25);
    updateButtonText();
}

VisComponents::~VisComponents() = default;

void VisComponents::onCheckBoxStateChanged(int state) {
    recreateGraph(state == Qt::Checked);
    updateButtonText(); update();
}

void VisComponents::onFindComponents() {
    if(!m_graph) return;
    std::unordered_map<int, int> colors;
    int count = 0;
    if(auto* ug = dynamic_cast<UndirectedGraph*>(m_graph.get())) {
        count = ConnectedComponentsSolver{}.solve(*m_graph, colors);
        m_graph->setComponentData(count, colors);
        QMessageBox::information(this, "Connected Components",
            QString("The graph has %1 connected components!").arg(count));
        m_btnToggleCondensed->setEnabled(false);
    } else if(auto* dg = dynamic_cast<DirectedGraph*>(m_graph.get())) {
        count = KosarajuSolver{}.solve(*dg, colors);
        m_graph->setComponentData(count, colors);
        QMessageBox::information(this, "Strongly Connected Components",
            QString("The graph has %1 strongly connected components!").arg(count));
        dg->buildCondensedGraph();
        m_btnToggleCondensed->setEnabled(true);
        m_btnToggleCondensed->setText("Show condensed graph");
    }
    update();
}

void VisComponents::onToggleCondensedView() {
    DirectedGraph* dg = dynamic_cast<DirectedGraph*>(m_graph.get());
    if(dg && dg->getNumComponents() > 0) {
        dg->toggleCondensedGraph();
        m_btnToggleCondensed->setText(dg->isShowingCondensedGraph() ? "Show normal graph" : "Show condensed graph");
        update();
    }
}

void VisComponents::updateButtonText() {
    if(dynamic_cast<UndirectedGraph*>(m_graph.get())) {
        m_btnComponents->setText("Find connected components");
        m_btnToggleCondensed->setEnabled(false);
    } else if(auto* dg = dynamic_cast<DirectedGraph*>(m_graph.get())) {
        m_btnComponents->setText("Find strongly connected components");
        m_btnToggleCondensed->setEnabled(m_graph->getNumComponents() > 0);
    }
}

void VisComponents::recreateGraph(bool directed) {
    if (!m_graph) return;
    
    struct EdgeInfo { int from; int to; int cost; };
    std::vector<std::pair<QPoint, int>> nodeInfos;
    for (const auto& n : m_graph->getNodes()) {
        nodeInfos.push_back({QPoint(n.getX(), n.getY()), n.getIndex()});
    }
    
    std::vector<EdgeInfo> edgeInfos;
    for (const auto& ed : m_graph->getEdges()) {
        edgeInfos.push_back({ed.getFirst().getIndex(), ed.getSecond().getIndex(), ed.getCost()});
    }

    if (directed) {
        m_graph = std::make_unique<DirectedGraph>();
    } else {
        m_graph = std::make_unique<UndirectedGraph>();
    }
    
    for (const auto& info : nodeInfos) {
        m_graph->addNode(info.first);
        m_graph->getNodes().back().setIndex(info.second);
    }
    
    auto& newNodes = m_graph->getNodes();
    for (const auto& info : edgeInfos) {
        Node* first = nullptr; Node* second = nullptr;
        for (auto& n : newNodes) {
            if (n.getIndex() == info.from) first = &n;
            if (n.getIndex() == info.to) second = &n;
        }
        if (first && second) m_graph->addEdge(*first, *second, info.cost);
    }
    
    m_firstNode = nullptr; m_draggedNode = nullptr; m_dragging = false;
    updateButtonText();
}

void VisComponents::mouseReleaseEvent(QMouseEvent* m) {
    if(!m_graph) return;
    if(m->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false; m_draggedNode = nullptr;
        if(m_hasMoved) { m_hasMoved = false; return; }
    }
    if(m->button() == Qt::RightButton) {
        bool overlayed = false;
        for(const auto& n : m_graph->getNodes()) {
            int dx = m->pos().x()-n.getX(), dy = m->pos().y()-n.getY();
            if(std::sqrt(dx*dx+dy*dy) < 25) { overlayed = true; break; }
        }
        if(!overlayed) { m_graph->addNode(m->pos()); update(); }
        return;
    } else if(m->button() == Qt::LeftButton) {

        auto& nodes = m_graph->getNodes();
        Node* selected = nullptr;
        for(auto& n : nodes)
            if(std::abs(m->pos().x()-n.getX()) < 10 && std::abs(m->pos().y()-n.getY()) < 10) { selected = &n; break; }
        if(selected) {
            if(m_firstNode) {
                if(m_firstNode->getIndex() == selected->getIndex()) { m_firstNode = nullptr; return; }
                bool exists = false;
                for(auto& ed : m_graph->getEdges()) {
                    if((ed.getFirst().getIndex()==m_firstNode->getIndex() && ed.getSecond().getIndex()==selected->getIndex()) ||
                       (!dynamic_cast<DirectedGraph*>(m_graph.get()) &&
                        ed.getFirst().getIndex()==selected->getIndex() && ed.getSecond().getIndex()==m_firstNode->getIndex()))
                    { exists = true; break; }
                }
                if(!exists) m_graph->addEdge(*m_firstNode, *selected);
                else QMessageBox::information(this, "Edge exists", "An edge already exists between these nodes!");
                m_firstNode = nullptr; update();
            } else { m_firstNode = selected; }
        }
    }
}

void VisComponents::mousePressEvent(QMouseEvent* m) {
    if(!m_graph) return;
    if(m->button() == Qt::LeftButton) {
        m_pressPosition = m->pos(); m_hasMoved = false;
        for(auto& n : m_graph->getNodes())
            if(std::abs(m->pos().x()-n.getX())<10 && std::abs(m->pos().y()-n.getY())<10) {
                m_draggedNode = &n; m_dragging = true; break;
            }
    }
}

void VisComponents::mouseMoveEvent(QMouseEvent* m) {
    if(!m_graph || !m_dragging || !m_draggedNode) return;
    m_hasMoved = true; m_draggedNode->setCoord(m->pos()); update();
}

void VisComponents::paintEvent(QPaintEvent*) {
    if(!m_graph) return;
    QPainter p(this); p.fillRect(rect(), Qt::black);
    GraphRendererFactory::createRenderer(*m_graph)->render(p, *m_graph);
}
