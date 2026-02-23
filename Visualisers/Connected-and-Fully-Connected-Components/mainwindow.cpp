#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "undirectedgraph.h"
#include "directedgraph.h"
#include <QPainter>
#include <QMessageBox>
#include <QDebug>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_graph(nullptr)
    , m_firstNode(nullptr)
    , m_draggedNode(nullptr)
    , m_dragging(false)
    , m_pressPosition(QPoint(0, 0))
{
    ui->setupUi(this);
    m_graph = new UndirectedGraph();
    connect(ui->checkBox, &QCheckBox::checkStateChanged, this, &MainWindow::onCheckBoxStateChanged);

    m_btnComponents = new QPushButton("Find connected components", this);
    m_btnComponents->setGeometry(10, 50, 250, 30);
    connect(m_btnComponents, &QPushButton::clicked, this, &MainWindow::onFindComponents);

    m_btnToggleCondensed = new QPushButton("Show condensed graph", this);
    m_btnToggleCondensed->setGeometry(10, 90, 250, 30);
    m_btnToggleCondensed->setEnabled(false);
    connect(m_btnToggleCondensed, &QPushButton::clicked, this, &MainWindow::onToggleCondensedView);

    updateButtonText();
}

MainWindow::~MainWindow()
{
    delete m_graph;
    delete ui;
}

void MainWindow::onCheckBoxStateChanged(int state)
{
    bool directed = (state == Qt::Checked);
    recreateGraph(directed);
    updateButtonText();
    update();
}

void MainWindow::onFindComponents()
{
    if(!m_graph)
        return;

    if(dynamic_cast<UndirectedGraph*>(m_graph)) {
        m_graph->findConnectedComponents();
        int numComponents = m_graph->getNumComponents();

        QMessageBox::information(this, "Connected Components",
                                 QString("The graph has %1 connected components!").arg(numComponents));
        m_btnToggleCondensed->setEnabled(false);
        update();
    } else if(dynamic_cast<DirectedGraph*>(m_graph)) {
        m_graph->findConnectedComponents();
        int numComponents = m_graph->getNumComponents();

        QMessageBox::information(this, "Strongly Connected Components",
                                 QString("The graph has %1 strongly connected components!").arg(numComponents));
        m_btnToggleCondensed->setEnabled(true);
        m_btnToggleCondensed->setText("Show normal graph");
        update();
    }
}

void MainWindow::onToggleCondensedView()
{
    DirectedGraph* dg = dynamic_cast<DirectedGraph*>(m_graph);
    if(dg && dg->getNumComponents() > 0) {
        dg->toggleCondensedGraph();
        if(dg->isShowingCondensedGraph()) {
            m_btnToggleCondensed->setText("Show normal graph");
        } else {
            m_btnToggleCondensed->setText("Show condensed graph");
        }
        update();
    }
}

void MainWindow::updateButtonText()
{
    if(dynamic_cast<UndirectedGraph*>(m_graph)) {
        m_btnComponents->setText("Find connected components");
        m_btnToggleCondensed->setEnabled(false);
    } else if(dynamic_cast<DirectedGraph*>(m_graph)) {
        m_btnComponents->setText("Find strongly connected components");
        m_btnToggleCondensed->setEnabled(m_graph->getNumComponents() > 0);
        if(m_btnToggleCondensed->isEnabled()) {
            DirectedGraph* dg = dynamic_cast<DirectedGraph*>(m_graph);
            if(dg && dg->isShowingCondensedGraph()) {
                m_btnToggleCondensed->setText("Show normal graph");
            } else {
                m_btnToggleCondensed->setText("Show condensed graph");
            }
        }
    }
}

void MainWindow::recreateGraph(bool directed)
{
    std::vector<Node> nodes = m_graph->getNodes();
    std::vector<Edge> edges = m_graph->getEdges();
    delete m_graph;
    m_graph = nullptr;

    if(directed)
        m_graph = new DirectedGraph();
    else
        m_graph = new UndirectedGraph();

    for(const auto& n : nodes) {
        m_graph->addNode(QPoint(n.getX(), n.getY()));
    }

    auto& newNodes = m_graph->getNodes();
    for(const auto& ed : edges) {
        Node* first = nullptr;
        Node* second = nullptr;

        for(auto& n : newNodes) {
            if(n.getIndex() == ed.getFirst().getIndex()) {
                first = &n;
            }
            if(n.getIndex() == ed.getSecond().getIndex()) {
                second = &n;
            }
        }

        if(first && second) {
            m_graph->addEdge(*first, *second);
        }
    }

    m_firstNode = nullptr;
    m_draggedNode = nullptr;
    m_dragging = false;
    updateButtonText();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *m)
{
    if(!m_graph)
        return;

    if (m->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        m_draggedNode = nullptr;

        if(m_hasMoved) {
            m_hasMoved = false;
            return;
        }
    }

    if(m->button() == Qt::RightButton) {
        bool overlayed = false;
        auto& nodes = m_graph->getNodes();
        QPoint pos = m->pos();

        for(const auto& n : nodes) {
            int dx = pos.x() - n.getX();
            int dy = pos.y() - n.getY();
            double dist = std::sqrt(dx * dx + dy * dy);
            if(dist < 25) {
                overlayed = true;
                break;
            }
        }

        if(!overlayed) {
            m_graph->addNode(pos);
            update();
        }
        return;
    }
    else if(m->button() == Qt::LeftButton) {
        auto& nodes = m_graph->getNodes();
        Node* selected = nullptr;
        for(auto& n : nodes) {
            if(std::abs(m->pos().x() - n.getX()) < 10 &&
                std::abs(m->pos().y() - n.getY()) < 10) {
                selected = &n;
                break;
            }
        }

        if(selected != nullptr) {
            if(m_firstNode != nullptr) {
                if(m_firstNode->getIndex() == selected->getIndex()) {
                    m_firstNode = nullptr;
                    return;
                }

                bool existingEdge = false;
                auto& edges = m_graph->getEdges();

                for(auto& ed : edges) {
                    if((ed.getFirst().getIndex() == m_firstNode->getIndex() &&
                         ed.getSecond().getIndex() == selected->getIndex()) ||
                        (!dynamic_cast<DirectedGraph*>(m_graph) &&
                         ed.getFirst().getIndex() == selected->getIndex() &&
                         ed.getSecond().getIndex() == m_firstNode->getIndex())) {
                        existingEdge = true;
                        break;
                    }
                }

                if(!existingEdge) {
                    m_graph->addEdge(*m_firstNode, *selected);
                } else {
                    QMessageBox::information(this, "Edge exists",
                                             "An edge already exists between these nodes!");
                }

                m_firstNode = nullptr;
                update();
            } else {
                m_firstNode = selected;
            }
        }
    }
}

void MainWindow::mousePressEvent(QMouseEvent *m)
{
    if (!m_graph)
        return;

    if (m->button() == Qt::LeftButton) {
        m_pressPosition = m->pos();
        m_hasMoved = false;

        for (auto& n : m_graph->getNodes()) {
            if (std::abs(m->pos().x() - n.getX()) < 10 &&
                std::abs(m->pos().y() - n.getY()) < 10) {
                m_draggedNode = &n;
                m_dragging = true;
                break;
            }
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *m)
{
    if (!m_graph || !m_dragging || !m_draggedNode)
        return;

    m_hasMoved = true;
    m_draggedNode->setCoord(m->pos());
    update();
}

void MainWindow::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    if(!m_graph)
        return;

    QPainter p(this);
    m_graph->drawEdge(p);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}
