//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_TOKEN_H
#define MD_TOKEN_H
#include <QList>
#include <QMap>
#include <QString>

#include "QtMarkdown_global.h"
#include "mddef.h"
namespace md::parser {
enum class TokenType {
  none,
  sharp,              // #
  space,              //
  left_parenthesis,   // (
  right_parenthesis,  // )
  left_bracket,       // [
  right_bracket,      // ]
  exclamation,        // !
  gt,                 // >
  quotation_en,       // "
  star,               // *
  tilde,              // ~
  text,
  backquote,  // `
  dollar
};
class QTMARKDOWNSHARED_EXPORT Token {
 public:
  explicit Token(SizeType offset, SizeType length, TokenType type = TokenType::none)
      : m_offset(offset), m_length(length), m_type(type) {}

  bool operator==(const Token& rhs) const;

  bool operator!=(const Token& rhs) const;

  [[nodiscard]] TokenType type() const { return m_type; }
  [[nodiscard]] SizeType offset() const { return m_offset; }
  [[nodiscard]] SizeType length() const { return m_length; }

 protected:
  TokenType m_type;
  SizeType m_offset;
  SizeType m_length;
};
extern QMap<QChar, Token> spMap;
inline bool isSharp(const Token& token) { return token.type() == TokenType::sharp; }
inline bool isSpace(const Token& token) { return token.type() == TokenType::space; }
inline bool isLeftParenthesis(const Token& token) { return token.type() == TokenType::left_parenthesis; }
inline bool isRightParenthesis(const Token& token) { return token.type() == TokenType::right_parenthesis; }
inline bool isLeftBracket(const Token& token) { return token.type() == TokenType::left_bracket; }
inline bool isRightBracket(const Token& token) { return token.type() == TokenType::right_bracket; }
inline bool isExclamation(const Token& token) { return token.type() == TokenType::exclamation; }
inline bool isGt(const Token& token) { return token.type() == TokenType::gt; }
inline bool isQuotationEn(const Token& token) { return token.type() == TokenType::quotation_en; }
inline bool isStar(const Token& token) { return token.type() == TokenType::star; }
inline bool isTilde(const Token& token) { return token.type() == TokenType::tilde; }
inline bool isText(const Token& token) { return token.type() == TokenType::text; }
inline bool isBackquote(const Token& token) { return token.type() == TokenType::backquote; }
inline bool isDollar(const Token& token) { return token.type() == TokenType::dollar; }
}  // namespace md::parser
#endif  // MD_TOKEN_H
