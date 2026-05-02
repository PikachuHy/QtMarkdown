//
// Created by pikachu on 2021/1/31.
//
// Parser for quote blocks (> text)

#include "ParserDetail.h"

#include <QRegularExpression>

#include "Document.h"
#include "Text.h"
#include "debug.h"
#include "mddef.h"

namespace md::parser {

[[nodiscard]] ParseResult parseQuoteBlock(const LineList& lines, int startIndex) {
    if (startIndex >= lines.size()) return ParseResult::fail();
    auto line = lines[startIndex];
    if (!line.startsWith("> ")) return ParseResult::fail();
    int i = startIndex;
    auto quoteBlock = std::make_unique<QuoteBlock>();
    while (i < lines.size() && lines[i].startsWith("> ")) {
      auto& line = lines[i];
      quoteBlock->appendChild(std::make_unique<Text>(line.offset + 2, line.length - 2));
      i++;
    }
    i++;
    skipEmptyLine(lines, i);
    return {true, i - startIndex, std::move(quoteBlock)};
}

}  // namespace md::parser
