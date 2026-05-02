//
// Created by pikachu on 2021/1/31.
//

#include "Parser.h"

#include "ParserDetail.h"
#include "nodes/Paragraph.h"
#include "Text.h"
#include "debug.h"

using namespace std::string_view_literals;

namespace md::parser {

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
