//
// Created by PikachuHy on 2021/11/5.
//

#include "Text.h"
namespace md::parser {
String Text::toString(const DocPtr& doc) const {
  String s;
  for (auto item : m_items) {
    s += item.toString(doc);
  }
  return s;
}

String LatexBlock::toString(DocPtr const& doc) const {
  String s;
  for (auto node : m_children) {
    if (node->type() == NodeType::text) {
      auto text = static_cast<Text*>(node);
      s += text->toString(doc);
    }
  }
  return s;
}
}  // namespace md::parser