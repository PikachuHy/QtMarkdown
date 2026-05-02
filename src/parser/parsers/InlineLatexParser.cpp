//
// Created by pikachu on 2021/1/31.
//
// Parser for inline LaTeX ($...$)

#include "ParserDetail.h"

#include <QRegularExpression>

#include "Document.h"
#include "Text.h"
#include "debug.h"
#include "mddef.h"

namespace md::parser {

[[nodiscard]] bool tryParseInlineLatex(const TokenList& tokens, int startIndex) {
    if (startIndex + 2 >= tokens.size()) return false;
    if (!isDollar(tokens[startIndex])) return false;
    for (int i = startIndex + 2; i < tokens.size(); ++i) {
      if (isDollar(tokens[i])) return true;
    }
    return false;
}

[[nodiscard]] ParseResult parseInlineLatex(const TokenList& tokens, int startIndex) {
    if (!tryParseInlineLatex(tokens, startIndex)) return ParseResult::fail();
    auto& token = tokens[startIndex + 1];
    int endIndex = startIndex + 1;
    int length = 0;
    while (endIndex < tokens.size()) {
      if (isDollar(tokens[endIndex])) {
        break;
      }
      length += tokens[endIndex].length();
      endIndex++;
    }
    auto code = std::make_unique<Text>(token.offset(), length);
    return {true, endIndex - startIndex + 1, std::make_unique<InlineLatex>(std::move(code))};
}

}  // namespace md::parser
