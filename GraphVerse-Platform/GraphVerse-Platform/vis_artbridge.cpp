#include "vis_artbridge.h"
#include "undirectedgraph.h"
#include "rendering/mutable_renderer_factory.h"
#include <QPainter>
#include <QPainterPath>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QTimerEvent>
#include <QTime>
#include <cmath>

VisArtBridge::VisArtBridge(QWidget* parent)
	: QWidget(parent), m_timer(new QTimer(this))
{
	setMinimumSize(700, 500);
	m_graph = std::make_unique<UndirectedGraph>();
	m_renderer = MutableRendererFactory::createRenderer();
	m_renderSettings.showDiscLowLabels = true;
	setupUi();
	connect(m_timer, &QTimer::timeout, this, &VisArtBridge::onAnimTick);
	m_pulseTimer = startTimer(16); // 60fps pulse animation
}

VisArtBridge::~VisArtBridge() = default;

void VisArtBridge::setupUi() {
	// ── Top bar ──────────────────────────────────────────────
	m_btnRun = new QPushButton("▶  Find Articulation Points & Bridges", this);
	m_btnRun->setGeometry(10, 10, 310, 36);
	m_btnRun->setCursor(Qt::PointingHandCursor);
	m_btnRun->setStyleSheet(
		"QPushButton { background: rgba(99,102,241,0.25); color: #c7d2fe;"
		" border: 1px solid rgba(99,102,241,0.6); border-radius: 6px;"
		" font: 700 11px 'Segoe UI'; padding: 0 14px; }"
		"QPushButton:hover { background: rgba(99,102,241,0.45); }"
		"QPushButton:disabled { background: rgba(50,50,80,0.3); color: #475569; border-color: #334155; }"
	);
	connect(m_btnRun, &QPushButton::clicked, this, &VisArtBridge::onRun);

	m_btnReset = new QPushButton("↺  Reset", this);
	m_btnReset->setGeometry(328, 10, 90, 36);
	m_btnReset->setCursor(Qt::PointingHandCursor);
	m_btnReset->setStyleSheet(
		"QPushButton { background: rgba(71,85,105,0.25); color: #94a3b8;"
		" border: 1px solid rgba(71,85,105,0.5); border-radius: 6px;"
		" font: 600 11px 'Segoe UI'; }"
		"QPushButton:hover { background: rgba(71,85,105,0.45); color: #e2e8f0; }"
	);
	connect(m_btnReset, &QPushButton::clicked, this, &VisArtBridge::onReset);

	m_instrLabel = new QLabel(
		"Right-click → add node   |   Left-click two nodes → add edge   |   Drag → move node",
		this);
	m_instrLabel->setGeometry(428, 10, 600, 36);
	m_instrLabel->setStyleSheet("color: #64748b; font: 400 10px 'Segoe UI';");

	// ── Step label (bottom-left) ──────────────────────────────
	m_stepLabel = new QLabel("", this);
	m_stepLabel->setStyleSheet(
		"color: #f59e0b; font: 600 11px 'Segoe UI';"
		"background: rgba(8,10,20,0.8); border-radius: 4px; padding: 4px 8px;");
	m_stepLabel->setWordWrap(true);
	m_stepLabel->hide();

	// ── Result panel (bottom-right) ──────────────────────────
	m_resultLabel = new QLabel("", this);
	m_resultLabel->setStyleSheet(
		"color: #e2e8f0; font: 500 11px 'Segoe UI';"
		"background: rgba(8,10,20,0.88); border: 1px solid rgba(99,102,241,0.2);"
		"border-radius: 6px; padding: 10px 14px;");
	m_resultLabel->setWordWrap(true);
	m_resultLabel->hide();
}

void VisArtBridge::resetState() {
	m_timer->stop();
	m_animating = false;
	m_animStep = 0;
	m_steps.clear();
	m_result = {};
	m_animState = AnimationState(); 
	m_stepLabel->hide();
	m_resultLabel->hide();
	m_btnRun->setEnabled(true);
	update();
}

void VisArtBridge::onReset() {
	resetState();
	m_graph = std::make_unique<UndirectedGraph>();
	m_firstNode = nullptr;
	m_draggedNode = nullptr;
	m_dragging = false;
	m_hasMoved = false;
	update();
}

void VisArtBridge::onRun() {
	if (!m_graph || m_graph->getNodes().empty()) return;
	if (m_animating) return;

	resetState();

	m_result = ArticulationBridgeSolver::solve(*m_graph, &m_steps);

	m_animStep = 0;
	m_animating = true;
	m_animState.isAnimating = true;
	m_animState.showResult = false;
	m_btnRun->setEnabled(false);
	m_stepLabel->show();
	m_stepLabel->setText("Pass 1/1: Running DFS...");
	m_timer->start(m_tickInterval);
}

