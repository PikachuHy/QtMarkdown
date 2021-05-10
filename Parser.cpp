//
// Created by pikachu on 2021/1/31.
//

#include "Parser.h"
#include <regex>
#include "Document.h"
#include <QRegularExpression>
#include <QDebug>
#include <utility>
#define emplace_back push_back
using namespace std::string_view_literals;

TokenList parseLine(String text);
Text *mergeToText(const TokenList& tokens, int prev, int cur) {
    if (prev >= cur) return nullptr;
    String str;
    for(int i=prev;i<cur;i++) {
        str += tokens[i].str();
    }
    return new Text(str);
}

TokenList parseLine(String text) {
    TokenList ret;
    int prev = 0;
    int cur = 0;
    for (auto ch: text) {
        if (spMap.contains(ch)) {
            if (prev != cur) {
                ret.emplace_back(Token(text.mid(prev, cur - prev)));
            }
            prev = cur + 1;
            ret.push_back(spMap[ch]);
        }
        cur++;
    }
    if (prev != cur) {
        ret.emplace_back(Token(text.mid(prev, cur - prev)));
    }
    for(auto it: ret) {
        // qDebug() << it.str();
    }
    return ret;
}

struct ParseResult {
    bool success;
    int offset;
    Node* node;
    static ParseResult fail() {
        return {
                false
        };
    }
};
class LineParser {
public:
    virtual ParseResult parse(const TokenList& tokens, int startIndex) = 0;
};
class BlockParser {
public:
    virtual ParseResult parse(const StringList& lines, int startIndex) = 0;
};
// 图片解析器
class ImageParser: public LineParser{
public:
    ParseResult parse(const TokenList &tokens, int startIndex) {
        auto ok = tryParse(tokens, startIndex);
        if (!ok) {
            return ParseResult::fail();
        }
        return _parse(tokens, startIndex);
    }

private:
    bool tryParse(const TokenList &tokens, int startIndex) {
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

    ParseResult _parse(const TokenList &tokens, int startIndex) {
        int i = startIndex;
        i++; // !
        i++; // [
        int prev = i;
        while (!isRightBracket(tokens[i])) i++;
        auto alt = mergeToText(tokens, prev, i);
        i++; // ]
        i++; // (
        prev = i;
        while (!isRightParenthesis(tokens[i])) i++;
        auto url = mergeToText(tokens, prev, i);
        i++; // )
        return {
                true,
                i - startIndex,
                new Image(alt, url)
        };
    }
};
// 链接解析器
class LinkParser: public LineParser{
public:
    ParseResult parse(const TokenList& tokens, int startIndex) {
        if (!tryParse(tokens, startIndex)) {
            return ParseResult::fail();
        }
        return _parse(tokens, startIndex);
    }
private:
    bool tryParse(const TokenList &tokens, int startIndex) {
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

    ParseResult _parse(const TokenList &tokens, int startIndex)  {
        int i = startIndex;
        i++; // [
        int prev = i;
        while (!isRightBracket(tokens[i])) i++;
        auto content = mergeToText(tokens, prev, i);
        i++; // ]
        i++; // (
        prev = i;
        while (!isRightParenthesis(tokens[i])) i++;
        auto href = mergeToText(tokens, prev, i);
        i++; // )
        return {
                true,
                i - startIndex,
                new Link(content, href)
        };
    }
};
// 行内代码解析器
class InlineCodeParser: public LineParser{
public:
    ParseResult parse(const TokenList &tokens, int startIndex) {
        if (startIndex >= tokens.size()) return ParseResult::fail();
        if (tryParse(tokens, startIndex)) {
            return parseInlineCode(tokens, startIndex);
        }
        return ParseResult::fail();
    }
private:
    bool tryParse(const TokenList &tokens, int startIndex)  {
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
    ParseResult parseInlineCode(const TokenList& tokens, int startIndex) {
        int i = startIndex;
        i++; // `
        int prev = i;
        while (!isBackquote(tokens[i])) i++;
        auto code = mergeToText(tokens, prev, i);
        i++; // `
        return {
            true,
            i - startIndex,
            new InlineCode(code)
        };
    }

};
// 行内公式解析器
class InlineLatexParser: public LineParser{
public:
    ParseResult parse(const TokenList &tokens, int startIndex) {
        if (!tryParse(tokens, startIndex)) {
            return ParseResult::fail();
        }
        return _parse(tokens, startIndex);
    }
private:
    bool tryParse(const TokenList &tokens, int startIndex) {
        int i = startIndex + 1;
        while (i < tokens.size()) {
            if (isDollar(tokens[i])) {
                return true;
            }
            i++;
        }
        return false;
    }

