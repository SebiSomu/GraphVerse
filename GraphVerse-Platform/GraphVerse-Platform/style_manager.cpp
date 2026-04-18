#include "style_manager.h"

StyleManager& StyleManager::instance() {
    static StyleManager instance;
    return instance;
}

// Background colors
QColor StyleManager::backgroundDeep() const {
    return QColor(8, 10, 20);
}

QColor StyleManager::backgroundSurface() const {
    return QColor(14, 17, 32);
}

QColor StyleManager::backgroundCard() const {
    return QColor(18, 22, 42);
}

QColor StyleManager::backgroundCardHover() const {
    return QColor(22, 28, 56);
}

// Accent colors
QColor StyleManager::accentVisualisers() const {
    return QColor(99, 102, 241);  // indigo
}

QColor StyleManager::accentApps() const {
    return QColor(20, 184, 166);  // teal
}

QColor StyleManager::accentTheoretical() const {
    return QColor(245, 158, 11);  // orange
}

// Text colors
QColor StyleManager::textPrimary() const {
    return QColor(235, 237, 255);
}

QColor StyleManager::textSecondary() const {
    return QColor(140, 148, 190);
}

QColor StyleManager::borderColor() const {
    return QColor(45, 55, 100);
}

QColor StyleManager::glowColor() const {
    return QColor(140, 148, 190);
}

// Button styles
QString StyleManager::animatedButtonStyle(const QColor& accent) const {
    return QString();
    // Note: AnimatedButton uses custom paint, style sheet not needed
    // This is kept for consistency if needed elsewhere
}

QString StyleManager::backButtonStyle() const {
    return QString();
    // Note: Back button uses custom styling through AnimatedButton
}

QString StyleManager::standardButtonStyle(const QColor& bg, const QColor& hover) const {
    return QString("QPushButton { background: %1; border-radius: 6px; }"
                   "QPushButton:hover { background: %2; }")
        .arg(bg.name())
        .arg(hover.name());
}

QString StyleManager::separatorStyle(const QColor& accent) const {
    return QString("background: rgba(%1,%2,%3,0.3); border:none;")
        .arg(accent.red())
        .arg(accent.green())
        .arg(accent.blue());
}
