#ifndef QTMARKDOWN_FONTMETRICSPROVIDER_H
#define QTMARKDOWN_FONTMETRICSPROVIDER_H

#include "mddef.h"
#include "QtMarkdown_global.h"
#include <QFontMetrics>

namespace md::render {

class QTMARKDOWNSHARED_EXPORT IFontMetricsProvider {
public:
    virtual ~IFontMetricsProvider() = default;
    virtual Size size(const Font& font, const String& text) const = 0;
    virtual int horizontalAdvance(const Font& font, const String& text) const = 0;
    virtual int height(const Font& font) const = 0;
};

// Production implementation -- wraps QFontMetrics.
// Stateless -- safe to use as a static singleton or local variable.
class QTMARKDOWNSHARED_EXPORT QtFontMetricsProvider : public IFontMetricsProvider {
public:
    Size size(const Font& font, const String& text) const override {
        QFontMetrics fm(font);
        return fm.size(Qt::TextSingleLine, text);
    }
    int horizontalAdvance(const Font& font, const String& text) const override {
        QFontMetrics fm(font);
        return fm.horizontalAdvance(text);
    }
    int height(const Font& font) const override {
        QFontMetrics fm(font);
        return fm.height();
    }
};

} // namespace md::render
#endif // QTMARKDOWN_FONTMETRICSPROVIDER_H
