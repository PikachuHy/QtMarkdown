//
// Created by pikachu on 2021/1/31.
//
// Parser for fenced code blocks (```)

#include "ParserDetail.h"

#include "Document.h"
#include "Text.h"
#include "debug.h"
#include "mddef.h"

namespace md::parser {

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

}  // namespace md::parser
