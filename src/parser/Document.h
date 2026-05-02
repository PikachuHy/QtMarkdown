//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_DOCUMENT_H
#define MD_DOCUMENT_H
#include <QDebug>
#include <QRect>
#include <iostream>
#include <memory>

#include "QtMarkdown_global.h"
#include "Token.h"
#include "Visitor.h"
#include "mddef.h"

#include "Node.h"
#include "Header.h"
#include "Paragraph.h"
#include "CheckboxList.h"
#include "UnorderedList.h"
#include "OrderedList.h"
#include "QuoteBlock.h"
#include "ItalicText.h"
#include "BoldText.h"
#include "ItalicBoldText.h"
#include "StrickoutText.h"
#include "Image.h"
#include "Link.h"
#include "CodeBlock.h"
#include "InlineCode.h"
#include "InlineLatex.h"
#include "Hr.h"
#include "Table.h"
#include "Lf.h"
#include "LatexBlock.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT Document {
 public:
  explicit Document(const String& str);
  String toHtml();
  void accept(VisitorNode* visitor);
  Container* root() const { return m_root.get(); }
  String& addBuffer() { return m_addBuffer; }
  const String& addBuffer() const { return m_addBuffer; }

 protected:
  String m_originalBuffer;
  String m_addBuffer;
  sptr<Container> m_root;
  friend class Parser;
  friend class Text;
  friend class PieceTableItem;
};

}  // namespace md::parser
#endif  // MD_DOCUMENT_H
