#ifndef QTMARKDOWN_CORE_ABSTRACTPAINTER_H
#define QTMARKDOWN_CORE_ABSTRACTPAINTER_H

#include "Types.h"
#include <QString>

namespace md::editor::core {

class AbstractPainter {
public:
    virtual ~AbstractPainter() = default;

    virtual void save() = 0;
    virtual void restore() = 0;
    virtual void setPen(const Color& color) = 0;
    virtual void drawRect(const Rect& rect) = 0;
    virtual void drawText(const Point& pos, const QString& text) = 0;
    virtual void drawLine(const Point& p1, const Point& p2) = 0;
};

} // namespace md::editor::core
#endif // QTMARKDOWN_CORE_ABSTRACTPAINTER_H
