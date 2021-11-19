//
// Created by PikachuHy on 2021/11/19.
//

#ifndef QTMARKDOWN_ELEMENT_H
#define QTMARKDOWN_ELEMENT_H
#include "mddef.h"
namespace md::parser {
class Node;
}
namespace md::render {
struct Element {
  md::parser::Node* node;
  Point pos;
  Size size;
};
using ElementList = std::vector<Element>;
}  // namespace md::render
#endif  // QTMARKDOWN_ELEMENT_H
