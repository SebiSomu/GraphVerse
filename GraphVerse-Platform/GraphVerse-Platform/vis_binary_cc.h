#ifndef VIS_BINARY_CC_H
#define VIS_BINARY_CC_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QComboBox>
#include <QCheckBox>
#include <QHideEvent>
#include <vector>
#include "algorithms/binary_image_cc_solver.h"

// ─────────────────────────────────────────────────────────────
//  VisBinaryCC
//  Visualizer: Binary Image Connected Components
//  with Morphological Pre-processing
// ─────────────────────────────────────────────────────────────
class VisBinaryCC : public QWidget {
    Q_OBJECT

public:
    explicit VisBinaryCC(QWidget* parent = nullptr);
    ~VisBinaryCC();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private slots:
    void onDetectComponents();
    void onErode();
    void onDilate();
    void onOpen();
    void onClose();
    void onReset();
    void onLoadPattern(int index);
    void onAnimTick();

private:
    // Grid dimensions
    static constexpr int ROWS = 30;
    static constexpr int COLS = 50;
    static constexpr int CELL = 18;

    void setupUi();
    void resetGrid();
    void applyMorphOp(std::vector<std::vector<int>> newGrid);
    void startAnimation();
    void updateInfoPanel();

    StructuringElement currentSE() const;

    // Grid data: 1=foreground, 0=background
    std::vector<std::vector<int>> m_grid;

    // Labeled result (for second-pass visualization)
    LabeledImage m_labeledImage;

    // Animation
    std::vector<CCAnimStep> m_animSteps;
    // Intermediate label state built during animation
    std::vector<std::vector<int>> m_animLabel;
    size_t m_animStep = 0;
    bool m_animating = false;
    bool m_showResult = false;
    QTimer* m_timer;
    int m_tickInterval = 4;

    // Drawing helpers
    int offsetX() const;
    int offsetY() const;

    // Color palette for components (max 24 distinct colors)
    static QColor componentColor(int id);

    // UI elements
    QWidget*     m_toolbar;
    QWidget*     m_infoPanel;
    QLabel*      m_statusLabel;
    QLabel*      m_compCountLabel;
    QLabel*      m_compStatsLabel;
    QComboBox*   m_patternBox;
    QComboBox*   m_seBox;        // structuring element selector
    QLabel*      m_passLabel;    // current pass indicator
    bool         m_mouseDrawing = false;
    int          m_drawValue    = 1; // what we draw with current drag

    // Predefined patterns
    void loadLetterH();
    void loadLetterI();
    void loadGrid3x3();
    void loadNoise();
    void loadDots();
    void loadCircles();
};

#endif // VIS_BINARY_CC_H
