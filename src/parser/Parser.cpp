//
// Created by pikachu on 2021/1/31.
//

#include "Parser.h"

#include <QRegularExpression>
#include <vector>

#include "Document.h"
#include "Text.h"
#include "debug.h"
#include "mddef.h"
using namespace std::string_view_literals;
namespace md::parser {
struct Line {
  Line(const String& text, qsizetype offset, qsizetype length) : text(text), offset(offset), length(length) {}
  Char operator[](qsizetype index) const {
    if (index < 0 || index >= length) {
      DEBUG << "ERROR index" << index << offset << length;
    }
    return text.at(offset + index);
  }
  [[nodiscard]] Char front() const { return text.at(offset); }
  [[nodiscard]] Char back() const { return text.at(offset + length - 1); }
  [[nodiscard]] bool startsWith(const String& s) const { return QStringView(text).mid(offset, length).startsWith(s); }
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
QDebug operator<<(QDebug debug, const Line& line) {
  QDebugStateSaver saver(debug);
  debug << line.text.mid(line.offset, line.length);

  return debug;
}
Line trimLeft(Line s) {
  int count = 0;
  while (count < s.length && s[count] == ' ') {
    count++;
  }
  if (count == 0) return s;
  s.offset += count;
  return s;
}
QList<Text*> mergeToText(const TokenList& tokens, int prev, int cur) {
  if (prev >= cur) return {};
  QList<Text*> texts;
  bool hasVal = false;
  qsizetype offset;
  qsizetype length;
  for (int i = prev; i < cur; i++) {
    if (!hasVal) {
      offset = tokens[i].offset();
      length = tokens[i].length();
      hasVal = true;
    } else {
      if (tokens[i].offset() == offset + length) {
        length += tokens[i].length();
      } else {
        texts.append(new Text(offset, length));
        offset = tokens[i].offset();
        length = tokens[i].length();
      }
    }
  }
  texts.append(new Text(offset, length));
  return texts;
}
QMap<Char, TokenType> ch2type = {{'#', TokenType::sharp},
                                 {'>', TokenType::gt},
                                 {'!', TokenType::exclamation},
                                 {'*', TokenType::star},
                                 {'~', TokenType::tilde},
                                 {'[', TokenType::left_bracket},
                                 {']', TokenType::right_bracket},
                                 {'(', TokenType::left_parenthesis},
                                 {')', TokenType::right_parenthesis},
                                 {'`', TokenType::backquote},
                                 {'$', TokenType::dollar}};
TokenList parseLine(Line text) {
  TokenList tokens;
  int prev = 0;
  int cur;
  for (cur = 0; cur < text.length; ++cur) {
    auto ch = text[cur];
    if (ch2type.contains(ch)) {
      if (prev != cur) {
        tokens.emplace_back(text.offset + prev, cur - prev, TokenType::text);
      }
      prev = cur + 1;
      tokens.emplace_back(text.offset + cur, 1, ch2type[ch]);
    }
  }
  if (prev != cur) {
    tokens.emplace_back(text.offset + prev, cur - prev, TokenType::text);
  }
  return tokens;
}

struct ParseResult {
  bool success;
  int offset;
  Node* node;
  static ParseResult fail() { return {false}; }
};
class LineParser {
 public:
  [[nodiscard]] virtual ParseResult parse(const TokenList& tokens, int startIndex) const = 0;
};
class BlockParser {
 public:
  [[nodiscard]] virtual ParseResult parse(const LineList& lines, int startIndex) const = 0;

 protected:
  void _parseLine(Container* ret, const std::vector<LineParser*>& parsers, const Line& line) const {
    auto tokens = parseLine(line);
    int i = 0;
    int prev = 0;
    while (i < tokens.size()) {
      bool parsed = false;
      for (auto& it : parsers) {
        auto parseRet = it->parse(tokens, i);
        if (parseRet.success) {
          auto texts = mergeToText(tokens, prev, i);
          ret->appendChildren(texts);
          ret->appendChild(parseRet.node);
          parsed = true;
          prev = i + parseRet.offset;
          i = prev;
          break;
        }
      }
      if (!parsed) i++;
    }
    auto texts = mergeToText(tokens, prev, i);
    ret->appendChildren(texts);
  }
};
// 图片解析器
class ImageParser : public LineParser {
 public:
  ParseResult parse(const TokenList& tokens, int startIndex) const override {
    auto ok = tryParse(tokens, startIndex);
    if (!ok) {
      return ParseResult::fail();
    }
    return _parse(tokens, startIndex);
  }

