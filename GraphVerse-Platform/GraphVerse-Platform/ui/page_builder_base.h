#ifndef PAGE_BUILDER_BASE_H
#define PAGE_BUILDER_BASE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QStackedWidget>
#include <functional>

// Abstract base class for page builders (SRP - Single Responsibility)
// Each derived class builds exactly one type of page
class PageBuilderBase {
public:
    virtual ~PageBuilderBase() = default;
    
    // Build and return the page widget
    virtual QWidget* build() = 0;
    
protected:
    // Helper to create a scrollable list area with consistent styling
    static QWidget* createListArea(QVBoxLayout*& outLayout);
    
    // Helper to create a scroll area wrapping a widget
    static QScrollArea* createScrollArea(QWidget* content);
    
    // Helper to create a page container with standard margins
    static QWidget* createPageContainer(const QString& title, 
                                        const QString& icon,
                                        const QColor& accentColor,
                                        QStackedWidget* stack,
                                        std::function<QWidget*()> headerBuilder);
};

#endif // PAGE_BUILDER_BASE_H
