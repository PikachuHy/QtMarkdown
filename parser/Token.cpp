//
// Created by pikachu on 2021/1/31.
//

#include "Token.h"

using namespace std::string_view_literals;
Token::Token(TokenType type) : m_type(type) {
    switch (type) {
        case TokenType::none:
            break;
        case TokenType::sharp:
            m_str = "#";
            break;
        case TokenType::space:
            m_str = " ";
            break;
        case TokenType::left_parenthesis:
            m_str = "(";
            break;
        case TokenType::right_parenthesis:
            m_str = ")";
            break;
        case TokenType::left_bracket:
            m_str = "[";
            break;
        case TokenType::right_bracket:
            m_str = "]";
            break;
        case TokenType::exclamation:
            m_str = "!";
            break;
        case TokenType::gt:
            m_str = ">";
            break;
        case TokenType::quotation_en:
            m_str = R"(")";
            break;
        case TokenType::star:
            m_str = "*";
            break;
        case TokenType::tilde:
            m_str = "~";
            break;
        case TokenType::backquote:
            m_str = "`";
            break;
        case TokenType::text:
            break;
        case TokenType::dollar:
            m_str = "$";
            break;
    }
}

Token::Token(String str, TokenType type): m_str(str), m_type(type) {

}

bool Token::operator==(const Token &rhs) const {
    return m_type == rhs.m_type &&
           m_str == rhs.m_str;
}

bool Token::operator!=(const Token &rhs) const {
    return !(rhs == *this);
}
QMap<QChar, Token> spMap = {
        {'#', Token(TokenType::sharp)},
//        {' ', Token(TokenType::space)},
        {'>', Token(TokenType::gt)},
        {'!', Token(TokenType::exclamation)},
        {'*', Token(TokenType::star)},
        {'~', Token(TokenType::tilde)},
        {'[', Token(TokenType::left_bracket)},
        {']', Token(TokenType::right_bracket)},
        {'(', Token(TokenType::left_parenthesis)},
        {')', Token(TokenType::right_parenthesis)},
        {'`', Token(TokenType::backquote)},
        {'$', Token(TokenType::dollar)},
};