 private:
  bool tryParse(const TokenList& tokens, int startIndex) const {
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

  ParseResult _parse(const TokenList& tokens, int startIndex) const {
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
    return {true, i - startIndex, new Image((Text*)alt[0], (Text*)url[0])};
  }
};
// 链接解析器
class LinkParser : public LineParser {
 public:
  ParseResult parse(const TokenList& tokens, int startIndex) const override {
    if (!tryParse(tokens, startIndex)) {
      return ParseResult::fail();
    }
    return _parse(tokens, startIndex);
  }

 private:
  bool tryParse(const TokenList& tokens, int startIndex) const {
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

  ParseResult _parse(const TokenList& tokens, int startIndex) const {
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
    return {true, i - startIndex, new Link(content[0], href[0])};
  }
};
// 行内代码解析器
class InlineCodeParser : public LineParser {
 public:
  [[nodiscard]] ParseResult parse(const TokenList& tokens, int startIndex) const override {
    if (startIndex >= tokens.size()) return ParseResult::fail();
    if (tryParse(tokens, startIndex)) {
      return parseInlineCode(tokens, startIndex);
    }
    return ParseResult::fail();
  }

 private:
  [[nodiscard]] bool tryParse(const TokenList& tokens, int startIndex) const {
    if (!isBackquote(tokens[startIndex])) return false;
    int i = startIndex + 1;
    while (i < tokens.size()) {
      if (isBackquote(tokens[i])) {
        return true;
      }
      i++;
    }
    return false;
  }
  [[nodiscard]] ParseResult parseInlineCode(const TokenList& tokens, int startIndex) const {
    int i = startIndex;
    i++;  // `
    int prev = i;
    while (!isBackquote(tokens[i])) i++;
    auto code = mergeToText(tokens, prev, i);
    i++;  // `
    return {true, i - startIndex, new InlineCode(code[0])};
  }
};
// 行内公式解析器
class InlineLatexParser : public LineParser {
 public:
  [[nodiscard]] ParseResult parse(const TokenList& tokens, int startIndex) const override {
    if (!tryParse(tokens, startIndex)) {
      return ParseResult::fail();
    }
    return _parse(tokens, startIndex);
  }

 private:
  [[nodiscard]] bool tryParse(const TokenList& tokens, int startIndex) const {
    if (startIndex + 2 >= tokens.size()) return false;
    if (!isDollar(tokens[startIndex])) return false;
    // 规定行内公式的格式是 $ text $
    if (!isText(tokens[startIndex + 1])) return false;
    if (!isDollar(tokens[startIndex + 2])) return false;
    return true;
  }

