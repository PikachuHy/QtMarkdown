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

#include "IBufferProvider.h"
#include "Node.h"
#include "nodes/Header.h"
#include "nodes/Paragraph.h"
#include "nodes/CheckboxList.h"
#include "nodes/UnorderedList.h"
#include "nodes/OrderedList.h"
#include "nodes/QuoteBlock.h"
#include "nodes/ItalicText.h"
#include "nodes/BoldText.h"
#include "nodes/ItalicBoldText.h"
#include "nodes/StrickoutText.h"
#include "nodes/Image.h"
#include "nodes/Link.h"
#include "nodes/CodeBlock.h"
#include "nodes/InlineCode.h"
#include "nodes/InlineLatex.h"
#include "nodes/Hr.h"
#include "nodes/Table.h"
#include "nodes/Lf.h"
#include "nodes/LatexBlock.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT Document : public IBufferProvider {
 public:
  explicit Document(const String& str);
  String toHtml();
  void accept(NodeVisitor* visitor);
  Container* root() const { return m_root.get(); }
  String& addBuffer() { return m_addBuffer; }
  const String& addBuffer() const override { return m_addBuffer; }
  const String& originalBuffer() const override { return m_originalBuffer; }

 protected:
  String m_originalBuffer;
  String m_addBuffer;
  sptr<Container> m_root;
  friend class Parser;
  friend class Text;
};

}  // namespace md::parser
#endif  // MD_DOCUMENT_H
