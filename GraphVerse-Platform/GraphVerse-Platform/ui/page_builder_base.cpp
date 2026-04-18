#include "page_builder_base.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>

// Note: This requires AnimatedButton class from GraphVersePlatform.h
// Forward declare to avoid circular dependency
class AnimatedButton;

QWidget* PageBuilderBase::createListArea(QVBoxLayout*& outLayout) {
    auto* listArea = new QWidget;
    outLayout = new QVBoxLayout(listArea);
    outLayout->setContentsMargins(48, 16, 48, 60);
    outLayout->setSpacing(18);
    return listArea;
}

QScrollArea* PageBuilderBase::createScrollArea(QWidget* content) {
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(content);
    return scroll;
}
