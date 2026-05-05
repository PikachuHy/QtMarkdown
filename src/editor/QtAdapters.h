#ifndef QTMARKDOWN_QTADAPTERS_H
#define QTMARKDOWN_QTADAPTERS_H

#include <QColor>
#include <QEvent>
#include <QFont>
#include <QImage>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QTimer>

#include "core/Event.h"
#include "core/Types.h"
#include "core/AbstractPainter.h"
#include "core/Timer.h"

namespace md::editor {

// -- Value type conversions --
inline core::Point fromQPoint(const QPoint& p) { return {p.x(), p.y()}; }
inline QPoint toQPoint(const core::Point& p) { return {p.x, p.y}; }
inline core::Size fromQSize(const QSize& s) { return {s.width(), s.height()}; }
inline QSize toQSize(const core::Size& s) { return {s.width, s.height}; }
inline core::Rect fromQRect(const QRect& r) { return {{r.x(), r.y()}, {r.width(), r.height()}}; }
inline QRect toQRect(const core::Rect& r) { return {r.pos.x, r.pos.y, r.size.width, r.size.height}; }
inline core::Color fromQColor(const QColor& c) { return {c.red(), c.green(), c.blue(), c.alpha()}; }
inline QColor toQColor(const core::Color& c) { return QColor(c.r, c.g, c.b, c.a); }

// -- FontDescription <-> QFont --
inline QString toQString(const String& s) {
    return QString::fromUtf8(s.data(), static_cast<int>(s.size()));
}
inline String fromQString(const QString& s) {
    return String(s.toUtf8().constData());
}

inline QFont toQFont(const core::FontDescription& fd) {
    QFont font(toQString(fd.family), fd.pixelSize);
    font.setPixelSize(fd.pixelSize);
    font.setBold(fd.bold);
    font.setItalic(fd.italic);
    font.setStrikeOut(fd.strikeOut);
    font.setUnderline(fd.underline);
    return font;
}

inline QImage toQImage(const core::ImageData& img) {
    if (img.isNull()) return {};
    if (img.pixels.empty()) return {};
    QImage qimg(img.pixels.data(), img.width, img.height, img.width * 4, QImage::Format_RGBA8888);
    return qimg.copy(); // deep copy since we don't own the data
}

inline core::ImageData fromQImage(const QImage& img) {
    if (img.isNull()) return {};
    QImage converted = img.convertToFormat(QImage::Format_RGBA8888);
    core::ImageData data;
    data.width = converted.width();
    data.height = converted.height();
    data.pixels.resize(static_cast<size_t>(converted.width() * converted.height() * 4));
    std::memcpy(data.pixels.data(), converted.constBits(), data.pixels.size());
    return data;
}

// -- QtKeyEvent adapter --
class QtKeyEvent : public core::KeyEvent {
public:
    explicit QtKeyEvent(const QKeyEvent* e) : m_event(e) {}
    core::Key key() const override;
    core::Modifier modifiers() const override;
    std::string text() const override;
    bool isAutoRepeat() const override;
private:
    const QKeyEvent* m_event;
};

inline core::Key QtKeyEvent::key() const {
    switch (m_event->key()) {
    case Qt::Key_Tab: return core::Key::Tab;
    case Qt::Key_Escape: return core::Key::Escape;
    case Qt::Key_Return: return core::Key::Return;
    case Qt::Key_Backspace: return core::Key::Backspace;
    case Qt::Key_Left: return core::Key::Left;
    case Qt::Key_Right: return core::Key::Right;
    case Qt::Key_Up: return core::Key::Up;
    case Qt::Key_Down: return core::Key::Down;
    case Qt::Key_Control: return core::Key::Key_Control;
    case Qt::Key_Shift: return core::Key::Key_Shift;
    default:
        if (m_event->key() >= Qt::Key_A && m_event->key() <= Qt::Key_Z)
            return static_cast<core::Key>(m_event->key());
        if (m_event->key() >= Qt::Key_0 && m_event->key() <= Qt::Key_9)
            return static_cast<core::Key>(m_event->key());
        return core::Key::Unknown;
    }
}
inline core::Modifier QtKeyEvent::modifiers() const {
    auto qtMods = m_event->modifiers();
    core::Modifier mods = core::Modifier::None;
    if (qtMods & Qt::ControlModifier) mods = mods | core::Modifier::Ctrl;
    if (qtMods & Qt::ShiftModifier)   mods = mods | core::Modifier::Shift;
    if (qtMods & Qt::AltModifier)     mods = mods | core::Modifier::Alt;
    if (qtMods & Qt::MetaModifier)    mods = mods | core::Modifier::Meta;
    if (qtMods & Qt::KeypadModifier)  mods = mods | core::Modifier::Keypad;
    if (qtMods & Qt::GroupSwitchModifier) mods = mods | core::Modifier::GroupSwitch;
    return mods;
}
inline std::string QtKeyEvent::text() const {
    return m_event->text().toStdString();
}
inline bool QtKeyEvent::isAutoRepeat() const {
    return m_event->isAutoRepeat();
}

// -- QtMouseEvent adapter --
class QtMouseEvent : public core::MouseEvent {
public:
    explicit QtMouseEvent(const QMouseEvent* e) : m_event(e) {}
    core::MouseEventType type() const override;
    core::Point pos() const override;
    core::MouseButton button() const override;
    core::Modifier modifiers() const override;
private:
    const QMouseEvent* m_event;
};

inline core::MouseEventType QtMouseEvent::type() const {
    switch (m_event->type()) {
    case QEvent::MouseButtonPress: return core::MouseEventType::Press;
    case QEvent::MouseButtonRelease: return core::MouseEventType::Release;
    case QEvent::MouseMove: return core::MouseEventType::Move;
    default: return core::MouseEventType::Move;
    }
}
inline core::Point QtMouseEvent::pos() const {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return fromQPoint(m_event->pos());
#else
    return fromQPoint(m_event->position().toPoint());
#endif
}
inline core::MouseButton QtMouseEvent::button() const {
    auto qtBtn = m_event->button();
    if (qtBtn == Qt::LeftButton) return core::MouseButton::Left;
    if (qtBtn == Qt::RightButton) return core::MouseButton::Right;
    if (qtBtn == Qt::MiddleButton) return core::MouseButton::Middle;
    return core::MouseButton::None;
}
inline core::Modifier QtMouseEvent::modifiers() const {
    auto qtMods = m_event->modifiers();
    core::Modifier mods = core::Modifier::None;
    if (qtMods & Qt::ControlModifier) mods = mods | core::Modifier::Ctrl;
    if (qtMods & Qt::ShiftModifier)   mods = mods | core::Modifier::Shift;
    if (qtMods & Qt::AltModifier)     mods = mods | core::Modifier::Alt;
    if (qtMods & Qt::MetaModifier)    mods = mods | core::Modifier::Meta;
    return mods;
}

// -- Qt Painter adapter --
class QtPainterAdapter : public core::AbstractPainter {
public:
    explicit QtPainterAdapter(QPainter* painter) : m_painter(painter) {}
    void save() override { m_painter->save(); }
    void restore() override { m_painter->restore(); }
    void setPen(const core::Color& color) override { m_painter->setPen(toQColor(color)); }
    void drawRect(const core::Rect& rect) override { m_painter->drawRect(toQRect(rect)); }
    void drawText(const core::Point& pos, const String& text) override { m_painter->drawText(toQPoint(pos), toQString(text)); }
    void drawLine(const core::Point& p1, const core::Point& p2) override { m_painter->drawLine(toQPoint(p1), toQPoint(p2)); }
    void setFont(const core::FontDescription& font) override { m_painter->setFont(toQFont(font)); }
    void fillRect(const core::Rect& rect, const core::Color& color) override { m_painter->fillRect(toQRect(rect), toQColor(color)); }
    void drawEllipse(const core::Rect& rect, const core::Color& color) override {
        m_painter->setBrush(QBrush(toQColor(color)));
        m_painter->drawEllipse(toQRect(rect));
    }
    void drawImage(const core::Rect& rect, const core::ImageData& image) override {
        m_painter->drawPixmap(toQRect(rect), QPixmap::fromImage(toQImage(image)));
    }
    void drawText(const core::Rect& rect, int flags, const String& text) override {
        m_painter->drawText(toQRect(rect), flags, toQString(text));
    }
    // Access the underlying QPainter for instruction dispatch (needed by LatexInstruction)
    void* nativePainter() const override { return m_painter; }
    QPainter* underlyingPainter() const { return m_painter; }
private:
    QPainter* m_painter;
};

// -- Qt Timer adapter --
class QtTimerAdapter : public core::ITimer {
public:
    explicit QtTimerAdapter(QObject* parent = nullptr);
    void start(int intervalMs) override;
    void stop() override;
    void setCallback(std::function<void()> callback) override;
    bool isRunning() const override;
private:
    QTimer m_timer;
};

} // namespace md::editor
#endif // QTMARKDOWN_QTADAPTERS_H
