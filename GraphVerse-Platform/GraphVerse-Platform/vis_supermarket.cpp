#include "vis_supermarket.h"
#include "Node.h"
#include "directedgraph.h"
#include "edge.h"
#include "algorithms/dijkstra_solver.h"
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
#include "GraphRenderer.h"

VisSupermarket::VisSupermarket(QWidget* parent)
	: QWidget(parent), m_graph(nullptr), m_startIdx(-1),
	m_endIdx(-1), m_finalPath(), m_pathCost(0),
	m_toolbar(nullptr), m_statusLabel(new QLabel(this)),
	m_nameInput(new QLineEdit(this)), m_mode(InteractionMode::Normal),
	m_draggedNodeIdx(-1), m_isDragging(false) {
	setupUi();
	buildHipermarketLayout();
}

VisSupermarket::~VisSupermarket() = default;

void VisSupermarket::resizeEvent(QResizeEvent* event) {
	QWidget::resizeEvent(event);
	if (m_toolbar)
		m_toolbar->setGeometry(0, 0, width(), 110);
}

void VisSupermarket::setupUi() {
	m_toolbar = new QWidget(this);
	auto* vlay = new QVBoxLayout(m_toolbar);
	vlay->setContentsMargins(24, 12, 24, 12);
	vlay->setSpacing(8);

	auto* topRow = new QHBoxLayout();
	m_nameInput->setPlaceholderText("Enter new Section Name");
	m_nameInput->setFixedWidth(200);
	m_nameInput->setStyleSheet(
		"QLineEdit { background: #1E293B; color: white; border: 1px solid "
		"#475569; border-radius: 6px; padding: 4px 8px; }");

	auto* btnAdd = new QPushButton("➕ Add Section", this);
	auto* btnRemove = new QPushButton("🗑 Remove", this);
	auto* btnFind = new QPushButton("🛒 Find Route", this);
	auto* btnReset = new QPushButton("🔄 Reset", this);

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
	m_statusLabel->setAlignment(Qt::AlignCenter);

	auto* hint = new QLabel("Click = Start · Shift+Click = End · Drag sections? "
		"(feature coming soon)",
		this);
	hint->setStyleSheet("color: #8C94BE; font-size: 11px;");
	topRow->addWidget(hint);

	vlay->addLayout(topRow);
	vlay->addWidget(m_statusLabel);
	
	m_toolbar->setGeometry(0, 0, width(), 110);

	connect(btnAdd, &QPushButton::clicked, this,
		&VisSupermarket::onAddSectionClicked);
	connect(btnRemove, &QPushButton::clicked, this,
		&VisSupermarket::onRemoveSectionClicked);
	connect(btnFind, &QPushButton::clicked, this,
		&VisSupermarket::onFindPathClicked);
	connect(btnReset, &QPushButton::clicked, this,
		&VisSupermarket::onResetClicked);
}

void VisSupermarket::updateStatus(const QString& text) {
	m_statusLabel->setText(text);
}

void VisSupermarket::buildHipermarketLayout() {
	m_graph = std::make_unique<DirectedGraph>();

	m_startIdx = -1;
	m_endIdx = -1;
	m_finalPath.clear();
	m_pathCost = 0;
	m_mode = InteractionMode::Normal;
	updateStatus("Hypermarket layout loaded. Add nodes or click to find paths.");

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
		{"Cashier 2", {960, 600}} };

	for (const auto& s : sections) {
		m_graph->addNode(s.pos);
		m_graph->getNodes().back().setName(s.name);
	}

	updateGraphEdges();
	update();
}

void VisSupermarket::updateGraphEdges() {
	if (!m_graph)
		return;

	m_graph->getEdges().clear();
	std::vector<Node*> nodes;
	for (auto& n : m_graph->getNodes()) nodes.push_back(&n);

	for (size_t i = 0; i < nodes.size(); ++i) {
		for (size_t j = i + 1; j < nodes.size(); ++j) {
			int dx = nodes[i]->getX() - nodes[j]->getX();
			int dy = nodes[i]->getY() - nodes[j]->getY();
			int dist = static_cast<int>(std::sqrt(dx * dx + dy * dy));
			if (dist < 250) {
				m_graph->addEdge(*nodes[i], *nodes[j], dist);
				m_graph->addEdge(*nodes[j], *nodes[i], dist);
			}
		}
	}
}

