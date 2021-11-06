//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_RENDER_MDDEF_H
#define QTMARKDOWN_RENDER_MDDEF_H
#include <QBrush>

#include "parser/mddef.h"
namespace md {
namespace render {
class Instruction;
}
using InstructionPtrList = std::vector<render::Instruction*>;
using Brush = QBrush;
using Color = QColor;
}  // namespace md
#endif  // QTMARKDOWN_RENDER_MDDEF_H