    ParseResult _parse(const TokenList &tokens, int startIndex) {
        int i = startIndex;
        i++;
        String latex;
        while (!isDollar(tokens[i])) {
            latex += tokens[i].str();
            i++;
        }
        i++;
        return {
                true,
                i - startIndex,
                new InlineLatex(new Text(latex))
        };
    }
};
// 加粗和斜体解析器
class SemanticTextParser: public LineParser{
public:
    ParseResult parse(const TokenList &tokens, int startIndex)  {
        if (tryParseItalicAndBold(tokens, startIndex)) {
            return _parseItalicAndBold(tokens, startIndex);
        }
        if (tryParseBold(tokens, startIndex)) {
            return _parseBold(tokens, startIndex);
        }
        if (tryParseItalic(tokens, startIndex)) {
            return _parseItalic(tokens, startIndex);
        }
        return ParseResult::fail();
    }
private:
    bool tryParseItalic(const TokenList& tokens, int startIndex) {
        int startCount = 0;
        int i = startIndex;
        while (i < tokens.size() && isStar(tokens[i])) {
            i++;
            startCount++;
        }
        if (startCount == 1) {
            return i + 2 < tokens.size() && isStar(tokens[i+1]);
        }
        return false;
    }
    bool tryParseBold(const TokenList& tokens, int startIndex) {
        int startCount = 0;
        int i = startIndex;
        while (i < tokens.size() && isStar(tokens[i])) {
            i++;
            startCount++;
        }
        if (startCount == 2) {
            return i + 3 < tokens.size()
                   && isStar(tokens[i+1])
                   && isStar(tokens[i+2]);
        }
        return false;
    }

    bool tryParseItalicAndBold(const TokenList& tokens, int startIndex) {
        int startCount = 0;
        int i = startIndex;
        while (i < tokens.size() && isStar(tokens[i])) {
            i++;
            startCount++;
        }
        if (startCount == 3) {
            return i + 4 < tokens.size()
                   && isStar(tokens[i+1])
                   && isStar(tokens[i+2])
                   && isStar(tokens[i+3]);
        }
        return false;
    }


    ParseResult _parseItalic(const TokenList& tokens, int startIndex) {
        int i = startIndex;
        i++;
        auto node = new ItalicText(tokens[i].str());
        i++;
        i++;
        return {
                true,
                i - startIndex,
                node
        };
    }
    ParseResult _parseBold(const TokenList& tokens, int startIndex) {
        int i = startIndex;
        i++;
        i++;
        auto node = new BoldText(tokens[i].str());
        i++;
        i++;
        i++;
        return {
                true,
                i - startIndex,
                node
        };
    }
    ParseResult _parseItalicAndBold(const TokenList& tokens, int startIndex) {
        int i = startIndex;
        i++;
        i++;
        i++;
        auto node = new BoldText(tokens[i].str());
        i++;
        i++;
        i++;
        i++;
        return {
                true,
                i - startIndex,
                node
        };
    }
};
// 标题解析器
class HeaderParser: public BlockParser {
public:
    ParseResult parse(const StringList &lines, int startIndex) override {
        if (startIndex < lines.size() && tryParseHeader(lines[startIndex])) {
            auto header = parseHeader(lines[startIndex]);
            return {
                    true,
                    1,
                    header
            };
        } else {
            return ParseResult::fail();
        }
    }
private:
    bool tryParseHeader(const String& line) {
        int i = 0;
        while (i < line.size() && line[i] == '#') i++;
        // 如果没有#或#的个数超过6个
        if (i == 0 || i > 6) return false;
        // #后面要接一个空格
        if (i < line.size() && line[i] == ' ') return true;
        return false;
    }
    Node *parseHeader(const String& line) {
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
        String str = line.mid(i);
        auto tokens = parseLine(str);
        auto ret = new Header(level);
        i = 0;
        int prev = 0;
        while (i < tokens.size()) {
            auto token = tokens[i];
            bool parsed = false;
            for(auto& it: parsers) {
                auto parseRet = it->parse(tokens, i);
                if (parseRet.success) {
                    auto text = mergeToText(tokens, prev, i);
                    if (text) ret->appendChild(text);
                    ret->appendChild(parseRet.node);
                    parsed = true;
                    prev = i + parseRet.offset;
                    i = prev;
                    break;
                }
            }
            if (!parsed) i++;
        }
        auto text = mergeToText(tokens, prev, i);
        if (text) ret->appendChild(text);
        return ret;
    }

};
// 表格解析器
class TableParser: public BlockParser {
public:
    ParseResult parse(const StringList &lines, int startIndex) override {
        if (startIndex < lines.size() && lines[startIndex].startsWith("|")) {
            return parseTable(lines, startIndex);
        } else {
            return ParseResult::fail();
        }
    }
private:

