#ifndef QTMARKDOWN_SIMPLEFONTMETRICSPROVIDER_H
#define QTMARKDOWN_SIMPLEFONTMETRICSPROVIDER_H

#include "render/FontMetricsProvider.h"

namespace md::render {

// Predictable metrics for tests -- no QGuiApplication needed.
// ASCII chars: pixelSize x 0.5 wide, CJK/other: pixelSize wide, height: pixelSize x 1.5
class SimpleFontMetricsProvider : public IFontMetricsProvider {
public:
    Size size(const Font& font, const String& text) const override {
        int pixelSize = font.pixelSize();
        int w = 0;
        for (const QChar& ch : text) {
            w += (ch.unicode() < 128) ? (pixelSize / 2) : pixelSize;
        }
        int h = pixelSize * 3 / 2;
        return Size(w, h);
    }
    int horizontalAdvance(const Font& font, const String& text) const override {
        return size(font, text).width();
    }
    int height(const Font& font) const override {
        return font.pixelSize() * 3 / 2;
    }
};

} // namespace md::render
#endif // QTMARKDOWN_SIMPLEFONTMETRICSPROVIDER_H
