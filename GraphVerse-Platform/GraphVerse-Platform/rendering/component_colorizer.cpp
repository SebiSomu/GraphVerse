#include "component_colorizer.h"

QColor ComponentColorizer::getColorForComponent(int component, int totalComponents) {
    if (totalComponents <= 0 || component < 0) return {255, 255, 255};
    static const std::vector<QColor> baseColors = {
        QColor(255, 0, 0), QColor(255, 255, 0), QColor(0, 0, 255),
        QColor(128, 0, 128), QColor(0, 255, 0), QColor(255, 165, 0),
        QColor(165, 42, 42), QColor(255, 105, 180)
    };
    int baseColorsSize = static_cast<int>(baseColors.size());
    if (component < baseColorsSize) return baseColors[component];
    int extraComponent = component - baseColorsSize;
    int totalExtra = totalComponents - baseColorsSize;
    if (totalExtra <= 0) return baseColors[0];
    float progression = static_cast<float>(extraComponent) / totalExtra;
    switch (extraComponent % 12) {
    case 0: return {120+static_cast<int>(135 * progression), 20+static_cast<int>(180 * progression), 20+static_cast<int>(180 * progression)};
    case 1: return {180+static_cast<int>(60 * progression), 180+static_cast<int>(60 * progression), 50+static_cast<int>(50 * progression)};
    case 2: return {30+static_cast<int>(40 * progression), 80+static_cast<int>(100 * progression), 150+static_cast<int>(80 * progression)};
    case 3: return {100+static_cast<int>(80 * progression), 30+static_cast<int>(40 * progression), 120+static_cast<int>(80 * progression)};
    case 4: return {30+static_cast<int>(40 * progression), 120+static_cast<int>(100 * progression), 80+static_cast<int>(80 * progression)};
    case 5: return {180+static_cast<int>(60 * progression), 100+static_cast<int>(80 * progression), 30+static_cast<int>(40 * progression)};
    case 6: return {0, 120+static_cast<int>(80 * progression), 120+static_cast<int>(80 * progression)};
    case 7: return {120+static_cast<int>(80 * progression), 0, 80+static_cast<int>(100 * progression)};
    case 8: return {160+static_cast<int>(60 * progression), 140+static_cast<int>(60 * progression), 100+static_cast<int>(60 * progression)};
    case 9: return {0, 100+static_cast<int>(80 * progression), 140+static_cast<int>(80 * progression)};
    case 10: return {160+static_cast<int>(60 * progression), 80+static_cast<int>(80 * progression), 100+static_cast<int>(80 * progression)};
    case 11: return {120+static_cast<int>(60 * progression), 80+static_cast<int>(60 * progression), 140+static_cast<int>(60 * progression)};
    default: return {255, 0, 0};
    }
}