  [[nodiscard]] ParseResult _parse(const TokenList& tokens, int startIndex) const {
    auto& token = tokens[startIndex + 1];
    return {true, 3, new InlineLatex(new Text(token.offset(), token.length()))};
  }
};
// 加粗和斜体解析器
class SemanticTextParser : public LineParser {
 public:
  [[nodiscard]] ParseResult parse(const TokenList& tokens, int startIndex) const override {
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

 private:
  [[nodiscard]] static bool tryParseItalic(const TokenList& tokens, int startIndex) {
    if (startIndex + 3 > tokens.size()) return false;
    auto offset_list = {0, 2};
    auto text_offset = 1;
    for (auto offset : offset_list) {
      if (!isStar(tokens[startIndex + offset])) return false;
    }
    return isText(tokens[startIndex + text_offset]);
  }
  [[nodiscard]] static bool tryParseBold(const TokenList& tokens, int startIndex) {
    if (startIndex + 5 > tokens.size()) return false;
    auto offset_list = {0, 1, 3, 4};
    auto text_offset = 2;
    for (auto offset : offset_list) {
      if (!isStar(tokens[startIndex + offset])) return false;
    }
    return isText(tokens[startIndex + text_offset]);
  }

  [[nodiscard]] static bool tryParseItalicAndBold(const TokenList& tokens, int startIndex) {
    if (startIndex + 7 > tokens.size()) return false;
    auto offset_list = {0, 1, 2, 4, 5, 6};
    auto text_offset = 3;
    for (auto offset : offset_list) {
      if (!isStar(tokens[startIndex + offset])) return false;
    }
    return isText(tokens[startIndex + text_offset]);
  }

  [[nodiscard]] static bool tryParseStrickout(const TokenList& tokens, int startIndex) {
    if (startIndex + 5 > tokens.size()) return false;
    auto offset_list = {0, 1, 3, 4};
    auto text_offset = 2;
    for (auto offset : offset_list) {
      if (!isTilde(tokens[startIndex + offset])) return false;
    }
    return isText(tokens[startIndex + text_offset]);
  }

  static ParseResult _parseItalic(const TokenList& tokens, int startIndex) {
    auto& token = tokens[startIndex + 1];
    auto text = new Text(token.offset(), token.length());
    auto node = new ItalicText(text);
    return {true, 3, node};
  }
  static ParseResult _parseBold(const TokenList& tokens, int startIndex) {
    auto& token = tokens[startIndex + 2];
    auto text = new Text(token.offset(), token.length());
    auto node = new BoldText(text);
    return {true, 5, node};
  }
  static ParseResult _parseItalicAndBold(const TokenList& tokens, int startIndex) {
    auto& token = tokens[startIndex + 3];
    auto text = new Text(token.offset(), token.length());
    auto node = new ItalicBoldText(text);
    return {true, 7, node};
  }
  static ParseResult _parseStrickout(const TokenList& tokens, int startIndex) {
    auto& token = tokens[startIndex + 2];
    auto text = new Text(token.offset(), token.length());
    auto node = new StrickoutText(text);
    return {true, 5, node};
  }
};
// 标题解析器
class HeaderParser : public BlockParser {
 public:
  ParseResult parse(const LineList& lines, int startIndex) const override {
    if (startIndex >= lines.size()) return ParseResult::fail();
    auto line = trimLeft(lines[startIndex]);
    if (tryParseHeader(line)) {
      auto header = parseHeader(line);
      return {true, 1, header};
    } else {
      return ParseResult::fail();
    }
  }

 private:
  [[nodiscard]] static bool tryParseHeader(const Line& line) {
    int i = 0;
    while (i < line.size() && line[i] == '#') i++;
    // 如果没有#或#的个数超过6个
    if (i == 0 || i > 6) return false;
    // #后面要接一个空格
    if (i < line.size() && line[i] == ' ') return true;
    return false;
  }
  [[nodiscard]] static Node* parseHeader(const Line& line) {
    static std::vector<LineParser*> parsers{
        new LinkParser(),
        new InlineCodeParser(),
        new InlineLatexParser(),
    };
    int i = 0;
    while (i < line.size() && line[i] == '#') i++;
    int level = i;
    // 跳过空格
    i++;
    Line str = line.mid(i);
    auto tokens = parseLine(str);
    auto header = new Header(level);
    i = 0;
    int prev = 0;
    while (i < tokens.size()) {
      auto token = tokens[i];
      bool parsed = false;
      for (auto& it : parsers) {
        auto parseRet = it->parse(tokens, i);
        if (parseRet.success) {
          auto texts = mergeToText(tokens, prev, i);
          header->appendChildren(texts);
          header->appendChild(parseRet.node);
          parsed = true;
          prev = i + parseRet.offset;
          i = prev;
          break;
        }
      }
      if (!parsed) i++;
    }
    auto texts = mergeToText(tokens, prev, i);
    header->appendChildren(texts);
    return header;
  }
};
// 表格解析器
class TableParser : public BlockParser {
 public:
  ParseResult parse(const LineList& lines, int startIndex) const override {
    //    if (startIndex < lines.size() && lines[startIndex].startsWith("|")) {
    //      return parseTable(lines, startIndex);
    //    } else {
    //      return ParseResult::fail();
    //    }
    return ParseResult::fail();
  }