    ParseResult parseTable(const StringList &lines, int startIndex) {
        int i = startIndex;
        char sep = '|';
        // 统计分隔符的个数
        auto countColumn = [sep](const String& row) {
            int colNum = -1;
            for(auto ch: row) {
                if (ch == sep) colNum++;
            }
            return colNum;
        };
        // 分割列
        auto cutColumn = [sep](const String& row) {
            int lastSepIndex = 0;
            StringList rowContent;
            for(int index=1;index<row.size();index++) {
                if (row[index] == sep) {
                    auto content = row.mid(lastSepIndex+1, index - (lastSepIndex+1));
                    rowContent.append(content);
                    lastSepIndex = index;
                }
            }
            return rowContent;
        };
        String headerRow = lines[i].trimmed();
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
        return {
            true,
            i - startIndex,
            new Table(tab)
        };
    }
};

// 行间公式解析器
class LatexBlockParser: public BlockParser {
public:
    ParseResult parse(const StringList &lines, int startIndex) override {
        if (startIndex < lines.size() && lines[startIndex].startsWith("$$")) {
            return parseLatexBlock(lines, startIndex);
        } else {
            return ParseResult::fail();
        }
    }
private:

    ParseResult parseLatexBlock(const StringList &lines, int startIndex) {
        int i = startIndex + 1;
        while (i < lines.size()) {
            if (lines[i].startsWith("$$")) {
                break;
            }
            i++;
        }
        if (i == startIndex + 1 || i == lines.size()) return ParseResult::fail();
        String latexCode;
        for(int j=startIndex+1;j<i;j++) {
            latexCode += lines[j] + '\n';
        }
        return {
            true,
            i - startIndex,
            new LatexBlock(new Text(latexCode))
        };
    }
};

// 段落解析器(默认解析器)
class ParagraphParser: public BlockParser {
public:
    ParseResult parse(const StringList &lines, int startIndex) override {
        return parseParagraph(lines, startIndex);
    }
private:

