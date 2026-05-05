//
// UTF-8 utility functions for QtMarkdown
//

#ifndef QTMARKDOWN_UTF8UTIL_H
#define QTMARKDOWN_UTF8UTIL_H

#include <cstddef>
#include <string>

namespace md {

// Determine the byte length of a UTF-8 sequence from its lead byte.
constexpr int utf8SequenceLength(char lead) {
    unsigned char uc = static_cast<unsigned char>(lead);
    if ((uc & 0x80) == 0) return 1;
    if ((uc & 0xE0) == 0xC0) return 2;
    if ((uc & 0xF0) == 0xE0) return 3;
    if ((uc & 0xF8) == 0xF0) return 4;
    return 1; // invalid byte, treat as single byte
}

// Decode the UTF-8 code point at byte position pos in string s.
// Returns U+FFFD (0xFFFD) on invalid sequence.
inline char32_t codePointAt(const std::string& s, size_t pos) {
    if (pos >= s.size()) return 0xFFFD;
    unsigned char lead = static_cast<unsigned char>(s[pos]);
    int len = utf8SequenceLength(static_cast<char>(lead));
    if (len == 1) return lead;
    if (pos + len > s.size()) return 0xFFFD;
    char32_t cp;
    switch (len) {
    case 2:
        cp = (lead & 0x1F) << 6;
        cp |= (static_cast<unsigned char>(s[pos + 1]) & 0x3F);
        break;
    case 3:
        cp = (lead & 0x0F) << 12;
        cp |= (static_cast<unsigned char>(s[pos + 1]) & 0x3F) << 6;
        cp |= (static_cast<unsigned char>(s[pos + 2]) & 0x3F);
        break;
    case 4:
        cp = (lead & 0x07) << 18;
        cp |= (static_cast<unsigned char>(s[pos + 1]) & 0x3F) << 12;
        cp |= (static_cast<unsigned char>(s[pos + 2]) & 0x3F) << 6;
        cp |= (static_cast<unsigned char>(s[pos + 3]) & 0x3F);
        break;
    default:
        return 0xFFFD;
    }
    return cp;
}

// Walk backward from pos (exclusive) to find the start byte of the previous
// code point. Returns the byte index of that start byte.
// Assumes pos > 0.
inline size_t previousCodePointStart(const std::string& s, size_t pos) {
    if (pos == 0) return 0;
    size_t p = pos - 1;
    while (p > 0 && (static_cast<unsigned char>(s[p]) & 0xC0) == 0x80) {
        --p;
    }
    return p;
}

} // namespace md

#endif // QTMARKDOWN_UTF8UTIL_H