 private:
#if 0
  ParseResult parseTable(const LineList& lines, int startIndex) const {
    int i = startIndex;
    char sep = '|';
    // 统计分隔符的个数
    auto countColumn = [sep](const Line& row) {
      int colNum = -1;
      for (int i = 0; i < row.length; ++i) {
        auto ch = row[i];
        if (ch == sep) colNum++;
      }
      return colNum;
    };
    // 分割列
    auto cutColumn = [sep](const Line& row) {
      int lastSepIndex = 0;
      LineList rowContent;
      for (int index = 1; index < row.size(); index++) {
        if (row[index] == sep) {
          rowContent.emplace_back(row.text, row.offset + lastSepIndex + 1, index - (lastSepIndex + 1));
          lastSepIndex = index;
        }
      }
      return rowContent;
    };
    Line headerRow = lines[i].trimmed();
    // 如果最后一个字符不是分割线
    if (headerRow.back() != sep) return ParseResult::fail();
    int colNum = countColumn(lines[i]);
    auto header = cutColumn(headerRow);
    i++;
    if (i >= lines.size()) return ParseResult::fail();
    int secondColNum = countColumn(lines[i]);
    if (secondColNum != colNum) return ParseResult::fail();
    i++;
    Table tab;
    tab.setHeader(header);
    while (i < lines.size() && !lines[i].isEmpty() && lines[i].front() == sep) {
      auto curRow = lines[i].trimmed();
      int curColNum = countColumn(curRow);
      if (curColNum != colNum) return ParseResult::fail();
      auto rowContent = cutColumn(curRow);
      tab.appendRow(rowContent);
      i++;
    }
    return {true, i - startIndex, new Table(tab)};
  }

#endif
};

// 行间公式解析器
class LatexBlockParser : public BlockParser {
 public:
  [[nodiscard]] ParseResult parse(const LineList& lines, int startIndex) const override {
    if (startIndex < lines.size() && lines[startIndex].startsWith("$$")) {
      return parseLatexBlock(lines, startIndex);
    } else {
      return ParseResult::fail();
    }
    return ParseResult::fail();
  }

 private:
  [[nodiscard]] ParseResult parseLatexBlock(const LineList& lines, int startIndex) const {
    int i = startIndex + 1;
    while (i < lines.size()) {
      if (lines[i].startsWith("$$")) {
        break;
      }
      i++;
    }
    if (i == startIndex + 1 || i == lines.size()) return ParseResult::fail();
    auto latexBlock = new LatexBlock();
    for (int j = startIndex + 1; j < i; j++) {
      latexBlock->appendChild(new Text(lines[j].offset, lines[j].length));
      // 添加一个换行符\n，还原文本时可以用到
      latexBlock->appendChild(new Lf());
    }
    i++;  // last $$
    return {true, i - startIndex, latexBlock};
  }
};

// 段落解析器(默认解析器)
class ParagraphParser : public BlockParser {
 public:
  [[nodiscard]] ParseResult parse(const LineList& lines, int startIndex) const override {
    return parseParagraph(lines, startIndex);
  }

 private:
  [[nodiscard]] ParseResult parseParagraph(const LineList& lines, int lineIndex) const {
    auto paragraph = new Paragraph();
    static std::vector<LineParser*> parsers{
        new ImageParser(), new LinkParser(), new InlineCodeParser(), new InlineLatexParser(), new SemanticTextParser(),
    };
    auto i = lineIndex;
    StringList prefix_list = {"#", "- ", "1. ", "```", "$$"};
    bool firstInParagraph = true;
    while (i < lines.size()) {
      auto& line = lines[i];
      if (line.length == 0) {
        // 如果空行，说明段落结束
        i++;
        break;
      }
      // 如果发现其他块元素的特征，段落也结束
      bool hasOtherFeat = false;
      for (const auto& s : prefix_list) {
        if (line.startsWith(s)) {
          hasOtherFeat = true;
          break;
        }
      }
      if (hasOtherFeat) break;
      // 不然的话，就加一个换行，但还是同一个段落
      if (!firstInParagraph) {
        paragraph->appendChild(new Lf());
      }
      _parseLine(paragraph, parsers, line);
      i++;
      firstInParagraph = false;
    }
    return {true, i - lineIndex, paragraph};
  }
};

// 行间代码解析器
class CodeBlockParser : public BlockParser {
 public:
  ParseResult parse(const LineList& lines, int startIndex) const override {
    if (!tryParseCodeBlock(lines, startIndex)) {
      return ParseResult::fail();
    }
    return parseCodeBlock(lines, startIndex);
  }

