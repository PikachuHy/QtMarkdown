#ifndef QTMARKDOWN_SIMPLEFONTMETRICSPROVIDER_H
#define QTMARKDOWN_SIMPLEFONTMETRICSPROVIDER_H

#include "core/Utf8Util.h"
#include "render/FontMetricsProvider.h"

namespace md::render {

// Predictable metrics for tests -- no QGuiApplication needed.
// ASCII chars: pixelSize x 0.5 wide, CJK/other: pixelSize wide, height: pixelSize x 1.5
class SimpleFontMetricsProvider : public IFontMetricsProvider {
public:
    editor::core::Size size(const editor::core::FontDescription& font, const String& text) const override {
        int pixelSize = font.pixelSize;
        int w = 0;
        for (size_t i = 0; i < text.size(); ) {
            auto seqLen = md::utf8SequenceLength(text[i]);
            w += (seqLen == 1 && static_cast<unsigned char>(text[i]) < 128) ? (pixelSize / 2) : pixelSize;
            i += seqLen;
        }
        int h = pixelSize * 3 / 2;
        return editor::core::Size(w, h);
    }
    int horizontalAdvance(const editor::core::FontDescription& font, const String& text) const override {
        return size(font, text).width;
    }
    int height(const editor::core::FontDescription& font) const override {
        return font.pixelSize * 3 / 2;
    }
};

} // namespace md::render
#endif // QTMARKDOWN_SIMPLEFONTMETRICSPROVIDER_H
