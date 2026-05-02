//
// Created by pikachu on 2021/1/31.
//
// Parser for unordered lists (- item)

#include "ParserDetail.h"

#include <QRegularExpression>

#include "Document.h"
#include "Text.h"
#include "debug.h"
#include "mddef.h"

namespace md::parser {

[[nodiscard]] ParseResult parseUnorderedList(const LineList& lines, int startIndex) {
    if (startIndex >= lines.size()) return ParseResult::fail();
    auto line = trimLeft(lines[startIndex]);
    if (!line.startsWith("- ")) return ParseResult::fail();
    static std::vector<LineParserFn> parsers = {
        parseLink,
        parseInlineLatex,
        parseInlineCode,
        parseSemanticText,
    };
    int i = startIndex;
    auto ul = std::make_unique<UnorderedList>();
    while (i < lines.size() && trimLeft(lines[i]).startsWith("- ")) {
      auto item = std::make_unique<UnorderedListItem>();
      _parseLine(item.get(), parsers, trimLeft(lines[i]).mid(2));
      ul->appendChild(std::move(item));
      i++;
    }
    skipEmptyLine(lines, i);
    return {true, i - startIndex, std::move(ul)};
}

}  // namespace md::parser
