#ifndef QTMARKDOWN_PLATFORM_QTFONTMETRICSPROVIDER_H
#define QTMARKDOWN_PLATFORM_QTFONTMETRICSPROVIDER_H

#include "render/FontMetricsProvider.h"
#include <QFont>
#include <QFontMetrics>

namespace md::render {

// Production implementation -- wraps QFontMetrics.
// Stateless -- safe to use as a static singleton or local variable.
class QtFontMetricsProvider : public IFontMetricsProvider {
public:
    Size size(const Font& font, const String& text) const override {
        QFontMetrics fm(toQFont(font));
        auto s = fm.size(Qt::TextSingleLine, toQString(text));
        return {s.width(), s.height()};
    }
    int horizontalAdvance(const Font& font, const String& text) const override {
        QFontMetrics fm(toQFont(font));
        return fm.horizontalAdvance(toQString(text));
    }
    int height(const Font& font) const override {
        QFontMetrics fm(toQFont(font));
        return fm.height();
    }

private:
    static QString toQString(const String& s) {
        return QString::fromUtf8(s.data(), static_cast<int>(s.size()));
    }
    static QFont toQFont(const Font& fd) {
        QFont font;
        if (!fd.family.empty()) {
            font.setFamily(toQString(fd.family));
        }
        font.setPixelSize(fd.pixelSize);
        font.setBold(fd.bold);
        font.setItalic(fd.italic);
        font.setStrikeOut(fd.strikeOut);
        font.setUnderline(fd.underline);
        return font;
    }
};

inline QtFontMetricsProvider g_defaultFontMetrics;

} // namespace md::render
#endif // QTMARKDOWN_PLATFORM_QTFONTMETRICSPROVIDER_H
