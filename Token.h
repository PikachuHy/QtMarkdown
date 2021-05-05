//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_TOKEN_H
#define MD_TOKEN_H
#include <string_view>
#include <QString>
#include <QList>
#include <QMap>
enum class TokenType {
    none,
    sharp, // #
    space, //
    left_parenthesis, // (
    right_parenthesis, // )
    left_bracket, // [
    right_bracket, // ]
    exclamation, // !
    gt, // >
    quotation_en, // "
    star, // *
    text,
    backquote, // `
    dollar
};
//using String = std::string_view;
using String = QString;
using StringList = QStringList;
class Token {
public:
    explicit Token(TokenType type = TokenType::none);

    explicit Token(String str, TokenType type = TokenType::none);

    bool operator==(const Token &rhs) const;

    bool operator!=(const Token &rhs) const;

    TokenType type() const { return m_type; }
    String str() const { return m_str; }
protected:
    TokenType m_type;
    String m_str;
};
using TokenList = std::vector<Token>;
extern QMap<QChar, Token> spMap;
inline bool isSharp(Token token) { return token.type() == TokenType::sharp; }
inline bool isSpace(Token token) { return token.type() == TokenType::space; }
inline bool isLeftParenthesis(Token token) { return token.type() == TokenType::left_parenthesis; }
inline bool isRightParenthesis(Token token) { return token.type() == TokenType::right_parenthesis; }
inline bool isLeftBracket(Token token) { return token.type() == TokenType::left_bracket; }
inline bool isRightBracket(Token token) { return token.type() == TokenType::right_bracket; }
inline bool isExclamation(Token token) { return token.type() == TokenType::exclamation; }
inline bool isGt(Token token) { return token.type() == TokenType::gt; }
inline bool isQuotationEn(Token token) { return token.type() == TokenType::quotation_en; }
inline bool isStar(Token token) { return token.type() == TokenType::star; }
inline bool isText(Token token) { return token.type() == TokenType::text; }
inline bool isBackquote(Token token) { return token.type() == TokenType::backquote; }
inline bool isDollar(Token token) { return token.type() == TokenType::dollar; }
#endif //MD_TOKEN_H
