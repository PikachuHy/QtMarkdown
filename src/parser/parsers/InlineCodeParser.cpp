//
// Created by pikachu on 2021/1/31.
//
// Parser for inline code (`code`)

#include "ParserDetail.h"

#include <QRegularExpression>

#include "Document.h"
#include "Text.h"
#include "debug.h"
#include "mddef.h"

namespace md::parser {

[[nodiscard]] bool tryParseInlineCode(const TokenList& tokens, int startIndex) {
    if (!isBackquote(tokens[startIndex])) return false;
    int i = startIndex + 1;
    while (i < tokens.size()) {
      if (isBackquote(tokens[i])) {
        if (i > startIndex + 1) return true;
        return false;
      }
      i++;
    }
    return false;
}

[[nodiscard]] ParseResult parseInlineCode(const TokenList& tokens, int startIndex) {
    if (!tryParseInlineCode(tokens, startIndex)) return ParseResult::fail();
    int i = startIndex;
    i++;  // `
    int prev = i;
    while (!isBackquote(tokens[i])) i++;
    auto code = mergeToText(tokens, prev, i);
    i++;  // `
    return {true, i - startIndex, std::make_unique<InlineCode>(std::move(code[0]))};
}

}  // namespace md::parser
