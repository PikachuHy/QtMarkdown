//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_H
#define MD_PARSER_H
#include "QtMarkdown_global.h"
#include "mddef.h"
namespace md::parser {
class Container;
class QTMARKDOWNSHARED_EXPORT Parser {
 public:
  static sptr<Container> parse(const String& text);
};
}  // namespace md::parser

#endif  // MD_PARSER_H