 private:
  [[nodiscard]] ParseResult parseCodeBlock(const LineList& lines, int startIndex) const {
    if (startIndex >= lines.size()) return ParseResult::fail();
    int i = startIndex;
    auto line = lines[i];
    i++;
    auto name = line.mid(3);
    auto codeBlock = new CodeBlock(new Text(name.offset, name.length));
    while (i < lines.size() && !lines[i].startsWith("```")) {
      codeBlock->appendChild(new Text(lines[i].offset, lines[i].length));
      codeBlock->appendChild(new Lf());
      i++;
    }
    i++;
    return {true, i - startIndex, codeBlock};
  }
  [[nodiscard]] bool tryParseCodeBlock(const LineList& lines, int i) const {
    if (i >= lines.size() || !lines[i].startsWith("```")) return false;
    i++;
    while (i < lines.size() && !lines[i].startsWith("```")) i++;
    if (i >= lines.size() || !lines[i].startsWith("```")) return false;
    return true;
  }
};

// Checkbox解析器
class CheckboxListParser : public BlockParser {
 public:
  [[nodiscard]] ParseResult parse(const LineList& lines, int startIndex) const override {
    if (startIndex >= lines.size()) return ParseResult::fail();
    auto line = lines[startIndex];
    if (line.startsWith("- [ ] ") || line.startsWith("- [x] ")) {
      return parseCheckboxList(lines, startIndex);
    } else {
      return ParseResult::fail();
    }
  }

 private:
  [[nodiscard]] ParseResult parseCheckboxList(const LineList& lines, int startIndex) const {
    static std::vector<LineParser*> parsers = {
        new LinkParser(),
        new InlineLatexParser(),
        new InlineCodeParser(),
        new SemanticTextParser(),
    };
    int i = startIndex;
    auto checkboxList = new CheckboxList();
    QString uncheckedPrefix = "- [ ] ";
    QString checkedPrefix = "- [x] ";
    while (i < lines.size()) {
      auto line = lines[i];
      if (line.startsWith(uncheckedPrefix)) {
        auto item = new CheckboxItem();
        item->setChecked(false);
        auto line2 = line.right(line.size() - uncheckedPrefix.size());
        _parseLine(item, parsers, line2);
        checkboxList->appendChild(item);
        i++;
      } else if (line.startsWith(checkedPrefix)) {
        auto item = new CheckboxItem();
        item->setChecked(true);
        auto line2 = line.right(line.size() - checkedPrefix.size());
        _parseLine(item, parsers, line2);
        checkboxList->appendChild(item);
        i++;
      } else {
        break;
      }
    }
    return {true, i - startIndex, checkboxList};
  }
};

// 无序列表解析器
class UnorderedListParser : public BlockParser {
 public:
  [[nodiscard]] ParseResult parse(const LineList& lines, int startIndex) const override {
    if (startIndex >= lines.size()) return ParseResult::fail();
    auto line = trimLeft(lines[startIndex]);
    if (line.startsWith("- ")) {
      return parseUnorderedList(lines, startIndex);
    } else {
      return ParseResult::fail();
    }
  }

 private:
  [[nodiscard]] ParseResult parseUnorderedList(const LineList& lines, int startIndex) const {
    static std::vector<LineParser*> parsers = {
        new LinkParser(),
        new InlineLatexParser(),
        new InlineCodeParser(),
        new SemanticTextParser(),
    };
    int i = startIndex;
    auto ul = new UnorderedList();
    while (i < lines.size() && trimLeft(lines[i]).startsWith("- ")) {
      auto item = new UnorderedListItem();
      _parseLine(item, parsers, trimLeft(lines[i]).mid(2));
      ul->appendChild(item);
      i++;
    }
    return {true, i - startIndex, ul};
  }
};

// 有序列表解析器
class OrderedListParser : public BlockParser {
 public:
  [[nodiscard]] ParseResult parse(const LineList& lines, int startIndex) const override {
    if (startIndex >= lines.size()) return ParseResult::fail();
    auto line = lines[startIndex];
    if (line.startsWith("1. ")) {
      return parseOrderedList(lines, startIndex);
    } else {
      return ParseResult::fail();
    }
  }

