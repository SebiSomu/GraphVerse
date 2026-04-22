#ifndef VIS_ARTBRIDGE_H
#define VIS_ARTBRIDGE_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QHideEvent>
#include <QTime>
#include <QTimerEvent>
#include <memory>
#include "graph.h"
#include "algorithms/articulation_bridge_solver.h"

// ─────────────────────────────────────────────────────────────
//  VisArtBridge
//  Visualizer: Articulation Points & Bridges (Tarjan DFS)
// ─────────────────────────────────────────────────────────────
class VisArtBridge : public QWidget {
	Q_OBJECT

public:
	explicit VisArtBridge(QWidget* parent = nullptr);
	~VisArtBridge();

protected:
	void paintEvent(QPaintEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
	void hideEvent(QHideEvent*) override;

private slots:
	void onRun();
	void onReset();
	void onAnimTick();

private:
	void setupUi();
	void resetState();
	void updateInfoPanel();

	// Graph interaction (same pattern as VisComponents)
	std::unique_ptr<Graph> m_graph;
	Node* m_firstNode = nullptr;
	Node* m_draggedNode = nullptr;
	bool   m_dragging = false;
	bool   m_hasMoved = false;
	QPoint m_pressPosition;

	// Algorithm result & animation
	ArtBridgeResult              m_result;
	std::vector<ArtBridgeAnimStep> m_steps;
	size_t                       m_animStep = 0;
	bool                         m_animating = false;
	bool                         m_showResult = false;

	// Live DFS state (built during animation)
	std::unordered_map<int, int>  m_liveDisc;
	std::unordered_map<int, int>  m_liveLow;
	std::unordered_set<int>      m_liveVisited;
	std::unordered_set<int>      m_liveAPs;
	std::vector<std::pair<int, int>> m_liveBridges;
	int                          m_activeNode = -1;  // currently explored node
	int                          m_activeNode2 = -1;  // secondary (edge target)

	QTimer* m_timer;
	int           m_tickInterval = 350; // ms per step — slow enough to follow

	// UI
	QPushButton* m_btnRun;
	QPushButton* m_btnReset;
	QLabel* m_instrLabel;
	QLabel* m_resultLabel;
	QLabel* m_stepLabel;

	// Pulse animation for APs
	float m_pulse = 0.f;
	int   m_pulseTimer = 0;
	void  timerEvent(QTimerEvent*) override;
};

#endif // VIS_ARTBRIDGE_H
