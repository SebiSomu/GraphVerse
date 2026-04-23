#include "vis_binary_cc.h"
#include "rendering/component_color_provider.h"
#include <QPainter>
#include <QPainterPath>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QFont>
#include <cmath>

// ─────────────────────────────────────────────────────────────
//  Constructor / Destructor
// ─────────────────────────────────────────────────────────────
VisBinaryCC::VisBinaryCC(QWidget* parent)
    : QWidget(parent), m_timer(new QTimer(this)), m_colorProvider(std::make_unique<ComponentColorProvider>())
{
    m_grid.assign(ROWS, std::vector<int>(COLS, 0));
    m_animLabel.assign(ROWS, std::vector<int>(COLS, 0));
    setupUi();
    loadLetterH();
    connect(m_timer, &QTimer::timeout, this, &VisBinaryCC::onAnimTick);
}

VisBinaryCC::~VisBinaryCC() = default;

// ─────────────────────────────────────────────────────────────
//  UI Setup
// ─────────────────────────────────────────────────────────────
void VisBinaryCC::setupUi() {
    // ── Toolbar ──────────────────────────────────────────────
    m_toolbar = new QWidget(this);
    m_toolbar->setFixedHeight(56);
    m_toolbar->setStyleSheet(
        "background: rgba(10,14,30,0.95);"
        "border-bottom: 1px solid rgba(99,102,241,0.2);"
    );

    auto btnStyle = [](const QString& accent) {
        return QString(
            "QPushButton { background: rgba(%1,0.15); color: #e2e8f0; border: 1px solid rgba(%1,0.4);"
            " border-radius: 5px; font: 600 10px 'Segoe UI'; padding: 0 10px; }"
            "QPushButton:hover { background: rgba(%1,0.30); }"
            "QPushButton:pressed { background: rgba(%1,0.50); }"
        ).arg(accent);
    };

    // Morphological buttons
    auto* btnErode  = new QPushButton("⊖ Erosion",   m_toolbar);
    auto* btnDilate = new QPushButton("⊕ Dilation",  m_toolbar);
    auto* btnOpen   = new QPushButton("○ Opening",   m_toolbar);
    auto* btnClose  = new QPushButton("● Closing",   m_toolbar);
    auto* btnDetect = new QPushButton("🔍 Detect Components", m_toolbar);
    auto* btnReset  = new QPushButton("↺ Reset",     m_toolbar);

    for (auto* btn : {btnErode, btnDilate, btnOpen, btnClose})
        btn->setFixedHeight(34);
    btnDetect->setFixedHeight(34);
    btnReset->setFixedHeight(34);

    btnErode ->setStyleSheet(btnStyle("239,68,68"));
    btnDilate->setStyleSheet(btnStyle("16,185,129"));
    btnOpen  ->setStyleSheet(btnStyle("245,158,11"));
    btnClose ->setStyleSheet(btnStyle("168,85,247"));
    btnDetect->setStyleSheet(
        "QPushButton { background: rgba(99,102,241,0.25); color: #c7d2fe;"
        " border: 1px solid rgba(99,102,241,0.6); border-radius: 5px;"
        " font: 700 10px 'Segoe UI'; padding: 0 14px; }"
        "QPushButton:hover { background: rgba(99,102,241,0.45); }"
    );
    btnReset->setStyleSheet(btnStyle("100,116,139"));

    // Structuring element selector
    m_seBox = new QComboBox(m_toolbar);
    m_seBox->addItem("SE: Cross (+)", 0);
    m_seBox->addItem("SE: Square (□)", 1);
    m_seBox->setFixedHeight(34);
    m_seBox->setStyleSheet(
        "QComboBox { background: rgba(30,41,59,0.9); color: #94a3b8; border: 1px solid #334155;"
        " border-radius: 5px; font: 600 10px 'Segoe UI'; padding: 0 8px; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView { background: #1e293b; color: #e2e8f0; selection-background-color: #3730a3; }"
    );

    // Pattern selector
    m_patternBox = new QComboBox(m_toolbar);
    m_patternBox->addItem("Pattern: Letter H",  0);
    m_patternBox->addItem("Pattern: Letter I",  1);
    m_patternBox->addItem("Pattern: Grid 3×3",  2);
    m_patternBox->addItem("Pattern: Dots",      3);
    m_patternBox->addItem("Pattern: Circles",   4);
    m_patternBox->addItem("Pattern: Noise",     5);
    m_patternBox->setFixedHeight(34);
    m_patternBox->setStyleSheet(m_seBox->styleSheet());

    auto* tbLay = new QHBoxLayout(m_toolbar);
    tbLay->setContentsMargins(16, 0, 16, 0);
    tbLay->setSpacing(8);
    tbLay->addWidget(m_patternBox);
    tbLay->addSpacing(6);
    tbLay->addWidget(m_seBox);
    tbLay->addSpacing(6);
    tbLay->addWidget(btnErode);
    tbLay->addWidget(btnDilate);
    tbLay->addWidget(btnOpen);
    tbLay->addWidget(btnClose);
    tbLay->addSpacing(10);
    tbLay->addWidget(btnDetect);
    tbLay->addStretch();
    tbLay->addWidget(btnReset);

    // ── Info Panel (right side) ───────────────────────────────
    m_infoPanel = new QWidget(this);
    m_infoPanel->setStyleSheet(
        "background: rgba(10,14,30,0.92);"
        "border-left: 1px solid rgba(99,102,241,0.18);"
    );

    m_statusLabel = new QLabel("Draw pixels with left-click.\nDetect with the 🔍 button.", m_infoPanel);
    m_statusLabel->setStyleSheet("color: #94a3b8; font: 500 11px 'Segoe UI';");
    m_statusLabel->setWordWrap(true);

    m_compCountLabel = new QLabel("Components: —", m_infoPanel);
    m_compCountLabel->setStyleSheet("color: #6366f1; font: 700 18px 'Segoe UI';");

    m_compStatsLabel = new QLabel("", m_infoPanel);
    m_compStatsLabel->setStyleSheet("color: #64748b; font: 400 10px 'Segoe UI';");
    m_compStatsLabel->setWordWrap(true);

    m_passLabel = new QLabel("", m_infoPanel);
    m_passLabel->setStyleSheet("color: #f59e0b; font: 600 10px 'Segoe UI';");

    // Legend
    auto* legendTitle = new QLabel("Morphological Legend:", m_infoPanel);
    legendTitle->setStyleSheet("color: #475569; font: 700 9px 'Segoe UI'; margin-top: 12px;");

    auto makeLegRow = [&](const QString& color, const QString& text) -> QLabel* {
        auto* lbl = new QLabel(QString("<span style='color:%1'>■</span> %2").arg(color, text), m_infoPanel);
        lbl->setStyleSheet("color: #64748b; font: 400 10px 'Segoe UI';");
        return lbl;
    };

    auto* infoPanelLay = new QVBoxLayout(m_infoPanel);
    infoPanelLay->setContentsMargins(14, 16, 14, 16);
    infoPanelLay->setSpacing(6);
    infoPanelLay->addWidget(m_compCountLabel);
    infoPanelLay->addWidget(m_compStatsLabel);
    infoPanelLay->addWidget(m_passLabel);
    infoPanelLay->addSpacing(8);
    infoPanelLay->addWidget(m_statusLabel);
    infoPanelLay->addSpacing(12);
    infoPanelLay->addWidget(legendTitle);
    infoPanelLay->addWidget(makeLegRow("#ef4444", "Erosion  — shrinks"));
    infoPanelLay->addWidget(makeLegRow("#10b981", "Dilation — expands"));
    infoPanelLay->addWidget(makeLegRow("#f59e0b", "Opening  — removes noise"));
    infoPanelLay->addWidget(makeLegRow("#a855f7", "Closing  — fills holes"));
    infoPanelLay->addStretch();

    // Wire signals
    connect(btnErode,  &QPushButton::clicked, this, &VisBinaryCC::onErode);
    connect(btnDilate, &QPushButton::clicked, this, &VisBinaryCC::onDilate);
    connect(btnOpen,   &QPushButton::clicked, this, &VisBinaryCC::onOpen);
    connect(btnClose,  &QPushButton::clicked, this, &VisBinaryCC::onClose);
    connect(btnDetect, &QPushButton::clicked, this, &VisBinaryCC::onDetectComponents);
    connect(btnReset,  &QPushButton::clicked, this, &VisBinaryCC::onReset);
    connect(m_patternBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VisBinaryCC::onLoadPattern);
}