// ─────────────────────────────────────────────────────────────
//  Animation tick
// ─────────────────────────────────────────────────────────────
void VisArtBridge::onAnimTick() {
	if (m_animStep >= m_steps.size()) {
		m_timer->stop();
		m_animating = false;
		m_animState.isAnimating = false;
		m_animState.showResult = true;
		m_animState.activeNode = -1;
		m_animState.activeNode2 = -1;
		m_animState.activeEdge = {-1, -1};
		m_animState.articulationPoints = m_result.articulationPoints;
		m_animState.bridges = m_result.bridges;
		m_stepLabel->hide();
		updateInfoPanel();
		m_btnRun->setEnabled(true);
		update();
		return;
	}

	const auto& step = m_steps[m_animStep++];

	switch (step.type) {
	case ArtBridgeStepType::NodeEnter:
		m_animState.visitedNodes.insert(step.nodeId);
		m_animState.discValues[step.nodeId] = step.discVal;
		m_animState.lowValues[step.nodeId] = step.lowVal;
		m_animState.activeNode = step.nodeId;
		m_animState.activeNode2 = -1;
		m_animState.activeEdge = {-1, -1};
		m_stepLabel->setText(
			QString("Enter node %1 — disc=%2, low=%3")
			.arg(step.nodeId).arg(step.discVal).arg(step.lowVal));
		break;

	case ArtBridgeStepType::EdgeTree:
		m_animState.activeNode = step.nodeId;
		m_animState.activeNode2 = step.nodeId2;
		m_animState.activeEdge = {step.nodeId, step.nodeId2};
		m_stepLabel->setText(
			QString("Tree edge: %1 → %2").arg(step.nodeId).arg(step.nodeId2));
		break;

	case ArtBridgeStepType::EdgeBack:
		m_animState.lowValues[step.nodeId] = step.lowVal;
		m_animState.activeNode = step.nodeId;
		m_animState.activeNode2 = step.nodeId2;
		m_animState.activeEdge = {step.nodeId, step.nodeId2};
		m_stepLabel->setText(
			QString("Back edge: %1 → %2 — low[%1]=%3")
			.arg(step.nodeId).arg(step.nodeId2).arg(step.lowVal));
		break;

	case ArtBridgeStepType::LowUpdate:
		m_animState.lowValues[step.nodeId] = step.lowVal;
		m_stepLabel->setText(
			QString("Update low[%1]=%2 (from child %3)")
			.arg(step.nodeId).arg(step.lowVal).arg(step.nodeId2));
		break;

	case ArtBridgeStepType::NodeExit:
		m_animState.activeNode = -1;
		m_animState.activeNode2 = -1;
		m_animState.activeEdge = {-1, -1};
		m_stepLabel->setText(
			QString("Exit node %1 — disc=%2, low=%3")
			.arg(step.nodeId).arg(step.discVal).arg(step.lowVal));
		break;

	case ArtBridgeStepType::ResultAP:
		m_animState.articulationPoints.insert(step.nodeId);
		m_stepLabel->setText(
			QString("✦ Articulation point found: node %1").arg(step.nodeId));
		break;

	case ArtBridgeStepType::ResultBridge:
		m_animState.bridges.push_back({
			std::min(step.nodeId, step.nodeId2),
			std::max(step.nodeId, step.nodeId2)
			});
		m_stepLabel->setText(
			QString("⚡ Bridge found: edge (%1 — %2)")
			.arg(step.nodeId).arg(step.nodeId2));
		break;

	case ArtBridgeStepType::Done:
		break;
	}

	update();
}

void VisArtBridge::timerEvent(QTimerEvent*) {
	m_animState.pulseValue = 0.5f + 0.5f * std::sin(static_cast<float>(QTime::currentTime().msec()) * 0.006f);
	if (m_animState.showResult) update();
}

void VisArtBridge::paintEvent(QPaintEvent*) {
	if (!m_graph || !m_renderer) return;
	
	QPainter p(this);
	
	int selectedNodeId = (m_firstNode) ? m_firstNode->getIndex() : -1;
	m_renderer->renderWithState(p, *m_graph, m_animState, m_renderSettings, selectedNodeId);
	
	if (m_animState.showResult || m_animating) {
		p.setFont(QFont("Segoe UI", 9, QFont::Bold));
		int ly = height() - 80;
		auto drawLegendItem = [&](QColor c, const QString& label, int y) {
			p.setBrush(c); p.setPen(Qt::NoPen);
			p.drawEllipse(14, y, 12, 12);
			p.setPen(QColor(148, 163, 184));
			p.setFont(QFont("Segoe UI", 9));
			p.drawText(32, y + 10, label);
			};
		drawLegendItem(QColor(239, 68, 68), "Articulation Point", ly);
		drawLegendItem(QColor(99, 102, 241), "Visited node", ly + 18);

		p.setPen(QPen(QColor(245, 158, 11), 3));
		p.drawLine(14, ly + 36 + 6, 26, ly + 36 + 6);
		p.setPen(QColor(148, 163, 184));
		p.setFont(QFont("Segoe UI", 9));
		p.drawText(32, ly + 36 + 10, "Bridge");
	}

	if (m_stepLabel->isVisible()) {
		m_stepLabel->adjustSize();
		m_stepLabel->move(10, height() - m_stepLabel->height() - 10);
	}
}

