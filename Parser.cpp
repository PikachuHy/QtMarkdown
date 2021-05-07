//
// Created by pikachu on 2021/1/31.
//

#include "Parser.h"
#include <regex>
#include "Document.h"
#include <QRegularExpression>
#include <QDebug>
#define emplace_back push_back
using namespace std::string_view_literals;

bool tryParseHeader(String line) {
    int i = 0;
    while (i < line.size() && line[i] == '#') i++;
    // 如果没有#或#的个数超过6个
    if (i == 0 || i > 6) return false;
    // #后面要接一个空格
    if (i < line.size() && line[i] == ' ') return true;
    return false;
}
// ![]()
bool tryParseImage(TokenList& tokens, int startIndex) {
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

// []()
bool tryParseLink(TokenList& tokens, int startIndex) {
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
bool tryParseInlineCode(TokenList& tokens, int startIndex) {
    int i = startIndex + 1;
    while (i < tokens.size()) {
        if (isBackquote(tokens[i])) {
            return true;
        }
        i++;
    }
    return false;
}
// ```
// ```
bool tryParseCodeBlock(int i, const StringList &lines) {
    if (i >= lines.size() || !lines[i].startsWith("```")) return false;
    i++;
    while (i < lines.size() && !lines[i].startsWith("```")) i++;
    if (i >= lines.size() || !lines[i].startsWith("```")) return false;
    return true;
}
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
bool tryParseBold(TokenList& tokens, int startIndex) {
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

bool tryParseItalicAndBold(TokenList& tokens, int startIndex) {
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

Node *parseInlineLatex(TokenList& tokens, int &i) {
    i++;
    String latex;
    while (!isDollar(tokens[i])) {
        latex += tokens[i].str();
        i++;
    }
    i++;
    return new InlineLatex(new Text(latex));
}
Node *parseItalic(TokenList& tokens, int &i) {
    i++;
    auto ret = new ItalicText(tokens[i].str());
    i++;
    i++;
    return ret;
}
Node *parseBold(TokenList& tokens, int &i) {
    i++;
    i++;
    auto ret = new BoldText(tokens[i].str());
    i++;
    i++;
    i++;
    return ret;
}
Node *parseItalicAndBold(TokenList& tokens, int &i) {
    i++;
    i++;
    i++;
    auto ret = new BoldText(tokens[i].str());
    i++;
    i++;
    i++;
    i++;
    return ret;
}
Node *parseHeader(String line) {
    int i = 0;
    while (i < line.size() && line[i] == '#') i++;
    int level = i;
    // 跳过空格
    i++;
    String str = line.mid(i);
    auto ret = new Header(level, str);
    return ret;
}
TokenList parseLine(String text);
Text *mergeToText(const TokenList& tokens, int prev, int cur) {
    if (prev >= cur) return nullptr;
    String str;
    for(int i=prev;i<cur;i++) {
        str += tokens[i].str();
    }
    return new Text(str);
}
Node *parseImage(TokenList& tokens, int &i) {
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
    return new Image(alt, url);
}
Node *parseLink(TokenList& tokens, int &i) {
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
    return new Link(content, href);
}
Node *parseInlineCode(TokenList& tokens, int &i) {
    i++; // `
    int prev = i;
    while (!isBackquote(tokens[i])) i++;
    auto code = mergeToText(tokens, prev, i);
    i++; // `
    i++;
    return new InlineCode(code);
}

Node* parseTable(int& startIndex, const StringList &lines) {
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
    if (headerRow.back() != sep) return nullptr;
    int colNum = countColumn(lines[i]);
    auto header = cutColumn(headerRow);
    i++;
    if (i >= lines.size()) return nullptr;
    int secondColNum = countColumn(lines[i]);
    if (secondColNum != colNum) return nullptr;
    i++;
    Table tab;
    tab.setHeader(header);
    while (i < lines.size() && !lines[i].isEmpty() && lines[i].front() == sep) {
        auto curRow = lines[i].trimmed();
        int curColNum = countColumn(curRow);
        if (curColNum != colNum) return nullptr;
        auto rowContent = cutColumn(curRow);
        tab.appendRow(rowContent);
        i++;
    }
    startIndex = i;
    return new Table(tab);
}
Node* parseLatexBlock(int& startIndex, const StringList &lines) {
    int i = startIndex + 1;
    while (i < lines.size()) {
        if (lines[i].startsWith("$$")) {
            break;
        }
        i++;
    }
    if (i == startIndex + 1 || i == lines.size()) return nullptr;
    String latexCode;
    for(int j=startIndex+1;j<i;j++) {
        latexCode += lines[j] + '\n';
    }
    startIndex = i + 1;
    return new LatexBlock(new Text(latexCode));
}
Node *parseParagraph(int &lineIndex, StringList& lines) {
    auto ret = new Paragraph();
    auto tokens = parseLine(lines[lineIndex]);
    int i = 0;
    int prev = 0;
    while (i < tokens.size()) {
        auto token = tokens[i];
        if (isExclamation(token)) {
            if (tryParseImage(tokens, i)) {
                auto text = mergeToText(tokens, prev, i);
                if (text) ret->appendChild(text);
                auto image = parseImage(tokens, i);
                ret->appendChild(image);
                prev = i;
            } else {
                // parse fail. forward
                i++;
            }
        }
        else if (isLeftBracket(token)) {
            if (tryParseLink(tokens, i)) {
                auto text = mergeToText(tokens, prev, i);
                if (text) ret->appendChild(text);
                auto link = parseLink(tokens, i);
                ret->appendChild(link);
                prev = i;
            } else {
                // parse fail. forward
                i++;
            }
        }
        else if (isBackquote(token)) {
            if (tryParseInlineCode(tokens, i)) {
                auto text = mergeToText(tokens, prev, i);
                if (text) ret->appendChild(text);
                auto inlineCode = parseInlineCode(tokens, i);
                ret->appendChild(inlineCode);
                prev = i;
            } else {
                // parse fail. forward
                i++;
            }
        }
        else if (isDollar(token)) {
            if (tryParseInlineCode(tokens, i)) {
                auto text = mergeToText(tokens, prev, i);
                if (text) ret->appendChild(text);
                auto node = parseInlineLatex(tokens, i);
                ret->appendChild(node);
                prev = i;
            } else {
                // parse fail. forward
                i++;
            }
        }
        else if (isStar(token)) {
            if (tryParseItalic(tokens, i)) {
                auto text = mergeToText(tokens, prev, i);
                if (text) ret->appendChild(text);
                auto node = parseItalic(tokens, i);
                ret->appendChild(node);
                prev = i;
            }
            else if (tryParseBold(tokens, i)) {
                auto text = mergeToText(tokens, prev, i);
                if (text) ret->appendChild(text);
                auto node = parseBold(tokens, i);
                ret->appendChild(node);
                prev = i;
            }
            else if (tryParseItalicAndBold(tokens, i)) {
                auto text = mergeToText(tokens, prev, i);
                if (text) ret->appendChild(text);
                auto node = parseItalicAndBold(tokens, i);
                ret->appendChild(node);
                prev = i;
            } else {
                // parse fail. forward
                i++;
            }
        }
        else {
            i++;
        }
    }
    auto text = mergeToText(tokens, prev, i);
    if (text) ret->appendChild(text);
    // TODO: 考虑合并多行
    lineIndex++;
    return ret;
}
Node *parseCodeBlock(int &i, StringList& lines) {
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
    return new CodeBlock(new Text(name), new Text(str));
}
Node* parseUnorderedList(int &i, StringList& lines) {
    auto ret = new UnorderedList();
    while (i < lines.size() && lines[i].startsWith("- ")) {
        ret->appendChild(new Text(lines[i].mid(2)));
        i++;
    }
    i++;
    return ret;
}
Node* parseOrderedList(int &i, StringList& lines) {
    auto ret = new OrderedList();
    while (i < lines.size()) {
        auto line = lines[i];
        bool hasDigit = false;
        int j = 0;
        while (j < line.size() && line[j].isDigit()) {
            hasDigit = true;
            j++;
        }
        if (!hasDigit) return ret;
        if (j >= line.size() || line[j] != '.') return ret;
        j++;
        if (j >= line.size() || line[j] != ' ') return ret;
        j++;
        ret->appendChild(new Text(line.mid(j)));
        // 下一行
        i++;
    }
    i++;
    return ret;
}
Node* parseQuoteBlock(int &i, StringList& lines) {
    auto ret = new QuoteBlock();
    while (i < lines.size() && lines[i].startsWith("> ")) {
        ret->appendChild(new Text(lines[i].mid(2)));
        i++;
    }
    i++;
    return ret;
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

Parser::Parser() {}

NodePtrList Parser::parse(String text) {
    NodePtrList ret;
    auto lines = text.split(QRegularExpression("(\r\n|\r|\n)"));
    int i = 0;
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
    return ret;
}

