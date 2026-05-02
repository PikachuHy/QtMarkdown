//
// Created by pikachu on 2021/1/31.
//
// Parser for headers (#, ##, ###, etc.)

#include "ParserDetail.h"

#include <QRegularExpression>

#include "Document.h"
#include "Text.h"
#include "debug.h"
#include "mddef.h"

namespace md::parser {

[[nodiscard]] bool tryParseHeader(const Line& line) {
    int i = 0;
    while (i < line.size() && line[i] == '#') i++;
    // 如果没有#或#的个数超过6个
    if (i == 0 || i > 6) return false;
    // #后面要接一个空格
    if (i < line.size() && line[i] == ' ') return true;
    return false;
}

[[nodiscard]] std::unique_ptr<Node> parseHeaderContent(const Line& line) {
    static std::vector<LineParserFn> parsers = {
        parseLink,
        parseInlineCode,
        parseInlineLatex,
    };
    int i = 0;
    while (i < line.size() && line[i] == '#') i++;
    int level = i;
    // 跳过空格
    i++;
    Line str = line.mid(i);
    auto header = std::make_unique<Header>(level);
    _parseLine(header.get(), parsers, str);
    return header;
}

[[nodiscard]] ParseResult parseHeader(const LineList& lines, int startIndex) {
    if (startIndex >= lines.size()) return ParseResult::fail();
    auto line = trimLeft(lines[startIndex]);
    if (!tryParseHeader(line)) return ParseResult::fail();
    auto header = parseHeaderContent(line);
    int i = startIndex + 1;
    skipEmptyLine(lines, i);
    return {true, i - startIndex, std::move(header)};
}

}  // namespace md::parser
