//
// Created by pikachu on 2021/1/31.
//

#include "ParserDetail.h"

#include <QRegularExpression>

#include "Document.h"
#include "Text.h"
#include "debug.h"
#include "mddef.h"

namespace md::parser {

// 图片解析器
bool tryParseImage(const TokenList& tokens, int startIndex) {
    int i = startIndex;
    if (i >= tokens.size() || !isExclamation(tokens[i])) return false;
    i++;
    if (i >= tokens.size() || !isLeftBracket(tokens[i])) return false;
    i++;
    // 可选的图片文本内容
    while (i < tokens.size() && !isRightBracket(tokens[i])) {
      i++;
    }
    if (i >= tokens.size() || !isRightBracket(tokens[i])) return false;
    i++;
    if (i >= tokens.size() || !isLeftParenthesis(tokens[i])) return false;
    i++;
    // 必须要有url
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

// 链接解析器
bool tryParseLink(const TokenList& tokens, int startIndex) {
    int i = startIndex;
    if (i >= tokens.size() || !isLeftBracket(tokens[i])) return false;
    i++;
    // 必须要有链接文本内容
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
    // 可选的url
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

// 行内代码解析器
[[nodiscard]] bool tryParseInlineCode(const TokenList& tokens, int startIndex) {
    if (!isBackquote(tokens[startIndex])) return false;
    int i = startIndex + 1;
    while (i < tokens.size()) {
      // 至少有一个字符才触发InlineCode
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

// 行内公式解析器
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

// 加粗和斜体解析器
[[nodiscard]] bool tryParseItalic(const TokenList& tokens, int startIndex) {
    if (startIndex + 3 > tokens.size()) return false;
    auto offset_list = {0, 2};
    auto text_offset = 1;
    for (auto offset : offset_list) {
      if (!isStar(tokens[startIndex + offset])) return false;
    }
    return isText(tokens[startIndex + text_offset]);
}
[[nodiscard]] bool tryParseBold(const TokenList& tokens, int startIndex) {
    if (startIndex + 5 > tokens.size()) return false;
    auto offset_list = {0, 1, 3, 4};
    auto text_offset = 2;
    for (auto offset : offset_list) {
      if (!isStar(tokens[startIndex + offset])) return false;
    }
    return isText(tokens[startIndex + text_offset]);
}

[[nodiscard]] bool tryParseItalicAndBold(const TokenList& tokens, int startIndex) {
    if (startIndex + 7 > tokens.size()) return false;
    auto offset_list = {0, 1, 2, 4, 5, 6};
    auto text_offset = 3;
    for (auto offset : offset_list) {
      if (!isStar(tokens[startIndex + offset])) return false;
    }
    return isText(tokens[startIndex + text_offset]);
}

[[nodiscard]] bool tryParseStrickout(const TokenList& tokens, int startIndex) {
    if (startIndex + 5 > tokens.size()) return false;
    auto offset_list = {0, 1, 3, 4};
    auto text_offset = 2;
    for (auto offset : offset_list) {
      if (!isTilde(tokens[startIndex + offset])) return false;
    }
    return isText(tokens[startIndex + text_offset]);
}

ParseResult _parseItalic(const TokenList& tokens, int startIndex) {
    auto& token = tokens[startIndex + 1];
    auto text = std::make_unique<Text>(token.offset(), token.length());
    return {true, 3, std::make_unique<ItalicText>(std::move(text))};
}
ParseResult _parseBold(const TokenList& tokens, int startIndex) {
    auto& token = tokens[startIndex + 2];
    auto text = std::make_unique<Text>(token.offset(), token.length());
    return {true, 5, std::make_unique<BoldText>(std::move(text))};
}
ParseResult _parseItalicAndBold(const TokenList& tokens, int startIndex) {
    auto& token = tokens[startIndex + 3];
    auto text = std::make_unique<Text>(token.offset(), token.length());
    return {true, 7, std::make_unique<ItalicBoldText>(std::move(text))};
}
ParseResult _parseStrickout(const TokenList& tokens, int startIndex) {
    auto& token = tokens[startIndex + 2];
    auto text = std::make_unique<Text>(token.offset(), token.length());
    return {true, 5, std::make_unique<StrickoutText>(std::move(text))};
}

[[nodiscard]] ParseResult parseSemanticText(const TokenList& tokens, int startIndex) {
    if (tryParseItalicAndBold(tokens, startIndex)) {
      return _parseItalicAndBold(tokens, startIndex);
    }
    if (tryParseBold(tokens, startIndex)) {
      return _parseBold(tokens, startIndex);
    }
    if (tryParseItalic(tokens, startIndex)) {
      return _parseItalic(tokens, startIndex);
    }
    if (tryParseStrickout(tokens, startIndex)) {
      return _parseStrickout(tokens, startIndex);
    }
    return ParseResult::fail();
}

}  // namespace md::parser
