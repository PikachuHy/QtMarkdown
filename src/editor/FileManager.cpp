//
// Created by PikachuHy on 2021/11/5.
//

#include "FileManager.h"
#include "Document.h"
#include "MarkdownSerializer.h"
#include "debug.h"

#include <QFile>

using namespace md::parser;

namespace md::editor {

FileManager::FileManager(const Document& doc) : m_doc(doc) {}

std::pair<bool, String> FileManager::loadFile(const String& path) {
    DEBUG << path;
    String notePath = path;
    String prefix = "file://";
    if (path.startsWith(prefix)) {
        notePath = path.mid(prefix.size());
    }
    QFile file(notePath);
    if (!file.exists()) {
        DEBUG << "file not exist:" << notePath;
        return {false, ""};
    }
    if (!file.open(QIODevice::ReadOnly)) {
        DEBUG << "file open fail:" << notePath;
        return {false, ""};
    }
    auto mdText = file.readAll();
    return {true, mdText};
}

bool FileManager::saveToFile(const String& path) const {
    String notePath = path;
    if (!notePath.endsWith(".md")) {
        notePath += ".md";
    }
    DEBUG << "note path" << notePath;
    QFile file(notePath);
    if (!file.open(QIODevice::WriteOnly)) {
        DEBUG << "file open fail:" << notePath;
        return false;
    }
    MarkdownSerializer serializer(m_doc.parserDoc());
    m_doc.parserDoc()->accept(&serializer);
    auto mdText = serializer.markdown();
    file.write(mdText.toUtf8());
    file.close();
    return true;
}

String FileManager::title() const {
    if (m_doc.root()->empty()) return "";
    auto node = m_doc.root()->childAt(0);
    if (node->type() != NodeType::header) return "";
    auto header = static_cast<Header*>(node);
    if (header->level() != 1) return "";
    String s;
    for (auto& child : header->children()) {
        if (child->type() != NodeType::text) continue;
        auto textNode = static_cast<Text*>(child.get());
        s += textNode->toString(m_doc.parserDoc());
    }
    return s;
}

} // namespace md::editor
