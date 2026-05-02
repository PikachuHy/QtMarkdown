//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_PARSERDETAIL_H
#define MD_PARSER_PARSERDETAIL_H

#include "Node.h"
#include "Token.h"
#include "mddef.h"

#include <QDebug>
#include <QMap>
#include <functional>
#include <memory>
#include <vector>

namespace md::parser {

struct Line {
  Line(const String& text, qsizetype offset, qsizetype length) : text(text), offset(offset), length(length) {}
  Char operator[](qsizetype index) const { return text.at(offset + index); }
  [[nodiscard]] Char front() const { return text.at(offset); }
  [[nodiscard]] Char back() const { return text.at(offset + length - 1); }
  [[nodiscard]] bool startsWith(const String& s) const {
    return QStringView(text).mid(offset, length).startsWith(s);
  }
  [[nodiscard]] qsizetype size() const { return length; }
  [[nodiscard]] Line trimmed() const {
    Line line(text, offset, length);
    while (line.length > 0 && text.at(line.offset) == ' ') {
      line.offset++;
      line.length--;
    }
    while (line.length > 0 && text.at(line.offset + line.length - 1) == ' ') {
      line.length--;
    }
    return line;
  }
  [[nodiscard]] Line mid(qsizetype pos, qsizetype len = -1) const {
    if (len == -1) len = length - pos;
    return {text, offset + pos, len};
  }
  [[nodiscard]] Line right(qsizetype n) const { return {text, offset + length - n, n}; }
  const String& text;
  qsizetype offset;
  qsizetype length;
};

using LineList = std::vector<Line>;

QDebug operator<<(QDebug debug, const Line& line);
Line trimLeft(Line s);
std::vector<std::unique_ptr<Text>> mergeToText(const TokenList& tokens, int prev, int cur);

extern QMap<Char, TokenType> ch2type;
TokenList parseLine(Line text);

struct ParseResult {
  bool success;
  int offset;
  std::unique_ptr<Node> node;
  static ParseResult fail() { return {false}; }
};

using LineParserFn = std::function<ParseResult(const TokenList&, int startIndex)>;
using BlockParserFn = std::function<ParseResult(const LineList&, int startIndex)>;

void _parseLine(Container* ret, const std::vector<LineParserFn>& parsers, const Line& line);
void skipEmptyLine(const LineList& lines, int& i);

// Inline parser function declarations (defined in InlineParser.cpp)
ParseResult parseImage(const TokenList& tokens, int startIndex);
ParseResult parseLink(const TokenList& tokens, int startIndex);
ParseResult parseInlineCode(const TokenList& tokens, int startIndex);
ParseResult parseInlineLatex(const TokenList& tokens, int startIndex);
ParseResult parseSemanticText(const TokenList& tokens, int startIndex);

// Block parser function declarations (defined in BlockParser.cpp)
ParseResult parseHeader(const LineList& lines, int startIndex);
ParseResult parseCodeBlock(const LineList& lines, int startIndex);
ParseResult parseCheckboxList(const LineList& lines, int startIndex);
ParseResult parseUnorderedList(const LineList& lines, int startIndex);
ParseResult parseOrderedList(const LineList& lines, int startIndex);
ParseResult parseQuoteBlock(const LineList& lines, int startIndex);
ParseResult parseLatexBlock(const LineList& lines, int startIndex);
ParseResult parseParagraph(const LineList& lines, int startIndex);

}  // namespace md::parser

#endif  // MD_PARSER_PARSERDETAIL_H