int VisSupermarket::nodeAt(QPoint pos) const {
	if (!m_graph || m_graph->getNodes().empty())
		return -1;

	// Use fixed bounds for store floor to keep it stable
	int minX = 50, maxX = 1000;
	int minY = 50, maxY = 650;

	double padX = 80, padY = 60, bottomPad = 120;
	double viewW = width();
	double viewH = height() - 130;
	double graphW = (maxX - minX) + padX * 2;
	double graphH = (maxY - minY) + padY + bottomPad;
	double scale = std::min(viewW / graphW, viewH / graphH);

	double offsetX = (viewW - graphW * scale) / 2.0;
	double offsetY = 130 + (viewH - graphH * scale) / 2.0;

	QPointF p = (QPointF(pos) - QPointF(offsetX, offsetY)) / scale +
		QPointF(minX - padX, minY - padY);

	for (const Node& n : m_graph->getNodes()) {
		double dx = p.x() - n.getX();
		double dy = p.y() - n.getY();
		if (std::sqrt(dx * dx + dy * dy) <= (NODE_R)+5)
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

void VisSupermarket::mousePressEvent(QMouseEvent* e) {
	QPoint rawPos = e->pos();
	if (!m_graph)
		return;

	// Static store bounds
	int minX = 50, maxX = 1000;
	int minY = 50, maxY = 650;

	double padX = 80, padY = 60, bottomPad = 120;
	double viewW = width();
	double viewH = height() - 130;
	double graphW = (maxX - minX) + padX * 2;
	double graphH = (maxY - minY) + padY + bottomPad;
	double scale = std::min(viewW / graphW, viewH / graphH);
	double offsetX = (viewW - graphW * scale) / 2.0;
	double offsetY = 130 + (viewH - graphH * scale) / 2.0;
	QPointF graphPos = (QPointF(rawPos) - QPointF(offsetX, offsetY)) / scale +
		QPointF(minX - padX, minY - padY);

	int hit = nodeAt(rawPos);

	if (m_mode == InteractionMode::Adding) {
		QString name = m_nameInput->text().trimmed();
		m_graph->addNode(graphPos.toPoint()); // Use transformed coordinates
		m_graph->getNodes().back().setName(name);

		updateGraphEdges();
		m_nameInput->clear();
		m_mode = InteractionMode::Normal;
		updateStatus("✅ Added section: " + name);
		update();
		return;
	}

	if (m_mode == InteractionMode::Removing) {
		if (hit != -1) {
			QString name = "";
			for(const auto& n : m_graph->getNodes()) if(n.getIndex() == hit) name = n.getName();
			m_graph->removeNode(hit);

			m_startIdx = m_endIdx = -1;
			m_finalPath.clear();
			updateGraphEdges();
			m_mode = InteractionMode::Normal;
			updateStatus("🗑 Removed: " + name);
			update();
		}
		return;
	}

	if (hit == -1)
		return;

	m_draggedNodeIdx = hit;
	m_isDragging = false;
}

void VisSupermarket::mouseMoveEvent(QMouseEvent* e) {
	if (m_draggedNodeIdx == -1 || m_mode != InteractionMode::Normal || !m_graph)
		return;

	m_isDragging = true;
	QPoint rawPos = e->pos();

	// Static Store Bounds
	int minX = 50, maxX = 1000;
	int minY = 50, maxY = 650;

	double padX = 80, padY = 60, bottomPad = 120;
	double viewW = width();
	double viewH = height() - 130;
	double graphW = (maxX - minX) + padX * 2;
	double graphH = (maxY - minY) + padY + bottomPad;
	double scale = std::min(viewW / graphW, viewH / graphH);
	double offsetX = (viewW - graphW * scale) / 2.0;
	double offsetY = 130 + (viewH - graphH * scale) / 2.0;
	QPointF graphPos = (QPointF(rawPos) - QPointF(offsetX, offsetY)) / scale +
		QPointF(minX - padX, minY - padY);

	// Clamp node to store walls
	int finalX = std::clamp((int)graphPos.x(), minX, maxX);
	int finalY = std::clamp((int)graphPos.y(), minY, maxY);

	for (Node& n : m_graph->getNodes()) {
		if (n.getIndex() == m_draggedNodeIdx) {
			n.setCoord(QPoint(finalX, finalY));
			break;
		}
	}

	updateGraphEdges();
	if (m_startIdx != -1 && m_endIdx != -1) {
		onFindPathClicked();
	}
	update();
}

void VisSupermarket::mouseReleaseEvent(QMouseEvent* e) {
	if (m_draggedNodeIdx == -1 || m_mode != InteractionMode::Normal)
		return;

	if (!m_isDragging) {
		if (e->modifiers() & Qt::ShiftModifier) {
			m_endIdx = m_draggedNodeIdx;
		}
		else {
			m_startIdx = m_draggedNodeIdx;
			if (m_startIdx == m_endIdx)
				m_endIdx = -1;
		}

		m_finalPath.clear();
		m_pathCost = 0;

		if (m_startIdx != -1 && m_endIdx != -1) {
			onFindPathClicked();
		}
		else {
			QString startName = "...";
			QString endName = "...";
			for (const auto& n : m_graph->getNodes()) {
				if (n.getIndex() == m_startIdx) startName = n.getName();
				if (n.getIndex() == m_endIdx) endName = n.getName();
			}
			updateStatus(QString("Route: %1 → %2").arg(startName, endName));
		}
		update();
	}

	m_draggedNodeIdx = -1;
	m_isDragging = false;
}

void VisSupermarket::onFindPathClicked() {
	if (m_startIdx == -1 || m_endIdx == -1) {
		updateStatus("❌ Please select both Start and End sections.");
		return;
	}
	std::vector<int> path;
	if (m_graph)
		DijkstraSolver{}.solve(*m_graph, m_startIdx, m_endIdx, path);
	m_finalPath = path;
	if (m_finalPath.empty()) {
		QString startName = "";
		QString endName = "";
		for (const auto& n : m_graph->getNodes()) {
			if (n.getIndex() == m_startIdx) startName = n.getName();
			if (n.getIndex() == m_endIdx) endName = n.getName();
		}
		updateStatus("⚠️ No route found between " + startName + " and " + endName);
	}
	else {
		m_pathCost = 0;
		const std::vector<Edge>& edges = m_graph->getEdges();
		for (size_t i = 0; i + 1 < m_finalPath.size(); ++i) {
			for (const Edge& e : edges) {
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

void VisSupermarket::paintEvent(QPaintEvent*) {
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);
	p.fillRect(rect(), QColor(8, 10, 20));

	if (!m_graph || m_graph->getNodes().empty())
		return;

	// Use fixed bounds to keep the "Store" area stable
	int minX = 50, maxX = 1000;
	int minY = 50, maxY = 650;

	double padX = 80, padY = 60, bottomPad = 120;
	double viewW = width();
	double viewH = height() - 130;
	double graphW = (maxX - minX) + padX * 2;
	double graphH = (maxY - minY) + padY + bottomPad;
	double scale = std::min(viewW / graphW, viewH / graphH);

	double offsetX = (viewW - graphW * scale) / 2.0;
	double offsetY = 130 + (viewH - graphH * scale) / 2.0;

	p.save();
	p.translate(offsetX, offsetY);
	p.scale(scale, scale);
	p.translate(-(minX - padX), -(minY - padY));

	// --- DRAW STORE FLOOR / PERIMETER ---
	QRectF storeRect(minX - padX, minY - padY, graphW, graphH);

	// Floor background
	p.setBrush(QColor(15, 20, 35));
	p.setPen(QPen(QColor(99, 102, 241, 100), 2, Qt::SolidLine, Qt::RoundCap,
		Qt::RoundJoin));
	p.drawRoundedRect(storeRect, 15, 15);

	// Grid limited to floor
	p.setPen(QPen(QColor(30, 40, 70), 1));
	for (int x = (int)storeRect.left(); x < storeRect.right(); x += 50)
		p.drawLine(x, (int)storeRect.top(), x, (int)storeRect.bottom());
	for (int y = (int)storeRect.top(); y < storeRect.bottom(); y += 50)
		p.drawLine((int)storeRect.left(), y, (int)storeRect.right(), y);

	RenderSettings settings;
	settings.nodeRadius = NODE_R;
	settings.edgeWidth = 2;
	settings.showEdgeCosts = false;

	// Draw graph edges
	if (m_graph) {
		for (const auto& e : m_graph->getEdges()) {
			// Skip one direction for visual clarity if they are symmetric
			if (e.getFirst().getIndex() < e.getSecond().getIndex()) {
				GraphRenderer::drawEdge(p, e, settings);
			}
		}
	}

	// Draw path if found
	if (!m_finalPath.empty()) {
		settings.highlightColor = QColor(99, 102, 241); // Indigo
		GraphRenderer::drawPath(p, *m_graph, m_finalPath, settings);
	} else {
		// Draw nodes
		for (const auto& node : m_graph->getNodes()) {
			bool isSpecial = (node.getIndex() == m_startIdx || node.getIndex() == m_endIdx);
			RenderSettings s = settings;
			if (isSpecial) {
				s.highlightColor = (node.getIndex() == m_startIdx) ? QColor(34, 197, 94) : QColor(239, 68, 68);
				GraphRenderer::drawNode(p, node, s, true);
			} else {
				GraphRenderer::drawNode(p, node, s);
			}
		}
	}
	p.restore();
}
