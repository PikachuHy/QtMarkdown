//
// Created by PikachuHy on 2021/11/5.
//

#include "nodes/LatexBlock.h"
#include "Text.h"

namespace md::parser {
String LatexBlock::toString(const IBufferProvider& doc) const {
  String s;
  for (auto& node : m_children) {
    if (node->type() == NodeType::text) {
      auto text = static_cast<Text*>(node.get());
      s += text->toString(doc);
    }
  }
  return s;
}
}  // namespace md::parser
