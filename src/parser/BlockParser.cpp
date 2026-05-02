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

}  // namespace md::parser