// ─────────────────────────────────────────────────────────────
//  Layout helpers
// ─────────────────────────────────────────────────────────────
void VisBinaryCC::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    m_toolbar->setGeometry(0, 0, width(), 56);

    const int INFO_W = 200;
    m_infoPanel->setGeometry(width() - INFO_W, 56, INFO_W, height() - 56);
}

int VisBinaryCC::offsetX() const {
    const int canvasW = width() - 200; // exclude info panel
    return std::max(8, (canvasW - COLS * CELL) / 2);
}
int VisBinaryCC::offsetY() const { return 56 + 14; }

// ─────────────────────────────────────────────────────────────
//  Grid helpers
// ─────────────────────────────────────────────────────────────
void VisBinaryCC::resetGrid() {
    m_timer->stop();
    m_animating = false;
    m_showResult = false;
    m_animStep = 0;
    m_animSteps.clear();
    m_grid.assign(ROWS, std::vector<int>(COLS, 0));
    m_animLabel.assign(ROWS, std::vector<int>(COLS, 0));
    m_labeledImage = {};
    m_compCountLabel->setText("Components: —");
    m_compStatsLabel->setText("");
    m_passLabel->setText("");
    m_statusLabel->setText("Draw pixels with left-click.\nDetect with the 🔍 button.");
    update();
}

