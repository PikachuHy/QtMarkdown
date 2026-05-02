//
// Created by pikachu on 2021/1/31.
//
// Parser for LaTeX math blocks ($$...$$)

#include "ParserDetail.h"

#include <QRegularExpression>

#include "Document.h"
#include "Text.h"
#include "debug.h"
#include "mddef.h"

namespace md::parser {

[[nodiscard]] ParseResult parseLatexBlock(const LineList& lines, int startIndex) {
    if (startIndex >= lines.size() || !lines[startIndex].startsWith("$$")) return ParseResult::fail();
    int i = startIndex + 1;
    while (i < lines.size()) {
      if (lines[i].startsWith("$$")) {
        break;
      }
      i++;
    }
    if (i == startIndex + 1 || i == lines.size()) return ParseResult::fail();
    auto latexBlock = std::make_unique<LatexBlock>();
    for (int j = startIndex + 1; j < i; j++) {
      latexBlock->appendChild(std::make_unique<Text>(lines[j].offset, lines[j].length));
      latexBlock->appendChild(std::make_unique<Lf>());
    }
    i++;  // last $$
    return {true, i - startIndex, std::move(latexBlock)};
}

}  // namespace md::parser
