#ifndef QTMARKDOWN_CORE_IIMAGEPROVIDER_H
#define QTMARKDOWN_CORE_IIMAGEPROVIDER_H

#include "Types.h"

namespace md::editor::core {

class IImageProvider {
public:
    virtual ~IImageProvider() = default;
    virtual ImageData load(const String& path) = 0;
    virtual bool exists(const String& path) = 0;
};

} // namespace md::editor::core
#endif // QTMARKDOWN_CORE_IIMAGEPROVIDER_H
