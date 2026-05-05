#ifndef QTMARKDOWN_FONTMETRICSPROVIDER_H
#define QTMARKDOWN_FONTMETRICSPROVIDER_H

#include "mddef.h"
#include "QtMarkdown_global.h"
#include "core/Types.h"
#include <QFont>
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
        QFont font(toQString(fd.family), fd.pixelSize);
        font.setPixelSize(fd.pixelSize);
        font.setBold(fd.bold);
        font.setItalic(fd.italic);
        font.setStrikeOut(fd.strikeOut);
        font.setUnderline(fd.underline);
        return font;
    }
};

} // namespace md::render
#endif // QTMARKDOWN_FONTMETRICSPROVIDER_H