    ParseResult parseParagraph(const StringList& lines, int lineIndex) {
        auto ret = new Paragraph();
        auto tokens = parseLine(lines[lineIndex]);
        int i = 0;
        int prev = 0;
        static std::vector<LineParser*> parsers{
                new ImageParser(),
                new LinkParser(),
                new InlineCodeParser(),
                new InlineLatexParser(),
                new SemanticTextParser(),
        };
        while (i < tokens.size()) {
            auto token = tokens[i];
            bool parsed = false;
            for(auto& it: parsers) {
                auto parseRet = it->parse(tokens, i);
                if (parseRet.success) {
                    auto text = mergeToText(tokens, prev, i);
                    if (text) ret->appendChild(text);
                    ret->appendChild(parseRet.node);
                    parsed = true;
                    prev = i + parseRet.offset;
                    i = prev;
                    break;
                }
            }
            if (!parsed) i++;
        }
        auto text = mergeToText(tokens, prev, i);
        if (text) ret->appendChild(text);
        return {
            true,
            1,
            ret
        };
    }
};

// 行间代码解析器
class CodeBlockParser: public BlockParser {
public:
    ParseResult parse(const StringList &lines, int startIndex) override {
        if (!tryParseCodeBlock(lines, startIndex)) {
            return ParseResult::fail();
        }
        return parseCodeBlock(lines, startIndex);
    }
private:
    ParseResult parseCodeBlock(const StringList& lines, int startIndex) {
        if (startIndex >= lines.size()) return ParseResult::fail();
        int i = startIndex;
        auto line = lines[i];
        i++;
        auto name = line.mid(3);
        String str;
        while (i < lines.size() && !lines[i].startsWith("```")) {
            str+= lines[i];
            str+= '\n';
            i++;
        }
        i++;
        return {
            true,
            i - startIndex,
            new CodeBlock(new Text(name), new Text(str))
        };
    }
    bool tryParseCodeBlock(const StringList &lines, int i) {
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
    ParseResult parse(const StringList &lines, int startIndex) override {
        if (startIndex >= lines.size()) return ParseResult::fail();
        auto line = lines[startIndex];
        if (line.startsWith("- [ ] ") || line.startsWith("- [x] ")) {
            return parseCheckboxList(lines, startIndex);
        } else {
            return ParseResult::fail();
        }
    }
private:
    ParseResult parseCheckboxList(const StringList& lines, int startIndex) {
        int i = startIndex;
        auto checkboxList = new CheckboxList();
        QString uncheckedPrefix = "- [ ] ";
        QString checkedPrefix = "- [x] ";
        while (i < lines.size()) {
            String line = lines[i];
            if (line.startsWith(uncheckedPrefix)) {
                auto item = new CheckboxItem();
                item->setChecked(false);
                line = line.right(line.size() - uncheckedPrefix.size());
                item->appendChild(new Text(line));
                checkboxList->appendChild(item);
                i++;
            } else if (line.startsWith(checkedPrefix)) {
                auto item = new CheckboxItem();
                item->setChecked(true);
                line = line.right(line.size() - checkedPrefix.size());
                item->appendChild(new Text(line));
                checkboxList->appendChild(item);
                i++;
            } else {
                break;
            }
        }
        return {
                true,
                i - startIndex,
                checkboxList
        };
    }
};

// 无序列表解析器
class UnorderedListParser: public BlockParser {
public:
    ParseResult parse(const StringList &lines, int startIndex) override {
        if (startIndex >= lines.size()) return ParseResult::fail();
        auto line = lines[startIndex];
        if (line.startsWith("- ")) {
            return parseUnorderedList(lines, startIndex);
        } else {
            return ParseResult::fail();
        }
    }
private:

    ParseResult parseUnorderedList(const StringList& lines, int startIndex) {
        int i = startIndex;
        auto ul = new UnorderedList();
        while (i < lines.size() && lines[i].startsWith("- ")) {
            ul->appendChild(new Text(lines[i].mid(2)));
            i++;
        }
        return {
                true,
                i - startIndex,
                ul
        };
    }
};

// 有序列表解析器
class OrderedListParser: public BlockParser {
public:
    ParseResult parse(const StringList &lines, int startIndex) override {
        if (startIndex >= lines.size()) return ParseResult::fail();
        auto line = lines[startIndex];
        if (line.startsWith("1. ")) {
            return parseOrderedList(lines, startIndex);
        } else {
            return ParseResult::fail();
        }
    }
private:

