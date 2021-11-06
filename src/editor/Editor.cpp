//
// Created by PikachuHy on 2021/11/5.
//

#include "Editor.h"

#include <QFile>
#include <memory>

#include "debug.h"
#include "render/Instruction.h"
#include "render/Render.h"
namespace md::editor {
void Editor::loadFile(const String& path) {
  QFile file(path);
  if (!file.exists()) {
    DEBUG << "file not exist:" << path;
    return;
  }
  if (!file.open(QIODevice::ReadOnly)) {
    DEBUG << "file open fail:" << path;
    return;
  }
  auto mdText = file.readAll();
  m_doc = std::make_shared<Document>(mdText);
}
void Editor::paintEvent(QPoint offset, Painter& painter) {
  if (!m_doc) return;
  for (const auto& instructionGroup : m_doc->m_instructionGroups) {
    auto h = instructionGroup.height();
    for (const auto& instructionLine : instructionGroup) {
      for (auto instruction : instructionLine) {
        instruction->run(painter, offset, m_doc.get());
      }
    }
    offset.setY(offset.y() + h);
  }
}
int Editor::width() const { return 800; }
int Editor::height() const {
  auto h = 0;
  for (const auto& instructionGroup : m_doc->m_instructionGroups) {
    h += instructionGroup.height();
  }
  return h;
}
}  // namespace md::editor