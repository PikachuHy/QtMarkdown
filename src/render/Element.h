//
// Created by PikachuHy on 2021/11/19.
//

#ifndef QTMARKDOWN_ELEMENT_H
#define QTMARKDOWN_ELEMENT_H
#include "QtMarkdown_global.h"
#include "mddef.h"
namespace md::parser {
class Node;
}
namespace md::render {
class QTMARKDOWNSHARED_EXPORT Element {
 public:
  md::parser::Node* node;
  Point pos;
  Size size;
};
using ElementList = std::vector<Element>;
}  // namespace md::render
#endif  // QTMARKDOWN_ELEMENT_H
