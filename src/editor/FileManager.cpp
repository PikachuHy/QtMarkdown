//
// Created by PikachuHy on 2021/11/5.
//

#include "FileManager.h"
#include "Document.h"
#include "MarkdownSerializer.h"
#include "debug.h"

#include <filesystem>
#include <fstream>
#include <sstream>

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
    if (!std::filesystem::exists(notePath.toStdString())) {
        DEBUG << "file not exist:" << notePath;
        return {false, ""};
    }
    std::ifstream file(notePath.toStdString(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        DEBUG << "file open fail:" << notePath;
        return {false, ""};
    }
    auto size = file.tellg();
    file.seekg(0);
    std::string content(static_cast<size_t>(size), '\0');
    file.read(content.data(), static_cast<std::streamsize>(size));
    return {true, String(std::move(content))};
}

bool FileManager::saveToFile(const String& path) const {
    String notePath = path;
    if (!notePath.endsWith(".md")) {
        notePath += ".md";
    }
    DEBUG << "note path" << notePath;
    std::ofstream file(notePath.toStdString(), std::ios::binary);
    if (!file.is_open()) {
        DEBUG << "file open fail:" << notePath;
        return false;
    }
    MarkdownSerializer serializer(m_doc.bufferProvider());
    m_doc.accept(&serializer);
    auto mdText = serializer.markdown();
    file.write(mdText.data(), static_cast<std::streamsize>(mdText.size()));
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
        s += textNode->toString(m_doc.bufferProvider());
    }
    return s;
}

} // namespace md::editor