void VisArtBridge::updateInfoPanel() {
	const int& nAP = static_cast<int>(m_result.articulationPoints.size());
	const int& nBr = static_cast<int>(m_result.bridges.size());

	QString text;
	if (m_result.isBiconnected) {
		text = "<b style='color:#10b981;'>✔ Graph is biconnected!</b><br>"
			"<span style='color:#64748b;'>No articulation points or bridges.<br>"
			"Removing any single node or edge<br>keeps the graph connected.</span>";
	}
	else {
		text = QString("<b style='color:#6366f1;'>Results</b><br>");

		if (nAP > 0) {
			text += QString("<br><span style='color:#ef4444;'>✦ Articulation Points (%1):</span><br>").arg(nAP);
			QStringList apList;
			for (int id : m_result.articulationPoints)
				apList << QString::number(id);
			text += "  Nodes: " + apList.join(", ");
		}
		else {
			text += "<br><span style='color:#10b981;'>✔ No articulation points</span>";
		}

		if (nBr > 0) {
			text += QString("<br><br><span style='color:#f59e0b;'>⚡ Bridges (%1):</span><br>").arg(nBr);
			for (auto& [u, v] : m_result.bridges)
				text += QString("  (%1 — %2)<br>").arg(u).arg(v);
		}
		else {
			text += "<br><span style='color:#10b981;'>✔ No bridges</span>";
		}
	}

	m_resultLabel->setText(text);
	m_resultLabel->adjustSize();

	int panelW = std::max(240, m_resultLabel->sizeHint().width() + 28);
	int panelH = m_resultLabel->heightForWidth(panelW) + 20;
	m_resultLabel->setFixedWidth(panelW);
	m_resultLabel->setGeometry(width() - panelW - 12,
		height() - panelH - 12,
		panelW, panelH);
	m_resultLabel->show();
}

// ─────────────────────────────────────────────────────────────
//  Mouse interaction (identical pattern to VisComponents)
// ─────────────────────────────────────────────────────────────
void VisArtBridge::mousePressEvent(QMouseEvent* m) {
	if (!m_graph || m_animating) return;
	if (m->button() == Qt::LeftButton) {
		m_pressPosition = m->pos();
		m_hasMoved = false;
		for (auto& n : m_graph->getNodes())
			if (std::abs(m->pos().x() - n.getX()) < 12 &&
				std::abs(m->pos().y() - n.getY()) < 12) {
				m_draggedNode = &n;
				m_dragging = true;
				break;
			}
	}
}

void VisArtBridge::mouseReleaseEvent(QMouseEvent* m) {
	if (!m_graph || m_animating) return;

	if (m->button() == Qt::LeftButton && m_dragging) {
		m_dragging = false;
		m_draggedNode = nullptr;
		if (m_hasMoved) { m_hasMoved = false; return; }
	}

	if (m->button() == Qt::RightButton) {
		// Add node (only if not overlapping existing)
		bool overlap = false;
		for (const auto& n : m_graph->getNodes()) {
			int dx = m->pos().x() - n.getX(), dy = m->pos().y() - n.getY();
			if (std::sqrt(dx * dx + dy * dy) < 28) { overlap = true; break; }
		}
		if (!overlap) {
			m_graph->addNode(m->pos());
			resetState();
			update();
		}
		return;
	}

	if (m->button() == Qt::LeftButton) {
		auto& nodes = m_graph->getNodes();
		Node* selected = nullptr;
		for (auto& n : nodes)
			if (std::abs(m->pos().x() - n.getX()) < 14 &&
				std::abs(m->pos().y() - n.getY()) < 14) {
				selected = &n; break;
			}

		if (selected) {
			if (m_firstNode) {
				if (m_firstNode->getIndex() == selected->getIndex()) {
					m_firstNode = nullptr; return;
				}
				// Check duplicate edge
				bool exists = false;
				for (const auto& e : m_graph->getEdges()) {
					if ((e.getFirst().getIndex() == m_firstNode->getIndex() &&
						e.getSecond().getIndex() == selected->getIndex()) ||
						(e.getFirst().getIndex() == selected->getIndex() &&
							e.getSecond().getIndex() == m_firstNode->getIndex()))
					{
						exists = true; break;
					}
				}
				if (!exists) {
					m_graph->addEdge(*m_firstNode, *selected);
					resetState();
				}
				m_firstNode = nullptr;
				update();
			}
			else {
				m_firstNode = selected;
				update();
			}
		}
	}
}

void VisArtBridge::mouseMoveEvent(QMouseEvent* m) {
	if (!m_graph || !m_dragging || !m_draggedNode || m_animating) return;
	m_hasMoved = true;
	m_draggedNode->setCoord(m->pos());
	resetState();
	update();
}

// ─────────────────────────────────────────────────────────────
//  Hide — full reset
// ─────────────────────────────────────────────────────────────
void VisArtBridge::hideEvent(QHideEvent* event) {
	onReset();
	QWidget::hideEvent(event);
}
