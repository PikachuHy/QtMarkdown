//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_RENDER_H
#define QTMARKDOWN_RENDER_H
#include "Instruction.h"
#include "mddef.h"
#include "parser/Document.h"
namespace md::render {
class Render {
 public:
  static InstructionGroup render(parser::Node* node, DocPtr doc);

 private:
};
}  // namespace md::render
#endif  // QTMARKDOWN_RENDER_H