void VisBinaryCC::applyMorphOp(std::vector<std::vector<int>> newGrid) {
    m_timer->stop();
    m_animating = false;
    m_showResult = false;
    m_animStep = 0;
    m_animSteps.clear();
    m_animLabel.assign(ROWS, std::vector<int>(COLS, 0));
    m_labeledImage = {};
    m_compCountLabel->setText("Components: —");
    m_compStatsLabel->setText("");
    m_passLabel->setText("");
    m_grid = std::move(newGrid);
    update();
}

StructuringElement VisBinaryCC::currentSE() const {
    return (m_seBox->currentIndex() == 0) ? StructuringElement::Cross : StructuringElement::Square;
}

// ─────────────────────────────────────────────────────────────
//  Slots — Morphological Ops
// ─────────────────────────────────────────────────────────────
void VisBinaryCC::onErode() {
    m_statusLabel->setText("Operation: Erosion applied.\nBorder pixels have been removed.");
    applyMorphOp(BinaryImageCCSolver::erode(m_grid, currentSE()));
}
void VisBinaryCC::onDilate() {
    m_statusLabel->setText("Operation: Dilation applied.\nComponents have been expanded.");
    applyMorphOp(BinaryImageCCSolver::dilate(m_grid, currentSE()));
}
void VisBinaryCC::onOpen() {
    m_statusLabel->setText("Operation: Opening applied.\n(Erosion → Dilation)\nSmall noise has been removed.");
    applyMorphOp(BinaryImageCCSolver::open(m_grid, currentSE()));
}
void VisBinaryCC::onClose() {
    m_statusLabel->setText("Operation: Closing applied.\n(Dilation → Erosion)\nSmall holes have been filled.");
    applyMorphOp(BinaryImageCCSolver::close(m_grid, currentSE()));
}
void VisBinaryCC::onReset() { resetGrid(); loadLetterH(); }

// ─────────────────────────────────────────────────────────────
//  Slot — Detect Components
// ─────────────────────────────────────────────────────────────
void VisBinaryCC::onDetectComponents() {
    if (m_animating) return;

    m_animSteps.clear();
    m_animLabel.assign(ROWS, std::vector<int>(COLS, 0));
    m_showResult = false;

    auto conn = (m_seBox->currentIndex() == 0) ? Connectivity::Four : Connectivity::Eight;
    m_labeledImage = BinaryImageCCSolver::solve(m_grid, conn, &m_animSteps);

    m_animStep = 0;
    m_animating = true;
    m_passLabel->setText("Pass 1/2: Provisional labeling...");
    m_statusLabel->setText("Animation in progress...");
    m_compCountLabel->setText("Components: —");
    m_tickInterval = 3;
    m_timer->start(m_tickInterval);
}

// ─────────────────────────────────────────────────────────────
//  Slot — Animation tick
// ─────────────────────────────────────────────────────────────
void VisBinaryCC::onAnimTick() {
    constexpr size_t BATCH = 6; // pixels per tick for smooth but fast anim
    for (size_t i = 0; i < BATCH && m_animStep < m_animSteps.size(); ++i, ++m_animStep) {
        const auto& step = m_animSteps[m_animStep];
        m_animLabel[step.r][step.c] = step.label;

        // Detect pass boundary
        if (m_animStep + 1 < m_animSteps.size()) {
            if (m_animSteps[m_animStep + 1].pass == 2 && step.pass == 1) {
                m_passLabel->setText("Pass 2/2: Final relabeling...");
            }
        }
    }
    update();

    if (m_animStep >= m_animSteps.size()) {
        m_timer->stop();
        m_animating = false;
        m_showResult = true;
        updateInfoPanel();
    }
}

