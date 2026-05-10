#ifndef QTMARKDOWN_FONTMETRICSPROVIDER_H
#define QTMARKDOWN_FONTMETRICSPROVIDER_H

#include "mddef.h"
#include "QtMarkdown_global.h"
#include "core/Types.h"

namespace md::render {

class QTMARKDOWNSHARED_EXPORT IFontMetricsProvider {
public:
    virtual ~IFontMetricsProvider() = default;
    virtual Size size(const Font& font, const String& text) const = 0;
    virtual int horizontalAdvance(const Font& font, const String& text) const = 0;
    virtual int height(const Font& font) const = 0;
    virtual int ascent(const Font& font) const = 0;
    virtual int lineSpacing(const Font& font) const { return height(font); }
};

} // namespace md::render
#endif // QTMARKDOWN_FONTMETRICSPROVIDER_H
