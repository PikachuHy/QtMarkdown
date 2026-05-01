//
// Created by pikachu on 2021/1/31.
//

#include "Parser.h"

#include <QRegularExpression>
#include <vector>
#include <memory>
#include <functional>

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
std::vector<std::unique_ptr<Text>> mergeToText(const TokenList& tokens, int prev, int cur) {
  if (prev >= cur) return {};
  std::vector<std::unique_ptr<Text>> texts;
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
        texts.push_back(std::make_unique<Text>(offset, length));
        offset = tokens[i].offset();
        length = tokens[i].length();
      }
    }
  }
  texts.push_back(std::make_unique<Text>(offset, length));
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
  std::unique_ptr<Node> node;
  static ParseResult fail() { return {false}; }
};

using LineParserFn = std::function<ParseResult(const TokenList&, int startIndex)>;
using BlockParserFn = std::function<ParseResult(const LineList&, int startIndex)>;

void _parseLine(Container* ret, const std::vector<LineParserFn>& parsers, const Line& line) {
    auto tokens = parseLine(line);
    int i = 0;
    int prev = 0;
    while (i < tokens.size()) {
      bool parsed = false;
      for (auto& it : parsers) {
        auto parseRet = it(tokens, i);
        if (parseRet.success) {
          auto texts = mergeToText(tokens, prev, i);
          ret->appendChildren(std::move(texts));
          ret->appendChild(std::move(parseRet.node));
          parsed = true;
          prev = i + parseRet.offset;
          i = prev;
          break;
        }
      }
      if (!parsed) i++;
    }
    auto texts = mergeToText(tokens, prev, i);
    ret->appendChildren(std::move(texts));
}

void skipEmptyLine(const LineList& lines, int& i) {
    // 如果是空行
    while (i < lines.size() && lines[i].length == 0) {
      i++;
    }
}

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

// 标题解析器
[[nodiscard]] bool tryParseHeader(const Line& line) {
    int i = 0;
    while (i < line.size() && line[i] == '#') i++;
    // 如果没有#或#的个数超过6个
    if (i == 0 || i > 6) return false;
    // #后面要接一个空格
    if (i < line.size() && line[i] == ' ') return true;
    return false;
}

[[nodiscard]] std::unique_ptr<Node> parseHeaderContent(const Line& line) {
    static std::vector<LineParserFn> parsers = {
        parseLink,
        parseInlineCode,
        parseInlineLatex,
    };
    int i = 0;
    while (i < line.size() && line[i] == '#') i++;
    int level = i;
    // 跳过空格
    i++;
    Line str = line.mid(i);
    auto header = std::make_unique<Header>(level);
    _parseLine(header.get(), parsers, str);
    return header;
}

[[nodiscard]] ParseResult parseHeader(const LineList& lines, int startIndex) {
    if (startIndex >= lines.size()) return ParseResult::fail();
    auto line = trimLeft(lines[startIndex]);
    if (!tryParseHeader(line)) return ParseResult::fail();
    auto header = parseHeaderContent(line);
    int i = startIndex + 1;
    skipEmptyLine(lines, i);
    return {true, i - startIndex, std::move(header)};
}

// 行间公式解析器
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
      // 添加一个换行符\n，还原文本时可以用到
      latexBlock->appendChild(std::make_unique<Lf>());
    }
    i++;  // last $$
    return {true, i - startIndex, std::move(latexBlock)};
}

// 段落解析器(默认解析器)
[[nodiscard]] ParseResult parseParagraph(const LineList& lines, int lineIndex) {
    auto paragraph = std::make_unique<Paragraph>();
    static std::vector<LineParserFn> parsers = {
        parseImage,
        parseLink,
        parseInlineCode,
        parseInlineLatex,
        parseSemanticText,
    };
    auto i = lineIndex;
    StringList prefix_list = {"# ", "## ", "### ", "#### ", "##### ", "###### ", "- ", "1. ", "```", "$$"};
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
      if (hasOtherFeat) {
        if (line.startsWith("```") && i + 1 == lines.size()) {
          // 只一个光秃秃的```，是构不成CodeBlock的，只能当作是段落
        } else {
          break;
        }
      }
      // 不然的话，就加一个换行，但还是同一个段落
      if (!firstInParagraph) {
        paragraph->appendChild(std::make_unique<Lf>());
      }
      _parseLine(paragraph.get(), parsers, line);
      i++;
      firstInParagraph = false;
    }
    skipEmptyLine(lines, i);
    return {true, i - lineIndex, std::move(paragraph)};
}

// 行间代码解析器
[[nodiscard]] bool tryParseCodeBlock(const LineList& lines, int startIndex) {
    if (startIndex >= lines.size() || !lines[startIndex].startsWith("```")) return false;
    int i = startIndex + 1;
    while (i < lines.size() && !lines[i].startsWith("```")) i++;
    if (i >= lines.size() || !lines[i].startsWith("```")) return false;
    return true;
}

[[nodiscard]] ParseResult parseCodeBlock(const LineList& lines, int startIndex) {
    if (!tryParseCodeBlock(lines, startIndex)) return ParseResult::fail();
    int i = startIndex;
    auto line = lines[i];
    i++;
    auto name = line.mid(3);
    auto codeBlock = std::make_unique<CodeBlock>(std::make_unique<Text>(name.offset, name.length));
    while (i < lines.size() && !lines[i].startsWith("```")) {
      codeBlock->appendChild(std::make_unique<Text>(lines[i].offset, lines[i].length));
      //      codeBlock->appendChild(new Lf());
      i++;
    }
    i++;
    skipEmptyLine(lines, i);
    return {true, i - startIndex, std::move(codeBlock)};
}

