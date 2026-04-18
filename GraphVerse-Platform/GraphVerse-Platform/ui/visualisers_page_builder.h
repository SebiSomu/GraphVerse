#ifndef VISUALISERS_PAGE_BUILDER_H
#define VISUALISERS_PAGE_BUILDER_H

#include "page_builder_base.h"
#include <QStackedWidget>
#include <functional>

// Builds the Algorithm Visualisers page (SRP)
// Receives all dependencies via callbacks (DIP) - no platform dependency
class VisualisersPageBuilder : public PageBuilderBase {
public:
    struct AlgoEntry {
        QString label;
        std::function<void()> onClick;
    };

    explicit VisualisersPageBuilder(
        QStackedWidget* stack,
        std::function<QWidget*(const QString& title, const QString& icon,
                               const QColor& accent)> headerBuilder,
        const std::vector<AlgoEntry>& entries,
        const QColor& accentColor);
    
    QWidget* build() override;

private:
    QStackedWidget* m_stack;
    std::function<QWidget*(const QString&, const QString&, const QColor&)> m_headerBuilder;
    std::vector<AlgoEntry> m_entries;
    QColor m_accentColor;
};

#endif // VISUALISERS_PAGE_BUILDER_H
