//
// Created by PikachuHy on 2021/11/5.
//

#include "Editor.h"

#include <QFile>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QStringList>
#include <memory>

#include "Cursor.h"
#include "debug.h"
#include "parser/Text.h"
#include "render/Instruction.h"
#include "render/Render.h"
using namespace md::parser;
namespace md::editor {
class SimpleMarkdownVisitor
    : public MultipleVisitor<Header, Text, ItalicText, BoldText, ItalicBoldText, Image, Link, CodeBlock, InlineCode,
                             Paragraph, CheckboxList, CheckboxItem, UnorderedList, OrderedList, UnorderedListItem,
                             OrderedListItem, Hr, QuoteBlock, Table, Lf> {
 public:
  explicit SimpleMarkdownVisitor(DocPtr doc) : m_doc(doc) {}
  void visit(Header *node) override {
    for (int i = 0; i < node->level(); ++i) {
      m_md += "#";
    }
    m_md += " ";
    for (auto it : node->children()) {
      it->accept(this);
    }
    m_md += "\n";
  }
  void visit(Text *node) override { m_md += node->toString(m_doc); }
  void visit(ItalicText *node) override {
    m_md += "*";
    node->text()->accept(this);
    m_md += "*";
  }
  void visit(BoldText *node) override {
    m_md += "**";
    node->text()->accept(this);
    m_md += "**";
  }
  void visit(ItalicBoldText *node) override {
    m_md += "***";
    node->text()->accept(this);
    m_md += "***";
  }
  void visit(Image *node) override {
    m_md += "![";
    if (node->alt()) {
      node->alt()->accept(this);
    } else {
      qDebug() << "image alt is null";
    }
    m_md += "]";
    m_md += "(";
    if (node->src()) {
      node->src()->accept(this);
    } else {
      qDebug() << "image src is null";
    }
    m_md += ")";
  }
  void visit(Link *node) override {
    m_md += "[";
    if (node->href()) {
      node->href()->accept(this);
    } else {
      qDebug() << "link href is null";
    }
    m_md += "]";
    m_md += "(";
    if (node->content()) {
      node->content()->accept(this);
    } else {
      qDebug() << "link content is null";
    }
    m_md += ")";
  }
  void visit(CodeBlock *node) override {
    m_md += "```";
    node->name()->accept(this);
    m_md += "\n";
    for (auto child : node->children()) {
      child->accept(this);
      m_md += "\n";
    }
    m_md += "```";
    m_md += "\n";
  }
  void visit(InlineCode *node) override {
    m_md += "`";
    if (auto code = node->code(); code) {
      code->accept(this);
    }
    m_md += "`";
  }
  void visit(Paragraph *node) override {
    if (node->children().empty()) return;
    for (auto it : node->children()) {
      it->accept(this);
    }
    m_md += "\n";
  }
  void visit(CheckboxList *node) override {
    for (auto child : node->children()) {
      ASSERT(child->type() == NodeType::checkbox_item);
      auto item = (CheckboxItem *)child;
      m_md += "- [";
      if (item->isChecked()) {
        m_md += "x";
      } else {
        m_md += " ";
      }
      m_md += "] ";
      item->accept(this);
      m_md += "\n";
    }
  }
  void visit(CheckboxItem *node) override {
    for (auto it : node->children()) {
      it->accept(this);
    }
  }
  void visit(UnorderedList *node) override {
    for (auto it : node->children()) {
      m_md += "- ";
      it->accept(this);
      m_md += "\n";
    }
  }
  void visit(OrderedList *node) override {
    int i = 1;
    for (auto it : node->children()) {
      m_md += QString("%1. ").arg(i);
      it->accept(this);
      m_md += "\n";
      i++;
    }
  }
  void visit(OrderedListItem *node) override {
    for (auto child : node->children()) {
      child->accept(this);
    }
  }
  void visit(UnorderedListItem *node) override {
    for (auto child : node->children()) {
      child->accept(this);
    }
  }
  void visit(Hr *node) override { m_md += "---\n"; }
  void visit(Lf *node) override { m_md += "\n"; }
  void visit(QuoteBlock *node) override {
    m_md += ">";
    for (auto it : node->children()) {
      it->accept(this);
      m_md += "\n";
    }
  }
  void visit(Table *node) override {}
  String markdown() { return m_md; }

 private:
  String m_md;
  DocPtr m_doc;
};
class MousePressVisitor : public MultipleVisitor<Link, CheckboxItem, Image, CodeBlock> {
 public:
  MousePressVisitor(Document &doc, Editor &editor, SizeType blockNo)
      : m_handled(false), m_doc(doc), m_editor(editor), m_blockNo(blockNo) {}
  void visit(Link *node) override {
    if (m_editor.m_holdCtrl) {
      auto url = node->href()->toString(&m_doc);
      m_editor.m_holdCtrl = false;
      m_editor.m_linkClickedCallback(url);
      m_handled = true;
    }
  }
  void visit(CheckboxItem *node) override {
    node->setChecked(!node->isChecked());
    m_doc.renderBlock(m_blockNo);
    m_handled = true;
  }
  void visit(Image *node) override {
    auto path = node->src()->toString(&m_doc);
    m_editor.m_imageClickedCallback(path);
    m_handled = true;
  }
  void visit(CodeBlock *node) override {
    String code;
    for (const auto &child : node->children()) {
      if (child->type() == NodeType::text) {
        auto text = (Text *)child;
        code += text->toString(&m_doc);
        code += "\n";
      } else if (child->type() == NodeType::lf) {
        code += "\n";
      } else {
        DEBUG << node->type();
        ASSERT(false && "not support");
      }
    }
    m_editor.m_copyCodeBtnClickedCallback(code);
    m_handled = true;
  }
  [[nodiscard]] bool handled() const { return m_handled; }

