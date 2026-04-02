#include "vis_floodfill.h"
#include "algorithms/flood_fill_solver.h"
#include <QPainter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <queue>

VisFloodFill::VisFloodFill(QWidget* parent)
    : QWidget(parent), m_timer(new QTimer(this)), m_animating(false) {
    m_grid.assign(ROWS, std::vector<Cell>(COLS));
    setupUi();
    resetGrid();

    connect(m_timer, &QTimer::timeout, this, &VisFloodFill::onAnimationTick);
}

VisFloodFill::~VisFloodFill() = default;

void VisFloodFill::setupUi() {
    m_toolbar = new QWidget(this);
    m_toolbar->setFixedHeight(60);
    m_toolbar->setStyleSheet("background: rgba(15, 23, 42, 0.8); border-bottom: 1px solid rgba(51, 65, 85, 0.5);");

    auto* btnReset = new QPushButton("Clear Grid", m_toolbar);
    btnReset->setFixedSize(110, 36);
    btnReset->setCursor(Qt::PointingHandCursor);
    btnReset->setStyleSheet(
        "QPushButton { background: #334155; color: #f1f5f9; border-radius: 6px; font-weight: 600; border: 1px solid #475569; }"
        "QPushButton:hover { background: #475569; }"
    );

    m_statusLabel = new QLabel("Click to place walls, Right-Click to start Flood Fill", m_toolbar);
    m_statusLabel->setStyleSheet("color: #94a3b8; font-weight: 500; font-size: 13px;");

    m_speedSlider = new QSlider(Qt::Horizontal, m_toolbar);
    m_speedSlider->setRange(1, 100);
    m_speedSlider->setValue(85);
    m_speedSlider->setFixedWidth(150);
    m_speedSlider->setStyleSheet(
        "QSlider::groove:horizontal { height: 4px; background: #374151; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: #22d3ee; width: 14px; height: 14px; margin: -5px 0; border-radius: 7px; }"
    );

    auto* speedLabel = new QLabel("Speed", m_toolbar);
    speedLabel->setStyleSheet("color: #64748b; font-size: 11px; font-weight: 600;");

    auto* layout = new QHBoxLayout(m_toolbar);
    layout->setContentsMargins(20, 0, 20, 0);
    layout->setSpacing(15);
    layout->addWidget(btnReset);
    layout->addSpacing(10);
    layout->addWidget(speedLabel);
    layout->addWidget(m_speedSlider);
    layout->addStretch();
    layout->addWidget(m_statusLabel);

    connect(btnReset, &QPushButton::clicked, this, &VisFloodFill::onResetClicked);
    connect(m_speedSlider, &QSlider::valueChanged, this, &VisFloodFill::onSpeedChanged);
}

void VisFloodFill::resetGrid() {
    m_timer->stop();
    m_animating = false;
    m_animStep = 0;
    m_fillOrder.clear();
    for (auto& row : m_grid) {
        for (auto& cell : row) {
            cell.type = 0;
            cell.visited = false;
        }
    }
    m_statusLabel->setText("Click to place walls, Right-Click to start Flood Fill");
    update();
}

void VisFloodFill::onResetClicked() {
    resetGrid();
}

void VisFloodFill::onSpeedChanged(int) {
    int val = m_speedSlider->value();
    m_tickInterval = 1 + (100 - val) * 2;
    if (m_timer->isActive()) {
        m_timer->setInterval(m_tickInterval);
    }
}

void VisFloodFill::mousePressEvent(QMouseEvent* event) {
    if (m_animating) return;

    int r = (event->pos().y() - offsetY()) / CELL_SIZE;
    int c = (event->pos().x() - offsetX()) / CELL_SIZE;

    if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
        if (event->button() == Qt::LeftButton) {
            m_grid[r][c].type = (m_grid[r][c].type == 1) ? 0 : 1;
            update();
        } else if (event->button() == Qt::RightButton) {
            if (m_grid[r][c].type != 1) {
                startFloodFill(r, c);
            }
        }
    }
}

void VisFloodFill::startFloodFill(int startR, int startC) {
    m_animating = true;
    m_animStep = 0;

    std::vector<std::vector<int>> gridData(ROWS, std::vector<int>(COLS));
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            gridData[r][c] = m_grid[r][c].type;
            m_grid[r][c].visited = false;
            if (m_grid[r][c].type == 2) {
                m_grid[r][c].type = 0;
                gridData[r][c] = 0;
            }
        }
    }

    auto fillPoints = FloodFillSolver::solve(gridData, startR, startC, 0, 2);
    m_fillOrder.clear();
    for (const auto& p : fillPoints) {
        m_fillOrder.push_back({p.r, p.c});
    }

    m_statusLabel->setText("Filling area...");
    onSpeedChanged(0);
    m_timer->start(m_tickInterval);
}

void VisFloodFill::onAnimationTick() {
    if (m_animStep < m_fillOrder.size()) {
        auto [r, c] = m_fillOrder[m_animStep++];
        m_grid[r][c].type = 2;
        update();
    } else {
        m_timer->stop();
        m_animating = false;
        m_statusLabel->setText("Flood Fill complete!");
    }
}

void VisFloodFill::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor(8, 10, 20));

    int ox = offsetX();
    int oy = offsetY();

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            QRect cellRect(ox + c * CELL_SIZE, oy + r * CELL_SIZE, CELL_SIZE, CELL_SIZE);
            
            if (m_grid[r][c].type == 1) {
                p.fillRect(cellRect.adjusted(1,1,-1,-1), QColor(51, 65, 85));
            } else if (m_grid[r][c].type == 2) {
                p.fillRect(cellRect.adjusted(1,1,-1,-1), QColor(20, 184, 166, 180));
            }
            
            p.setPen(QColor(30, 41, 59, 100));
            p.drawRect(cellRect);
        }
    }
}

int VisFloodFill::offsetX() const { return (width() - COLS * CELL_SIZE) / 2; }
int VisFloodFill::offsetY() const { return 80; }

void VisFloodFill::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    m_toolbar->setFixedWidth(width());
}

void VisFloodFill::hideEvent(QHideEvent* event) {
    onResetClicked();
    QWidget::hideEvent(event);
}
