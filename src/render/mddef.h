//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_RENDER_MDDEF_H
#define QTMARKDOWN_RENDER_MDDEF_H
#include <QBrush>
#include <QFont>
#include <QString>
#include "parser/mddef.h"
namespace md {
namespace render {
class Instruction;
}
using InstructionPtr = std::unique_ptr<render::Instruction>;
using InstructionPtrList = std::vector<InstructionPtr>;
using Brush = QBrush;
using Color = QColor;
using Painter = QPainter;
using Point = QPoint;
using Rect = QRect;
using Size = QSize;
using Font = QFont;

// Convert md::String (UTF-8 std::string) to QString for Qt bridge code
inline QString toQString(const String& s) {
    return QString::fromUtf8(s.data(), static_cast<int>(s.size()));
}
}  // namespace md
#endif  // QTMARKDOWN_RENDER_MDDEF_H
