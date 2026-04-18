#ifndef APPS_PAGE_BUILDER_H
#define APPS_PAGE_BUILDER_H

#include "page_builder_base.h"
#include <QStackedWidget>
#include <functional>

// Builds the Real-World Applications page (SRP)
// Receives all dependencies via callbacks (DIP) - no platform dependency
class AppsPageBuilder : public PageBuilderBase {
public:
    struct AppEntry {
        QString label;
        std::function<void()> onClick;
    };

    explicit AppsPageBuilder(
        QStackedWidget* stack,
        std::function<QWidget*(const QString& title, const QString& icon,
                               const QColor& accent)> headerBuilder,
        const std::vector<AppEntry>& entries,
        const QColor& accentColor);
    
    QWidget* build() override;

private:
    QStackedWidget* m_stack;
    std::function<QWidget*(const QString&, const QString&, const QColor&)> m_headerBuilder;
    std::vector<AppEntry> m_entries;
    QColor m_accentColor;
};

#endif // APPS_PAGE_BUILDER_H
