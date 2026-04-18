#ifndef THEORETICAL_PAGE_BUILDER_H
#define THEORETICAL_PAGE_BUILDER_H

#include "page_builder_base.h"
#include <QStackedWidget>
#include <functional>

// Builds the Theoretical Applications page (SRP)
// Receives all dependencies via callbacks (DIP) - no platform dependency
class TheoreticalPageBuilder : public PageBuilderBase {
public:
    struct TheoreticalEntry {
        QString label;
        std::function<void()> onClick;
    };

    explicit TheoreticalPageBuilder(
        QStackedWidget* stack,
        std::function<QWidget*(const QString& title, const QString& icon,
                               const QColor& accent)> headerBuilder,
        const std::vector<TheoreticalEntry>& entries,
        const QColor& accentColor);
    
    QWidget* build() override;

private:
    QStackedWidget* m_stack;
    std::function<QWidget*(const QString&, const QString&, const QColor&)> m_headerBuilder;
    std::vector<TheoreticalEntry> m_entries;
    QColor m_accentColor;
};

#endif // THEORETICAL_PAGE_BUILDER_H