 private:
  [[nodiscard]] ParseResult parseOrderedList(const LineList& lines, int startIndex) const {
    static std::vector<LineParser*> parsers = {
        new LinkParser(),
        new InlineLatexParser(),
        new InlineCodeParser(),
        new SemanticTextParser(),
    };
    int i = startIndex;
    auto ol = new OrderedList();
    while (i < lines.size()) {
      auto line = lines[i];
      bool hasDigit = false;
      int j = 0;
      while (j < line.size() && line[j].isDigit()) {
        hasDigit = true;
        j++;
      }
      if (!hasDigit) return {true, i - startIndex, ol};
      if (j >= line.size() || line[j] != '.') return {true, i - startIndex, ol};
      j++;
      if (j >= line.size() || line[j] != ' ') return {true, i - startIndex, ol};
      j++;
      auto item = new OrderedListItem();
      _parseLine(item, parsers, line.mid(j));
      ol->appendChild(item);
      // 下一行
      i++;
    }
    return {true, i - startIndex, ol};
  }
};

// 引用解析器
class QuoteBlockParser : public BlockParser {
 public:
  [[nodiscard]] ParseResult parse(const LineList& lines, int startIndex) const override {
    if (startIndex >= lines.size()) return ParseResult::fail();
    auto line = lines[startIndex];
    if (line.startsWith("> ")) {
      return parseQuoteBlock(lines, startIndex);
    } else {
      return ParseResult::fail();
    }
  }

 private:
  [[nodiscard]] ParseResult parseQuoteBlock(const LineList& lines, int startIndex) const {
    int i = startIndex;
    auto quoteBlock = new QuoteBlock();
    while (i < lines.size() && lines[i].startsWith("> ")) {
      auto& line = lines[i];
      quoteBlock->appendChild(new Text(line.offset + 2, line.length - 2));
      i++;
    }
    i++;
    return {true, i - startIndex, quoteBlock};
  }
};

class ParserPrivate {
 public:
  explicit ParserPrivate(const String& text) : m_text(text) {}
  void splitTextToLines() {
    qsizetype i = 0;
    qsizetype offset = 0;
    qsizetype length = 0;
    while (i < m_text.size()) {
      if (m_text.at(i) == '\r') {
        m_lines.emplace_back(m_text, offset, length);
        if (i + 1 < m_text.size() && m_text.at(i + 1) == '\n') {
          offset += length + 2;
          i++;
          i++;
        } else {
          offset += length + 1;
          i++;
        }
        length = 0;
      } else if (m_text.at(i) == '\n') {
        i++;
        m_lines.emplace_back(m_text, offset, length);
        offset += length + 1;
        length = 0;
      } else {
        i++;
        length++;
      }
    }
    // 最后还剩的一点
    if (length != 0) {
      m_lines.emplace_back(m_text, offset, length);
    }
  }
  sptr<Container> parse() {
    static std::vector<BlockParser*> parsers = {
        new HeaderParser(),        new CodeBlockParser(),   new CheckboxListParser(),
        new UnorderedListParser(), new OrderedListParser(), new QuoteBlockParser(),
        new TableParser(),         new LatexBlockParser(),  new ParagraphParser()};
    auto nodes = std::make_shared<Container>();
    splitTextToLines();
    int i = 0;
    while (i < m_lines.size()) {
      for (auto parser : parsers) {
        auto parseRet = parser->parse(m_lines, i);
        if (parseRet.success) {
          i += parseRet.offset;
          nodes->appendChild(parseRet.node);
          break;
        }
      }
    }
    return nodes;
  }

 private:
  const String& m_text;
  LineList m_lines;
};
sptr<Container> Parser::parse(const String& text) {
  ParserPrivate parser(text);
  return parser.parse();
}
}  // namespace md::parser
