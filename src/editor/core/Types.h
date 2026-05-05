#ifndef QTMARKDOWN_CORE_TYPES_H
#define QTMARKDOWN_CORE_TYPES_H

#include <cstdint>
#include <string>
#include <vector>

namespace md::editor::core {

struct Point {
    int x = 0;
    int y = 0;
    constexpr Point() = default;
    constexpr Point(int x, int y) : x(x), y(y) {}
    constexpr Point operator+(const Point& other) const { return {x + other.x, y + other.y}; }
    constexpr Point& operator+=(const Point& other) { x += other.x; y += other.y; return *this; }
    constexpr Point operator-(const Point& other) const { return {x - other.x, y - other.y}; }
    constexpr bool operator==(const Point& other) const { return x == other.x && y == other.y; }
    constexpr bool operator!=(const Point& other) const { return !(*this == other); }
    void setX(int x_) { x = x_; }
    void setY(int y_) { y = y_; }
    int manhattanLength() const { return (x < 0 ? -x : x) + (y < 0 ? -y : y); }
};

struct Size {
    int width = 0;
    int height = 0;
    constexpr Size() = default;
    constexpr Size(int w, int h) : width(w), height(h) {}
    constexpr bool operator==(const Size& other) const { return width == other.width && height == other.height; }
    constexpr bool operator!=(const Size& other) const { return !(*this == other); }
};

struct Rect {
    Point pos;
    Size size;
    constexpr Rect() = default;
    constexpr Rect(const Point& p, const Size& s) : pos(p), size(s) {}
    constexpr Rect(int x, int y, int w, int h) : pos(x, y), size(w, h) {}
    constexpr int x() const { return pos.x; }
    constexpr int y() const { return pos.y; }
    constexpr int width() const { return size.width; }
    constexpr int height() const { return size.height; }
    constexpr bool contains(const Point& p) const {
        return p.x >= pos.x && p.x <= pos.x + size.width
            && p.y >= pos.y && p.y <= pos.y + size.height;
    }
    constexpr bool isEmpty() const { return size.width <= 0 || size.height <= 0; }
};

struct Color {
    int r = 0, g = 0, b = 0, a = 255;
    constexpr Color() = default;
    constexpr Color(int r, int g, int b, int a = 255) : r(r), g(g), b(b), a(a) {}
    constexpr bool operator==(const Color& other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
    constexpr bool operator!=(const Color& other) const { return !(*this == other); }
    static constexpr Color black() { return {0, 0, 0, 255}; }
    static constexpr Color white() { return {255, 255, 255, 255}; }
    static constexpr Color red() { return {255, 0, 0, 255}; }
    static constexpr Color green() { return {0, 255, 0, 255}; }
    static constexpr Color blue() { return {0, 0, 255, 255}; }
    static constexpr Color yellow() { return {255, 255, 0, 255}; }
    static constexpr Color transparent() { return {0, 0, 0, 0}; }
};

struct Margins {
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
    constexpr Margins() = default;
    constexpr Margins(int l, int t, int r, int b) : left(l), top(t), right(r), bottom(b) {}
};

struct FontDescription {
    std::string family;
    int pixelSize = 16;
    bool bold = false;
    bool italic = false;
    bool underline = false;
    bool strikeOut = false;
};

struct ImageData {
    int width = 0;
    int height = 0;
    std::vector<unsigned char> pixels;  // RGBA, row-major
    bool isNull() const { return pixels.empty(); }
};

} // namespace md::editor::core
#endif // QTMARKDOWN_CORE_TYPES_H
