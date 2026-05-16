#ifndef TEST_NULLIMAGEPROVIDER_H
#define TEST_NULLIMAGEPROVIDER_H

#include "editor/core/IImageProvider.h"

namespace md::editor::core {
class NullImageProvider : public IImageProvider {
public:
    ImageData load(const String&) override { return {}; }
    bool exists(const String&) override { return false; }
};
}  // namespace md::editor::core

#endif