    ParseResult parseOrderedList(const StringList& lines, int startIndex) {
        int i = startIndex;
        auto ret = new OrderedList();
        while (i < lines.size()) {
            auto line = lines[i];
            bool hasDigit = false;
            int j = 0;
            while (j < line.size() && line[j].isDigit()) {
                hasDigit = true;
                j++;
            }
            if (!hasDigit) return {
                true,
                i - startIndex,
                ret
            };
            if (j >= line.size() || line[j] != '.') return {
                        true,
                        i - startIndex,
                        ret
                };
            j++;
            if (j >= line.size() || line[j] != ' ') return {
                        true,
                        i - startIndex,
                        ret
                };
            j++;
            ret->appendChild(new Text(line.mid(j)));
            // 下一行
            i++;
        }
        return {
                true,
                i - startIndex,
                ret
        };
    }
};

// 引用解析器
class QuoteBlockParser: public BlockParser {
public:
    ParseResult parse(const StringList &lines, int startIndex) override {
        if (startIndex >= lines.size()) return ParseResult::fail();
        auto line = lines[startIndex];
        if (line.startsWith("> ")) {
            return parseQuoteBlock(lines, startIndex);
        } else {
            return ParseResult::fail();
        }
    }
private:

    ParseResult parseQuoteBlock(const StringList& lines, int startIndex) {
        int i = startIndex;
        auto quoteBlock = new QuoteBlock();
        while (i < lines.size() && lines[i].startsWith("> ")) {
            quoteBlock->appendChild(new Text(lines[i].mid(2)));
            i++;
        }
        i++;
        return {
                true,
                i - startIndex,
                quoteBlock
        };
    }
};


Parser::Parser() {}

NodePtrList Parser::parse(String text) {
    static std::vector<BlockParser*> parsers = {
            new HeaderParser(),
            new CodeBlockParser(),
            new CheckboxListParser(),
            new UnorderedListParser(),
            new OrderedListParser(),
            new QuoteBlockParser(),
            new TableParser(),
            new LatexBlockParser(),
            new ParagraphParser()
    };
    NodePtrList ret;
    auto lines = text.split(QRegularExpression("(\r\n|\r|\n)"));
    int i = 0;
#if 1
    while (i < lines.size()) {
        for(auto parser: parsers) {
            auto parseRet = parser->parse(lines, i);
            if (parseRet.success) {
                i+= parseRet.offset;
                ret.push_back(parseRet.node);
                break;
            }
        }
    }
    return ret;
#endif
#if 0
    while (i < lines.size()) {
        // qDebug() << "parse" << i << lines[i];
        auto line = lines[i];
        if (line.isEmpty()) {
            i++;
            continue;
        }
        if (line.startsWith("#")) {
            if (tryParseHeader(line)) {
                auto header = parseHeader(line);
                ret.emplace_back(header);
                i++;
            } else {
                auto paragraph = parseParagraph(i, lines);
                ret.emplace_back(paragraph);
            }
        }
        else if (line.startsWith("```")) {
            if (tryParseCodeBlock(i, lines)) {
                auto codeBlock = parseCodeBlock(i, lines);
                ret.emplace_back(codeBlock);
            } else {
                auto paragraph = parseParagraph(i, lines);
                ret.emplace_back(paragraph);
            }
        }
        else if (line.startsWith("- [ ] ") || line.startsWith("- [x] ")) {
            auto checkbox = parseCheckboxList(i, lines);
            ret.emplace_back(checkbox);
        }
        else if (line.startsWith("- ")) {
            auto ul = parseUnorderedList(i, lines);
            ret.emplace_back(ul);
        }
        else if (line.startsWith("1. ")) {
            auto ol = parseOrderedList(i, lines);
            ret.emplace_back(ol);
        }
        else if (line == "---") {
            ret.emplace_back(new Hr());
            i++;
        }
        else if (line.startsWith("> ")) {
            auto quoteBlock = parseQuoteBlock(i, lines);
            ret.emplace_back(quoteBlock);
        }
        else if (line.startsWith("|")) {
            auto node = parseTable(i, lines);
            if (node == nullptr) {
                auto paragraph = parseParagraph(i, lines);
                ret.emplace_back(paragraph);
            } else {
                ret.emplace_back(node);
            }
        }
        else if (line.startsWith("$$")) {
            auto node = parseLatexBlock(i, lines);
            if (node == nullptr) {
                auto paragraph = parseParagraph(i, lines);
                ret.emplace_back(paragraph);
            } else {
                ret.emplace_back(node);
            }
        }
        else {
            auto paragraph = parseParagraph(i, lines);
            ret.emplace_back(paragraph);
        }
    }
#endif
    return ret;
}

