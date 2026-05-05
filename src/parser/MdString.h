//
// Qt-free UTF-8 String class for QtMarkdown Parser
//

#ifndef QTMARKDOWN_MDSTRING_H
#define QTMARKDOWN_MDSTRING_H

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "../core/Utf8Util.h"

namespace md {

class String {
public:
    using size_type = std::string::size_type;
    static constexpr size_type npos = std::string::npos;

    String() = default;
    String(const String&) = default;
    String(String&&) noexcept = default;
    String& operator=(const String&) = default;
    String& operator=(String&&) noexcept = default;
    ~String() = default;

    // Implicit conversion from const char* and std::string
    // NOLINTNEXTLINE(google-explicit-constructor)
    String(const char* s) : m_str(s) {}
    // NOLINTNEXTLINE(google-explicit-constructor)
    String(const std::string& s) : m_str(s) {}
    // NOLINTNEXTLINE(google-explicit-constructor)
    String(std::string&& s) noexcept : m_str(std::move(s)) {}
    // NOLINTNEXTLINE(google-explicit-constructor)
    String(char ch) : m_str(1, ch) {}

    // Size
    [[nodiscard]] size_type size() const noexcept { return m_str.size(); }
    [[nodiscard]] size_type length() const noexcept { return m_str.size(); }
    [[nodiscard]] bool empty() const noexcept { return m_str.empty(); }
    [[nodiscard]] bool isEmpty() const noexcept { return m_str.empty(); }

    // Element access
    [[nodiscard]] char operator[](size_type i) const noexcept { return m_str[i]; }
    [[nodiscard]] char& operator[](size_type i) noexcept { return m_str[i]; }
    [[nodiscard]] char at(size_type i) const { return m_str.at(i); }
    [[nodiscard]] char& at(size_type i) { return m_str.at(i); }
    [[nodiscard]] const char* c_str() const noexcept { return m_str.c_str(); }
    [[nodiscard]] const char* data() const noexcept { return m_str.data(); }
    [[nodiscard]] char* data() noexcept { return m_str.data(); }
    [[nodiscard]] char front() const { return m_str.front(); }
    [[nodiscard]] char& front() { return m_str.front(); }
    [[nodiscard]] char back() const { return m_str.back(); }
    [[nodiscard]] char& back() { return m_str.back(); }

    // Iterators
    using iterator = std::string::iterator;
    using const_iterator = std::string::const_iterator;
    iterator begin() noexcept { return m_str.begin(); }
    iterator end() noexcept { return m_str.end(); }
    [[nodiscard]] const_iterator begin() const noexcept { return m_str.begin(); }
    [[nodiscard]] const_iterator end() const noexcept { return m_str.end(); }

    // Substring
    [[nodiscard]] String mid(size_type pos, size_type len = npos) const {
        if (pos > m_str.size()) return String();
        return String(m_str.substr(pos, len));
    }
    [[nodiscard]] String left(size_type n) const {
        return String(m_str.substr(0, n));
    }

    // Prefix / suffix checks
    [[nodiscard]] bool startsWith(const String& s) const {
        if (s.size() > m_str.size()) return false;
        return m_str.compare(0, s.size(), s.m_str) == 0;
    }
    [[nodiscard]] bool startsWith(char c) const {
        return !m_str.empty() && m_str[0] == c;
    }
    [[nodiscard]] bool endsWith(const String& s) const {
        if (s.size() > m_str.size()) return false;
        return m_str.compare(m_str.size() - s.size(), s.size(), s.m_str) == 0;
    }
    [[nodiscard]] bool endsWith(char c) const {
        return !m_str.empty() && m_str.back() == c;
    }

    // Substring comparison (for Line::startsWith with offset)
    [[nodiscard]] int compare(size_type pos, size_type n, const String& s) const {
        return m_str.compare(pos, n, s.m_str);
    }

    // Search
    [[nodiscard]] bool contains(char c) const {
        return m_str.find(c) != std::string::npos;
    }
    [[nodiscard]] size_type count(char c) const {
        size_type cnt = 0;
        for (char ch : m_str) {
            if (ch == c) ++cnt;
        }
        return cnt;
    }

    // Split
    [[nodiscard]] std::vector<String> split(char delim) const {
        std::vector<String> result;
        size_type start = 0;
        while (start < m_str.size()) {
            auto end = m_str.find(delim, start);
            if (end == std::string::npos) {
                result.emplace_back(m_str.substr(start));
                break;
            }
            result.emplace_back(m_str.substr(start, end - start));
            start = end + 1;
        }
        return result;
    }

    // Concatenation
    String& operator+=(const String& rhs) { m_str += rhs.m_str; return *this; }
    String& operator+=(const char* rhs) { m_str += rhs; return *this; }
    String& operator+=(char rhs) { m_str += rhs; return *this; }

    friend String operator+(String lhs, const String& rhs) { lhs += rhs; return lhs; }
    friend String operator+(String lhs, const char* rhs) { lhs += rhs; return lhs; }
    friend String operator+(const char* lhs, const String& rhs) { return String(lhs) + rhs; }

    // Comparison
    [[nodiscard]] bool operator==(const String& rhs) const noexcept { return m_str == rhs.m_str; }
    [[nodiscard]] bool operator!=(const String& rhs) const noexcept { return m_str != rhs.m_str; }
    [[nodiscard]] bool operator<(const String& rhs) const noexcept { return m_str < rhs.m_str; }

    // Capacity
    void reserve(size_type n) { m_str.reserve(n); }

    // Append
    void append(const String& s) { m_str.append(s.m_str); }
    void append(char c) { m_str.push_back(c); }

    // Code point at position
    [[nodiscard]] char32_t codePointAt(size_type pos) const {
        return md::codePointAt(m_str, pos);
    }

    // Qt bridge: call .data()/.size() with QString::fromUtf8()
    // toQString() is NOT provided here; use the helper in the render/editor layer.

    // Access underlying std::string
    [[nodiscard]] const std::string& toStdString() const noexcept { return m_str; }
    std::string& toStdString() noexcept { return m_str; }

    // erase (needed by some algorithms like std::reverse)
    iterator erase(iterator first, iterator last) { return m_str.erase(first, last); }

    // Stream output
    friend std::ostream& operator<<(std::ostream& os, const String& s) {
        os << s.m_str;
        return os;
    }

private:
    std::string m_str;
};

} // namespace md

#endif // QTMARKDOWN_MDSTRING_H
