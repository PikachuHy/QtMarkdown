#ifndef QTMARKDOWN_DEFAULTFONTMETRICS_H
#define QTMARKDOWN_DEFAULTFONTMETRICS_H

#include "FontMetricsProvider.h"

namespace md::render {

class DefaultFontMetrics : public IFontMetricsProvider {
public:
    Size size(const Font& font, const String& text) const override {
        int charWidth = font.pixelSize * 3 / 5;
        if (charWidth < 1) charWidth = 1;
        return {static_cast<int>(text.size()) * charWidth, font.pixelSize};
    }
    int horizontalAdvance(const Font& font, const String& text) const override {
        int charWidth = font.pixelSize * 3 / 5;
        if (charWidth < 1) charWidth = 1;
        return static_cast<int>(text.size()) * charWidth;
    }
    int height(const Font& font) const override { return font.pixelSize; }
    int ascent(const Font& font) const override { return font.pixelSize * 4 / 5; }
};

inline DefaultFontMetrics g_defaultFontMetrics;

} // namespace md::render

#endif
