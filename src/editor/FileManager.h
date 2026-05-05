//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_FILEMANAGER_H
#define QTMARKDOWN_FILEMANAGER_H

#include "QtMarkdown_global.h"
#include "render/mddef.h"

namespace md::editor {
class Document;

class QTMARKDOWNSHARED_EXPORT FileManager {
public:
    explicit FileManager(const Document& doc);

    // Static: pure file I/O, no Document needed. Returns {ok, fileContents}.
    static std::pair<bool, String> loadFile(const String& path);

    // Instance methods: need Document for serialization/metadata.
    bool saveToFile(const String& path) const;
    String title() const;

private:
    const Document& m_doc;
};

} // namespace md::editor
#endif // QTMARKDOWN_FILEMANAGER_H
