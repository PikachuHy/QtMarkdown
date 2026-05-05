#ifndef QTMARKDOWN_PARSECONTEXT_H
#define QTMARKDOWN_PARSECONTEXT_H

#include "PieceTable.h"
#include "mddef.h"

namespace md::parser {

struct ParseContext {
    PieceTableItem::BufferType bufferType = PieceTableItem::original;
    SizeType baseOffset = 0;
};

extern thread_local ParseContext g_parseContext;

}  // namespace md::parser

#endif  // QTMARKDOWN_PARSECONTEXT_H
