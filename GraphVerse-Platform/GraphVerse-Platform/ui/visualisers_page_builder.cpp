#include "visualisers_page_builder.h"
#include "../GraphVersePlatform.h"  // For AnimatedButton
#include <QVBoxLayout>
#include <QScrollArea>

VisualisersPageBuilder::VisualisersPageBuilder(
    QStackedWidget* stack,
    std::function<QWidget*(const QString&, const QString&, const QColor&)> headerBuilder,
    const std::vector<AlgoEntry>& entries,
    const QColor& accentColor)
    : m_stack(stack),
      m_headerBuilder(headerBuilder),
      m_entries(entries),
      m_accentColor(accentColor) {}

QWidget* VisualisersPageBuilder::build() {
    auto* page = new QWidget;
    auto* vlay = new QVBoxLayout(page);
    vlay->setContentsMargins(0, 0, 0, 0);
    vlay->setSpacing(0);

    // Use the provided header builder (identical to original buildPageHeader)
    vlay->addWidget(m_headerBuilder("Algorithm Visualisers", "⚡", m_accentColor));

    auto* listArea = new QWidget;
    auto* llay = new QVBoxLayout(listArea);
    llay->setContentsMargins(48, 16, 48, 60);
    llay->setSpacing(18);

    for (const auto& entry : m_entries) {
        auto* btn = new AnimatedButton(entry.label);
        btn->setGlowColor(m_accentColor);
        btn->setMinimumHeight(64);
        btn->setMaximumHeight(64);
        btn->setFont(QFont("Segoe UI", 11, QFont::Medium));
        llay->addWidget(btn);

        QObject::connect(btn, &QPushButton::clicked, entry.onClick);
    }
    llay->addStretch();

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(listArea);
    vlay->addWidget(scroll);

    return page;
}
