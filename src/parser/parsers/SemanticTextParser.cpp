//
// Created by pikachu on 2021/1/31.
//
// Parser for bold, italic, bold-italic, and strikethrough text

#include "ParserDetail.h"

#include "Document.h"
#include "Text.h"
#include "debug.h"
#include "mddef.h"

namespace md::parser {

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
