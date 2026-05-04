#ifndef QTMARKDOWN_IBUFFERPROVIDER_H
#define QTMARKDOWN_IBUFFERPROVIDER_H

#include "QtMarkdown_global.h"
#include "mddef.h"

namespace md::parser {

class QTMARKDOWNSHARED_EXPORT IBufferProvider {
public:
    virtual ~IBufferProvider() = default;
    virtual const String& originalBuffer() const = 0;
    virtual const String& addBuffer() const = 0;
};

} // namespace md::parser

#endif // QTMARKDOWN_IBUFFERPROVIDER_H
