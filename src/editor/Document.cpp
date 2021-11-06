//
// Created by PikachuHy on 2021/11/5.
//

#include "Document.h"

#include "debug.h"
#include "parser/Parser.h"
#include "render/Render.h"
using namespace md::parser;
using namespace md::render;
namespace md::editor {
Document::Document(const String& str) : parser::Document(str) {
  for (auto node : m_root->children()) {
    m_instructionGroups.append(Render::render(node, this));
  }
}
}  // namespace md::editor