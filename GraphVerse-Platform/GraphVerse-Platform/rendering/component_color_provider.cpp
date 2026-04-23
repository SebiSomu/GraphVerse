#include "component_color_provider.h"

// 24 distinct colors palette - same as used in vis_binary_cc
const std::vector<QColor> ComponentColorProvider::s_palette = {
    QColor(99,  102, 241),  // indigo
    QColor(16,  185, 129),  // emerald
    QColor(245, 158, 11),   // amber
    QColor(239, 68,  68),   // red
    QColor(59,  130, 246),  // blue
    QColor(139, 92,  246),  // violet
    QColor(20,  184, 166),  // teal
    QColor(236, 72,  153),  // pink
    QColor(14,  165, 233),  // sky
    QColor(168, 85,  247),  // purple
    QColor(250, 204, 21),  // yellow
    QColor(249, 115, 22),  // orange
    QColor(6,   182, 212),  // cyan
    QColor(132, 204, 22),  // lime
    QColor(244, 63,  94),   // rose
    QColor(180, 83,  9),    // dark orange
    QColor(37,  99,  235),  // dark blue
    QColor(147, 51,  234),  // dark purple
    QColor(15,  118, 110),  // dark teal
    QColor(190, 18,  60),   // dark red
    QColor(71,  85,  105),  // slate
    QColor(51,  65,  85),   // dark slate
    QColor(64,  64,  64),   // gray
    QColor(0,   0,   0)     // black (fallback)
};

ComponentColorProvider::ComponentColorProvider() = default;

QColor ComponentColorProvider::getColorForComponent(int componentId) const {
    if (componentId < 0 || s_palette.empty()) {
        return Qt::white;
    }
    return s_palette[componentId % s_palette.size()];
}

void ComponentColorProvider::setComponentCount(int count) {
    m_componentCount = count;
}
