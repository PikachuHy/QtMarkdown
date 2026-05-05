//
// Created by pikachu on 2021/1/31.
//
// Parser for links ([text](url))

#include "ParserDetail.h"

#include "Document.h"
#include "Text.h"
#include "debug.h"
#include "mddef.h"

namespace md::parser {

bool tryParseLink(const TokenList& tokens, int startIndex) {
    int i = startIndex;
    if (i >= tokens.size() || !isLeftBracket(tokens[i])) return false;
    i++;
    bool hasLinkText = false;
    while (i < tokens.size() && !isRightBracket(tokens[i])) {
      i++;
      hasLinkText = true;
    }
    if (!hasLinkText) return false;
    if (i >= tokens.size() || !isRightBracket(tokens[i])) return false;
    i++;
    if (i >= tokens.size() || !isLeftParenthesis(tokens[i])) return false;
    i++;
    while (i < tokens.size() && !isRightParenthesis(tokens[i])) {
      i++;
    }
    if (i >= tokens.size() || !isRightParenthesis(tokens[i])) return false;
    return true;
}

ParseResult parseLink(const TokenList& tokens, int startIndex) {
    if (!tryParseLink(tokens, startIndex)) return ParseResult::fail();
    int i = startIndex;
    i++;  // [
    int prev = i;
    while (!isRightBracket(tokens[i])) i++;
    auto content = mergeToText(tokens, prev, i);
    i++;  // ]
    i++;  // (
    prev = i;
    while (!isRightParenthesis(tokens[i])) i++;
    auto href = mergeToText(tokens, prev, i);
    i++;  // )
    return {true, i - startIndex, std::make_unique<Link>(std::move(content[0]), std::move(href[0]))};
}

}  // namespace md::parser
