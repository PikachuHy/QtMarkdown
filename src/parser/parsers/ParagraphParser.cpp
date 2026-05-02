//
// Created by pikachu on 2021/1/31.
//
// Parser for paragraphs (default/fallback block parser)

#include "ParserDetail.h"

#include <QRegularExpression>

#include "Document.h"
#include "Text.h"
#include "debug.h"
#include "mddef.h"

namespace md::parser {

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
        i++;
        break;
      }
      bool hasOtherFeat = false;
      for (const auto& s : prefix_list) {
        if (line.startsWith(s)) {
          hasOtherFeat = true;
          break;
        }
      }
      if (hasOtherFeat) {
        if (line.startsWith("```") && i + 1 == lines.size()) {
        } else {
          break;
        }
      }
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

}  // namespace md::parser
