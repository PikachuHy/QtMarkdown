#ifndef QTMARKDOWN_CORE_ABSTRACTPAINTER_H
#define QTMARKDOWN_CORE_ABSTRACTPAINTER_H

#include "Types.h"
#include "parser/MdString.h"

namespace md::editor::core {

class AbstractPainter {
public:
    virtual ~AbstractPainter() = default;

    virtual void save() = 0;
    virtual void restore() = 0;
    virtual void setPen(const Color& color) = 0;
    virtual void drawRect(const Rect& rect) = 0;
    virtual void drawText(const Point& pos, const String& text) = 0;
    virtual void drawLine(const Point& p1, const Point& p2) = 0;

    // New methods needed by Instruction::run()
    virtual void setFont(const FontDescription& font) = 0;
    virtual void fillRect(const Rect& rect, const Color& color) = 0;
    virtual void drawEllipse(const Rect& rect, const Color& color) = 0;
    virtual void drawImage(const Rect& rect, const ImageData& image) = 0;
    virtual void drawText(const Rect& rect, int flags, const String& text) = 0;

    // Returns a pointer to the native platform painter context.
    // For Qt: returns QPainter*. Returns nullptr in the base class.
    virtual void* nativePainter() const { return nullptr; }
};

} // namespace md::editor::core
#endif // QTMARKDOWN_CORE_ABSTRACTPAINTER_H
