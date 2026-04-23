#ifndef COMPONENT_COLOR_PROVIDER_H
#define COMPONENT_COLOR_PROVIDER_H

#include "icomponent_colorable.h"
#include <QColor>
#include <vector>

// Concrete implementation of IComponentColorProvider
// Uses a predefined palette of 24 distinct colors
class ComponentColorProvider : public IComponentColorProvider {
public:
    ComponentColorProvider();
    
    // IComponentColorProvider implementation
    QColor getColorForComponent(int componentId) const override;
    void setComponentCount(int count) override;

private:
    int m_componentCount = 0;
    
    // 24 distinct colors palette
    static const std::vector<QColor> s_palette;
};

#endif // COMPONENT_COLOR_PROVIDER_H
