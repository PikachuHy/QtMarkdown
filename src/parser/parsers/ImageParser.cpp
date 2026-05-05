//
// Created by pikachu on 2021/1/31.
//
// Parser for images (![alt](url))

#include "ParserDetail.h"

#include "Document.h"
#include "Text.h"
#include "debug.h"
#include "mddef.h"

namespace md::parser {

bool tryParseImage(const TokenList& tokens, int startIndex) {
    int i = startIndex;
    if (i >= tokens.size() || !isExclamation(tokens[i])) return false;
    i++;
    if (i >= tokens.size() || !isLeftBracket(tokens[i])) return false;
    i++;
    while (i < tokens.size() && !isRightBracket(tokens[i])) {
      i++;
    }
    if (i >= tokens.size() || !isRightBracket(tokens[i])) return false;
    i++;
    if (i >= tokens.size() || !isLeftParenthesis(tokens[i])) return false;
    i++;
    bool hasUrl = false;
    while (i < tokens.size() && !isRightParenthesis(tokens[i])) {
      hasUrl = true;
      i++;
    }
    if (!hasUrl) return false;
    if (i >= tokens.size() || !isRightParenthesis(tokens[i])) return false;
    return true;
}

ParseResult parseImage(const TokenList& tokens, int startIndex) {
    if (!tryParseImage(tokens, startIndex)) return ParseResult::fail();
    int i = startIndex;
    i++;  // !
    i++;  // [
    int prev = i;
    while (!isRightBracket(tokens[i])) i++;
    auto alt = mergeToText(tokens, prev, i);
    i++;  // ]
    i++;  // (
    prev = i;
    while (!isRightParenthesis(tokens[i])) i++;
    auto url = mergeToText(tokens, prev, i);
    i++;  // )
    return {true, i - startIndex, std::make_unique<Image>(
        alt.empty() ? nullptr : std::move(alt[0]),
        std::move(url[0])
    )};
}

}  // namespace md::parser
