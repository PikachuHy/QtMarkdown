//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_DOCUMENT_H
#define QTMARKDOWN_DOCUMENT_H
#include "mddef.h"
#include "parser/Document.h"
#include "render/Instruction.h"
namespace md::editor {
class Document : public parser::Document, public std::enable_shared_from_this<Document> {
 public:
  explicit Document(const String& str);

 private:
  QList<render::InstructionGroup> m_instructionGroups;
  friend class Editor;
};
}  // namespace md::editor

#endif  // QTMARKDOWN_DOCUMENT_H
