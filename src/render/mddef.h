//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_RENDER_MDDEF_H
#define QTMARKDOWN_RENDER_MDDEF_H
#include "parser/mddef.h"
#include "core/Types.h"
namespace md::editor::core {
class AbstractPainter;
}
namespace md {
namespace render {
class Instruction;
}
using InstructionPtr = std::unique_ptr<render::Instruction>;
using InstructionPtrList = std::vector<InstructionPtr>;
using Color = editor::core::Color;
using Painter = editor::core::AbstractPainter;
using Point = editor::core::Point;
using Rect = editor::core::Rect;
using Size = editor::core::Size;
using Font = editor::core::FontDescription;
}  // namespace md
#endif  // QTMARKDOWN_RENDER_MDDEF_H
