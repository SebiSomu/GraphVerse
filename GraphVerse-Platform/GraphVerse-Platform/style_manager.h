#ifndef STYLE_MANAGER_H
#define STYLE_MANAGER_H

#include <QColor>
#include <QString>

// StyleManager - Centralized styling (SRP)
// Replaces the anonymous namespace Palette and scattered style definitions
// Provides consistent styling across the entire application

class StyleManager {
public:
    static StyleManager& instance();
    
    // Delete copy/move operations (singleton)
    StyleManager(const StyleManager&) = delete;
    StyleManager& operator=(const StyleManager&) = delete;
    StyleManager(StyleManager&&) = delete;
    StyleManager& operator=(StyleManager&&) = delete;
    
    // Background colors
    QColor backgroundDeep() const;
    QColor backgroundSurface() const;
    QColor backgroundCard() const;
    QColor backgroundCardHover() const;
    
    // Accent colors
    QColor accentVisualisers() const;  // indigo
    QColor accentApps() const;         // teal
    QColor accentTheoretical() const;  // orange
    
    // Text colors
    QColor textPrimary() const;
    QColor textSecondary() const;
    QColor borderColor() const;
    
    // Utility colors
    QColor glowColor() const;
    
    // Button styles
    QString animatedButtonStyle(const QColor& accent) const;
    QString backButtonStyle() const;
    QString standardButtonStyle(const QColor& bg, const QColor& hover) const;
    
    // Layout constants
    int pageMargin() const { return 48; }
    int sectionSpacing() const { return 18; }
    int buttonHeight() const { return 64; }
    int backButtonHeight() const { return 44; }
    int headerSpacing() const { return 12; }
    
    // Font settings
    QString titleFont() const { return "font: 700 28px 'Segoe UI';"; }
    QString buttonFont() const { return "font: 500 11px 'Segoe UI';"; }
    
    // Separator style
    QString separatorStyle(const QColor& accent) const;

private:
    StyleManager() = default;
    ~StyleManager() = default;
};

#endif // STYLE_MANAGER_H
