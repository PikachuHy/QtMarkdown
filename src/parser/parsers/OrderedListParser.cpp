//
// Created by pikachu on 2021/1/31.
//
// Parser for ordered lists (1. item)

#include "ParserDetail.h"

#include <cctype>

#include "Document.h"
#include "Text.h"
#include "debug.h"
#include "mddef.h"

namespace md::parser {

[[nodiscard]] ParseResult parseOrderedList(const LineList& lines, int startIndex) {
    if (startIndex >= lines.size()) return ParseResult::fail();
    auto line = lines[startIndex];
    if (!line.startsWith("1. ")) return ParseResult::fail();
    static std::vector<LineParserFn> parsers = {
        parseLink,
        parseInlineLatex,
        parseInlineCode,
        parseSemanticText,
    };
    int i = startIndex;
    auto ol = std::make_unique<OrderedList>();
    while (i < lines.size()) {
      auto line = lines[i];
      bool hasDigit = false;
      int j = 0;
      while (j < line.size() && std::isdigit(static_cast<unsigned char>(line[j]))) {
        hasDigit = true;
        j++;
      }
      if (!hasDigit) break;
      if (j >= line.size() || line[j] != '.') return {true, i - startIndex, std::move(ol)};
      j++;
      if (j >= line.size() || line[j] != ' ') return {true, i - startIndex, std::move(ol)};
      j++;
      auto item = std::make_unique<OrderedListItem>();
      _parseLine(item.get(), parsers, line.mid(j));
      ol->appendChild(std::move(item));
      i++;
    }
    skipEmptyLine(lines, i);
    return {true, i - startIndex, std::move(ol)};
}

}  // namespace md::parser