 private:
  bool m_handled;
  Document &m_doc;
  Editor &m_editor;
  SizeType m_blockNo;
};
Editor::Editor() {
  m_cursor = std::make_shared<Cursor>();
  m_renderSetting = std::make_shared<render::RenderSetting>();
  m_linkClickedCallback = [](String s) { DEBUG << "click link" << s; };
  m_imageClickedCallback = [](String s) { DEBUG << "click link" << s; };
  m_copyCodeBtnClickedCallback = [](String s) { DEBUG << "click link" << s; };
}
void Editor::loadText(const String &text) {
  m_doc = std::make_shared<Document>(text, m_renderSetting);
  m_cursor = std::make_shared<Cursor>();
  m_doc->updateCursor(*m_cursor, m_cursor->coord());
  DEBUG << "load text done";
}
void Editor::loadFile(const String &path) {
  DEBUG << path;
  String notePath = path;
  String prefix = "file://";
  if (path.startsWith(prefix)) {
    notePath = path.mid(prefix.size());
  }
  QFile file(notePath);
  if (!file.exists()) {
    DEBUG << "file not exist:" << notePath;
    return;
  }
  if (!file.open(QIODevice::ReadOnly)) {
    DEBUG << "file open fail:" << notePath;
    return;
  }
  auto mdText = file.readAll();
  loadText(mdText);
}

bool Editor::saveToFile(const String &path) {
  String notePath = path;
  String prefix = "file://";
  if (path.startsWith(prefix)) {
    notePath = path.mid(prefix.size());
  }
  if (!notePath.endsWith(".md")) {
    notePath += ".md";
  }
  DEBUG << "note path" << notePath;
  QFile file(notePath);
  if (!file.open(QIODevice::WriteOnly)) {
    DEBUG << "file open fail:" << notePath;
    return false;
  }
  SimpleMarkdownVisitor visitor(m_doc.get());
  m_doc->accept(&visitor);
  auto mdText = visitor.markdown();
  file.write(mdText.toLocal8Bit());
  file.close();
  return true;
}
void Editor::drawDoc(QPoint offset, Painter &painter) {
  if (!m_doc) return;
  // 如果最后一个block不是段落，添加一个段落
  if (m_doc->m_root->children().back()->type() != NodeType::paragraph) {
    auto newParagraph = new Paragraph();
    m_doc->m_root->appendChild(newParagraph);
    m_doc->m_blocks.push_back(render::Render::render(newParagraph, m_renderSetting, m_doc.get()));
  }
  auto oldOffset = offset;
  offset.setY(offset.y() + m_renderSetting->docMargin.top());
  for (const auto &block : m_doc->m_blocks) {
    auto h = block.height();
    // 把每个指令都画出来
    for (const auto &instruction : block) {
      instruction->run(painter, offset, m_doc.get());
    }
    offset.setY(offset.y() + h);
  }
  auto pos = m_cursor->pos();
  auto coord = m_cursor->coord();
  // 高亮当前Block
  int h = m_renderSetting->docMargin.top();
  for (int i = 0; i < coord.blockNo; ++i) {
    h += m_doc->m_blocks[i].height();
  }
  auto block = m_doc->m_blocks[coord.blockNo];
  painter.save();
  painter.setPen(QColor(0, 255, 255));
  auto highlightPos = Point(m_renderSetting->docMargin.left(), h);
  auto size = Size(block.width(), block.height() - m_renderSetting->lineSpacing);
  painter.drawRect(Rect(highlightPos + oldOffset, size));
  painter.restore();
  auto node = m_doc->m_root->children()[coord.blockNo];
  auto typePos = Point(0, h) + oldOffset;
  if (node->type() == NodeType::paragraph) {
    painter.drawText(typePos, "P");
  } else if (node->type() == NodeType::header) {
    painter.drawText(typePos, "H");
  } else if (node->type() == NodeType::ol) {
    painter.drawText(typePos, "ol");
  } else if (node->type() == NodeType::ul) {
    painter.drawText(typePos, "ul");
  } else if (node->type() == NodeType::checkbox) {
    painter.drawText(typePos, "cb");
  }
}
int Editor::width() const { return m_renderSetting->maxWidth; }
int Editor::height() const {
  auto h = 0;
  for (const auto &instructionGroup : m_doc->m_blocks) {
    h += instructionGroup.height();
  }
  return h + m_renderSetting->docMargin.top() + m_renderSetting->docMargin.bottom();
}
void Editor::drawCursor(QPoint offset, Painter &painter) {
  if (m_showCursor) {
    auto pos = m_cursor->pos();
    pos += offset;
    painter.save();
    painter.setPen(Qt::red);
    auto x = pos.x();
    auto y = pos.y();
    auto h = m_cursor->height();
    auto delta = 2;
    painter.drawLine(x - delta, y, x + delta, y);
    painter.drawLine(x, y, x, y + h);
    painter.drawLine(x - delta, y + h, x + delta, y + h);
    painter.restore();
  }
  m_showCursor = !m_showCursor;
}
void Editor::keyPressEvent(KeyEvent *event) {
  if (event->key() == Qt::Key_Tab) {
    return;
  }
  if (event->modifiers() & Qt::Modifier::CTRL) {
    m_holdCtrl = true;
  }
  if (event->key() == Qt::Key_Left) {
    if (event->modifiers() & Qt::Modifier::CTRL) {
      m_doc->moveCursorToBol(*m_cursor);
    } else {
      m_doc->moveCursorToLeft(*m_cursor);
    }
  } else if (event->key() == Qt::Key_Right) {
    if (event->modifiers() & Qt::Modifier::CTRL) {
      m_doc->moveCursorToEol(*m_cursor);
    } else {
      m_doc->moveCursorToRight(*m_cursor);
    }
  } else if (event->key() == Qt::Key_Up) {
    m_doc->moveCursorToUp(*m_cursor);
  } else if (event->key() == Qt::Key_Down) {
    m_doc->moveCursorToDown(*m_cursor);
  } else if (event->modifiers() & Qt::Modifier::CTRL) {
  } else if (event->key() == Qt::Key_Backspace) {
    m_doc->removeText(*m_cursor);
  } else if (event->key() == Qt::Key_Return) {
    // 处理回车，要拆结点
    m_doc->insertReturn(*m_cursor);
  } else {
    auto text = event->text();
    m_doc->insertText(*m_cursor, text);
  }
}
Point Editor::cursorPos() const { return m_cursor->pos(); }
void Editor::mousePressEvent(Point offset, MouseEvent *event) {
  auto oldOffset = offset;
  offset.setY(offset.y() + m_renderSetting->docMargin.top());
  for (int blockNo = 0; blockNo < m_doc->m_blocks.size(); ++blockNo) {
    const auto &block = m_doc->m_blocks[blockNo];
    auto h = block.height();
    for (const auto &element : block.elementList()) {
      if (Rect(element.pos + offset, element.size).contains(event->pos())) {
        MousePressVisitor visitor(*m_doc, *this, blockNo);
        element.node->accept(&visitor);
        if (visitor.handled()) {
          return;
        }
      }
    }
    offset.setY(offset.y() + h);
  }
  m_doc->moveCursorToPos(*m_cursor, event->pos());
}
void Editor::insertText(String str) { m_doc->insertText(*m_cursor, str); }
void Editor::reset() {
  m_cursor = std::make_shared<Cursor>();
  m_doc = std::make_shared<Document>("", m_renderSetting);
}
String Editor::cursorCoord() const {
  String s;
  auto pos = m_cursor->pos();
  auto coord = m_cursor->coord();
  s += QString("Cursor: (%1, %2, %3)").arg(pos.x()).arg(pos.y()).arg(m_cursor->height());
  s += "\n";
  s += QString("BlockNo: %1/%2").arg(coord.blockNo).arg(m_doc->m_blocks.size());
  s += "\n";
  auto &block = m_doc->m_blocks[coord.blockNo];
  s += QString("LineNo: %1/%2").arg(coord.lineNo).arg(block.countOfLogicalLine());
  s += "\n";
  //  s += QString("CellNo: %1/%2").arg(coord.cellNo).arg(block.countOfLogicalItem(coord.lineNo));
  //  s += "\n";
  s += QString("Offset: %1/%2").arg(coord.offset).arg(block.logicalLineAt(coord.lineNo).length());
  s += "\n";
  s += QString("Hold Ctrl: ");
  if (m_holdCtrl)
    s += "YES";
  else
    s += "NO";
  s += "\n";
  return s;
}
CursorShape Editor::cursorShape(Point offset, Point pos) {
  auto oldOffset = offset;
  offset.setY(offset.y() + m_renderSetting->docMargin.top());
  for (const auto &block : m_doc->m_blocks) {
    auto h = block.height();
    for (const auto &element : block.elementList()) {
      if (Rect(element.pos + offset, element.size).contains(pos)) {
        return PointingHandCursor;
      }
    }
    offset.setY(offset.y() + h);
  }
  return IBeamCursor;
}
void Editor::keyReleaseEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Control) {
    m_holdCtrl = false;
  }
}
}  // namespace md::editor