void VisBinaryCC::updateInfoPanel() {
    int n = m_labeledImage.numComponents;
    m_compCountLabel->setText(QString("Components: %1").arg(n));
    m_passLabel->setText("✓ Detection complete!");

    if (n == 0) {
        m_compStatsLabel->setText("No components found.");
        m_statusLabel->setText("Image is empty.");
        return;
    }

    const auto& sizes = m_labeledImage.componentSizes;
    int maxS = *std::max_element(sizes.begin(), sizes.end());
    int minS = *std::min_element(sizes.begin(), sizes.end());
    double avg = 0;
    for (int s : sizes) avg += s;
    avg /= sizes.size();

    m_compStatsLabel->setText(
        QString("Max: %1 px  Min: %2 px\nAvg: %3 px/comp")
            .arg(maxS).arg(minS).arg(QString::number(avg, 'f', 1))
    );
    m_statusLabel->setText(
        QString("Each connected component\nis colored distinctly.\nConnectivity: %1")
            .arg(m_seBox->currentIndex() == 0 ? "4 (Cross)" : "8 (Square)")
    );
}

// ─────────────────────────────────────────────────────────────
//  Slot — Load pattern
// ─────────────────────────────────────────────────────────────
void VisBinaryCC::onLoadPattern(int index) {
    resetGrid();
    switch (index) {
    case 0: loadLetterH();  break;
    case 1: loadLetterI();  break;
    case 2: loadGrid3x3(); break;
    case 3: loadDots();    break;
    case 4: loadCircles(); break;
    case 5: loadNoise();   break;
    }
    update();
}

// ─────────────────────────────────────────────────────────────
//  Predefined patterns
// ─────────────────────────────────────────────────────────────
void VisBinaryCC::loadLetterH() {
    // Draw two capital H letters side by side
    auto drawH = [&](int r0, int c0) {
        for (int r = r0; r < r0+10; ++r) {
            if (r >= 0 && r < ROWS) {
                if (c0   >= 0 && c0   < COLS) m_grid[r][c0]   = 1;
                if (c0+4 >= 0 && c0+4 < COLS) m_grid[r][c0+4] = 1;
            }
        }
        int mid = r0 + 5;
        for (int c = c0; c <= c0+4 && c < COLS; ++c)
            if (mid >= 0 && mid < ROWS) m_grid[mid][c] = 1;
    };
    drawH(10, 5);
    drawH(10, 15);
    drawH(10, 35);
}

void VisBinaryCC::loadLetterI() {
    auto drawI = [&](int r0, int c0) {
        for (int c = c0; c <= c0+6 && c < COLS; ++c) {
            if (r0 >= 0 && r0 < ROWS)    m_grid[r0][c]    = 1;
            if (r0+8 >= 0 && r0+8 < ROWS) m_grid[r0+8][c] = 1;
        }
        int mid = c0 + 3;
        for (int r = r0; r <= r0+8 && r < ROWS; ++r)
            if (mid >= 0 && mid < COLS) m_grid[r][mid] = 1;
    };
    drawI(8, 4);
    drawI(8, 18);
    drawI(8, 32);
}

void VisBinaryCC::loadGrid3x3() {
    // 3x3 arrangement of 4x4 blocks
    for (int gi = 0; gi < 3; ++gi) {
        for (int gj = 0; gj < 3; ++gj) {
            int r0 = 5 + gi * 8;
            int c0 = 8 + gj * 13;
            for (int dr = 0; dr < 4; ++dr)
                for (int dc = 0; dc < 4; ++dc)
                    if (r0+dr < ROWS && c0+dc < COLS)
                        m_grid[r0+dr][c0+dc] = 1;
        }
    }
}

void VisBinaryCC::loadDots() {
    // Regular dot grid — each dot is 1 pixel → many small components
    for (int r = 4; r < ROWS - 2; r += 4) {
        for (int c = 3; c < COLS - 2; c += 4) {
            m_grid[r][c] = 1;
        }
    }
}

void VisBinaryCC::loadCircles() {
    // Three approximate circles
    auto drawCircle = [&](int cr, int cc, int rad) {
        for (int r = 0; r < ROWS; ++r) {
            for (int c = 0; c < COLS; ++c) {
                double dr = r - cr, dc = c - cc;
                double d = std::sqrt(dr*dr + dc*dc);
                if (std::abs(d - rad) < 1.2)
                    m_grid[r][c] = 1;
            }
        }
    };
    drawCircle(15, 10, 7);
    drawCircle(15, 27, 7);
    drawCircle(15, 44, 5);
}

