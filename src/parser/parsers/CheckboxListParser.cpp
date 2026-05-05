//
// Created by pikachu on 2021/1/31.
//
// Parser for checkbox lists (- [ ] and - [x])

#include "ParserDetail.h"

#include "Document.h"
#include "Text.h"
#include "debug.h"
#include "mddef.h"

namespace md::parser {

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
    String uncheckedPrefix = "- [ ] ";
    String checkedPrefix = "- [x] ";
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

}  // namespace md::parser
