#ifndef QTMARKDOWN_PLATFORM_QTLATEXPLATFORM_H
#define QTMARKDOWN_PLATFORM_QTLATEXPLATFORM_H

#include "microtex.h"
#include "graphic_qt.h"

namespace md::platform::qt {

inline void initLatex() {
    static bool initialized = false;
    if (!initialized) {
        microtex::PlatformFactory::registerFactory("qt", std::make_unique<microtex::PlatformFactory_qt>());
        microtex::PlatformFactory::activate("qt");
        microtex::MicroTeX::setRenderGlyphUsePath(true);
        initialized = true;
    }
}

} // namespace md::platform::qt
#endif // QTMARKDOWN_PLATFORM_QTLATEXPLATFORM_H
