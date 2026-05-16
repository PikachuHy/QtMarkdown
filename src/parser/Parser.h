//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_H
#define MD_PARSER_H
#include "PieceTable.h"
#include "QtMarkdown_global.h"
#include "mddef.h"
namespace md::parser {
class Container;
class QTMARKDOWNPARSER_EXPORT Parser {
 public:
  static std::unique_ptr<Container> parse(const String& text);
  static std::unique_ptr<Container> parse(const String& text, PieceTableItem::BufferType bufferType, SizeType baseOffset = 0);
};
}  // namespace md::parser

#endif  // MD_PARSER_H
