//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_RENDER_MDDEF_H
#define QTMARKDOWN_RENDER_MDDEF_H
#include <QBrush>
#include <QFont>
#include "parser/mddef.h"
namespace md {
namespace render {
class Instruction;
}
using InstructionPtr = render::Instruction*;
using InstructionPtrList = std::vector<InstructionPtr>;
using Brush = QBrush;
using Color = QColor;
using Painter = QPainter;
using Point = QPoint;
using Rect = QRect;
using Size = QSize;
using Font = QFont;
}  // namespace md
#endif  // QTMARKDOWN_RENDER_MDDEF_H
