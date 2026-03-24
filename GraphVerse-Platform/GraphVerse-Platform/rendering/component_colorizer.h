#ifndef COMPONENT_COLORIZER_H
#define COMPONENT_COLORIZER_H

#include <QColor>
#include <vector>

class ComponentColorizer {
public:
    static QColor getColorForComponent(int component, int totalComponents);
};

#endif // COMPONENT_COLORIZER_H