void VisBinaryCC::loadNoise() {
    // Random-looking pattern (deterministic seed)
    unsigned state = 0xDEADBEEF;
    for (int r = 2; r < ROWS-2; ++r) {
        for (int c = 2; c < COLS-2; ++c) {
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            m_grid[r][c] = (state % 3 == 0) ? 1 : 0;
        }
    }
}

// ─────────────────────────────────────────────────────────────
//  Mouse interaction — draw / erase pixels
// ─────────────────────────────────────────────────────────────
void VisBinaryCC::mousePressEvent(QMouseEvent* event) {
    if (m_animating) return;
    int r = (event->pos().y() - offsetY()) / CELL;
    int c = (event->pos().x() - offsetX()) / CELL;
    if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
        m_drawValue = (m_grid[r][c] == 1) ? 0 : 1;
        m_grid[r][c] = m_drawValue;
        m_mouseDrawing = true;
        // Reset labels on manual edit
        m_showResult = false;
        m_animLabel.assign(ROWS, std::vector<int>(COLS, 0));
        m_compCountLabel->setText("Components: —");
        m_compStatsLabel->setText("");
        m_passLabel->setText("");
        update();
    }
}

void VisBinaryCC::mouseMoveEvent(QMouseEvent* event) {
    if (!m_mouseDrawing || m_animating) return;
    int r = (event->pos().y() - offsetY()) / CELL;
    int c = (event->pos().x() - offsetX()) / CELL;
    if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
        m_grid[r][c] = m_drawValue;
        update();
    }
}

// ─────────────────────────────────────────────────────────────
//  Paint
// ─────────────────────────────────────────────────────────────
void VisBinaryCC::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Background
    p.fillRect(rect(), QColor(8, 10, 20));

    int ox = offsetX();
    int oy = offsetY();

    // Draw cells
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            QRect cell(ox + c * CELL, oy + r * CELL, CELL, CELL);
            QRect inner = cell.adjusted(1, 1, -1, -1);

            QColor fill;
            if (m_showResult) {
                // Final labeled result
                int lbl = m_labeledImage.label[r][c];
                fill = (lbl > 0) ? m_colorProvider->getColorForComponent(lbl - 1) : QColor(18, 24, 40);
            } else if (m_animating || m_animStep > 0) {
                // During animation — show progressive labeling
                int lbl = m_animLabel[r][c];
                if (lbl > 0) {
                    fill = m_colorProvider->getColorForComponent(lbl - 1);
                } else if (m_grid[r][c] == 1) {
                    fill = QColor(55, 65, 81); // unlabeled foreground
                } else {
                    fill = QColor(18, 24, 40);
                }
            } else {
                // Raw binary
                fill = (m_grid[r][c] == 1) ? QColor(226, 232, 240) : QColor(18, 24, 40);
            }

            p.fillRect(inner, fill);

            // Grid lines
            p.setPen(QColor(30, 41, 59, 80));
            p.drawRect(cell);
        }
    }

    // Outer border around grid
    QRect gridRect(ox, oy, COLS * CELL, ROWS * CELL);
    p.setPen(QPen(QColor(99, 102, 241, 60), 1));
    p.drawRect(gridRect);

    // Pass label overlay (first pass progress bar)
    if (m_animating && !m_animSteps.empty()) {
        float progress = static_cast<float>(m_animStep) / m_animSteps.size();
        int barW = static_cast<int>(COLS * CELL * progress);
        QRect bar(ox, oy + ROWS * CELL + 4, barW, 3);
        QLinearGradient grad(bar.topLeft(), bar.topRight());
        grad.setColorAt(0, QColor(99, 102, 241));
        grad.setColorAt(1, QColor(16, 185, 129));
        p.fillRect(bar, grad);
        // Track background
        p.fillRect(QRect(ox, oy + ROWS * CELL + 4, COLS * CELL, 3), QColor(30, 41, 59));
        p.fillRect(bar, grad);
    }
}

// ─────────────────────────────────────────────────────────────
//  Hide
// ─────────────────────────────────────────────────────────────
void VisBinaryCC::hideEvent(QHideEvent* event) {
    onReset();
    QWidget::hideEvent(event);
}