// Checkbox解析器
[[nodiscard]] ParseResult parseCheckboxList(const LineList& lines, int startIndex) {
    if (startIndex >= lines.size()) return ParseResult::fail();
    auto line = lines[startIndex];
    if (!line.startsWith("- [ ] ") && !line.startsWith("- [x] ")) return ParseResult::fail();
    static std::vector<LineParserFn> parsers = {
        parseLink,
        parseInlineLatex,
        parseInlineCode,
        parseSemanticText,
    };
    int i = startIndex;
    auto checkboxList = std::make_unique<CheckboxList>();
    QString uncheckedPrefix = "- [ ] ";
    QString checkedPrefix = "- [x] ";
    while (i < lines.size()) {
      auto line = lines[i];
      if (line.startsWith(uncheckedPrefix)) {
        auto item = std::make_unique<CheckboxItem>();
        item->setChecked(false);
        auto line2 = line.right(line.size() - uncheckedPrefix.size());
        _parseLine(item.get(), parsers, line2);
        checkboxList->appendChild(std::move(item));
        i++;
      } else if (line.startsWith(checkedPrefix)) {
        auto item = std::make_unique<CheckboxItem>();
        item->setChecked(true);
        auto line2 = line.right(line.size() - checkedPrefix.size());
        _parseLine(item.get(), parsers, line2);
        checkboxList->appendChild(std::move(item));
        i++;
      } else {
        break;
      }
    }
    skipEmptyLine(lines, i);
    return {true, i - startIndex, std::move(checkboxList)};
}

// 无序列表解析器
[[nodiscard]] ParseResult parseUnorderedList(const LineList& lines, int startIndex) {
    if (startIndex >= lines.size()) return ParseResult::fail();
    auto line = trimLeft(lines[startIndex]);
    if (!line.startsWith("- ")) return ParseResult::fail();
    static std::vector<LineParserFn> parsers = {
        parseLink,
        parseInlineLatex,
        parseInlineCode,
        parseSemanticText,
    };
    int i = startIndex;
    auto ul = std::make_unique<UnorderedList>();
    while (i < lines.size() && trimLeft(lines[i]).startsWith("- ")) {
      auto item = std::make_unique<UnorderedListItem>();
      _parseLine(item.get(), parsers, trimLeft(lines[i]).mid(2));
      ul->appendChild(std::move(item));
      i++;
    }
    skipEmptyLine(lines, i);
    return {true, i - startIndex, std::move(ul)};
}

// 有序列表解析器
[[nodiscard]] ParseResult parseOrderedList(const LineList& lines, int startIndex) {
    if (startIndex >= lines.size()) return ParseResult::fail();
    auto line = lines[startIndex];
    if (!line.startsWith("1. ")) return ParseResult::fail();
    static std::vector<LineParserFn> parsers = {
        parseLink,
        parseInlineLatex,
        parseInlineCode,
        parseSemanticText,
    };
    int i = startIndex;
    auto ol = std::make_unique<OrderedList>();
    while (i < lines.size()) {
      auto line = lines[i];
      bool hasDigit = false;
      int j = 0;
      while (j < line.size() && line[j].isDigit()) {
        hasDigit = true;
        j++;
      }
      if (!hasDigit) break;
      if (j >= line.size() || line[j] != '.') return {true, i - startIndex, std::move(ol)};
      j++;
      if (j >= line.size() || line[j] != ' ') return {true, i - startIndex, std::move(ol)};
      j++;
      auto item = std::make_unique<OrderedListItem>();
      _parseLine(item.get(), parsers, line.mid(j));
      ol->appendChild(std::move(item));
      // 下一行
      i++;
    }
    skipEmptyLine(lines, i);
    return {true, i - startIndex, std::move(ol)};
}

// 引用解析器
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
    static std::vector<BlockParserFn> parsers = {
        parseHeader,
        parseCodeBlock,
        parseCheckboxList,
        parseUnorderedList,
        parseOrderedList,
        parseQuoteBlock,
        // TableParser removed (dead code — always returned fail)
        parseLatexBlock,
        parseParagraph,
    };
    auto nodes = std::make_shared<Container>();
    splitTextToLines();
    int i = 0;
    while (i < m_lines.size()) {
      for (auto& parser : parsers) {
        auto parseRet = parser(m_lines, i);
        if (parseRet.success) {
          i += parseRet.offset;
          if (parseRet.node->type() == NodeType::paragraph) {
            // 空段落直接去掉
            auto paragraphNode = static_cast<Paragraph*>(parseRet.node.get());
#if 1
            if (paragraphNode->children().empty()) {
              DEBUG << "delete empty paragraph node";
              parseRet.node.reset();
              break;
            } else {
              nodes->appendChild(std::move(parseRet.node));
            }
#endif
          } else {
            nodes->appendChild(std::move(parseRet.node));
          }
          break;
        }
      }
    }
    // 如果文档为空，默认添加一个段落
    if (nodes->children().empty()) {
      nodes->appendChild(std::make_unique<Paragraph>());
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
