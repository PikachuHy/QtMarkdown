#ifndef QTMARKDOWN_PARSECONTEXT_H
#define QTMARKDOWN_PARSECONTEXT_H

#include "PieceTable.h"
#include "mddef.h"

namespace md::parser {

struct ParseContext {
    PieceTableItem::BufferType bufferType = PieceTableItem::original;
    SizeType baseOffset = 0;
};

namespace detail {
extern thread_local ParseContext tls_parseContext;
}  // namespace detail

// RAII guard that sets the per-thread parse context for the duration of a parse.
// All Text nodes constructed within the guard's scope use this context.
class ParseContextGuard {
public:
    explicit ParseContextGuard(PieceTableItem::BufferType bufferType, SizeType baseOffset)
        : m_saved(detail::tls_parseContext) {
        detail::tls_parseContext.bufferType = bufferType;
        detail::tls_parseContext.baseOffset = baseOffset;
    }
    ~ParseContextGuard() { detail::tls_parseContext = m_saved; }
    ParseContextGuard(const ParseContextGuard&) = delete;
    ParseContextGuard& operator=(const ParseContextGuard&) = delete;

    // Returns the current thread's active parse context.
    static const ParseContext& current() { return detail::tls_parseContext; }
private:
    ParseContext m_saved;
};

}  // namespace md::parser

#endif  // QTMARKDOWN_PARSECONTEXT_H
