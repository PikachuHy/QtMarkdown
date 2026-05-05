#ifndef QTMARKDOWN_QTIMAGEPROVIDER_H
#define QTMARKDOWN_QTIMAGEPROVIDER_H

#include <QFile>
#include <QImage>
#include <QString>

#include "core/IImageProvider.h"
#include "QtAdapters.h"

namespace md::editor {

class QtImageProvider : public core::IImageProvider {
public:
    core::ImageData load(const String& path) override {
        QImage img(toQString(path));
        if (img.isNull()) return {};
        return fromQImage(img);
    }

    bool exists(const String& path) override {
        return QFile::exists(toQString(path));
    }
};

} // namespace md::editor
#endif // QTMARKDOWN_QTIMAGEPROVIDER_H
