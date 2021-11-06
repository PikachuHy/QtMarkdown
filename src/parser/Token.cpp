//
// Created by pikachu on 2021/1/31.
//

#include "Token.h"

namespace md::parser {

bool Token::operator==(const Token &rhs) const {
  return m_type == rhs.m_type && m_offset == rhs.m_offset && m_length == rhs.m_length;
}

bool Token::operator!=(const Token &rhs) const { return !(rhs == *this); }
}  // namespace md::parser