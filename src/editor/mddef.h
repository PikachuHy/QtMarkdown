//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_EDITOR_MDDEF_H
#define QTMARKDOWN_EDITOR_MDDEF_H
#include "parser/mddef.h"
#include "render/mddef.h"
class QPaintEvent;
class QKeyEvent;
class QMouseEvent;
class QPainter;
namespace md {
using KeyEvent = QKeyEvent;
using PaintEvent = QPaintEvent;
using MouseEvent = QMouseEvent;
using Painter = QPainter;
using Timer = QTimer;
using Point = QPoint;
}  // namespace md
#endif  // QTMARKDOWN_EDITOR_MDDEF_H
