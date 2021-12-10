//
// Created by PikachuHy on 2021/11/5.
//

#include "Editor.h"

#include <QFile>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QStringList>
#include <memory>
#include <vector>

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
    m_editor.m_checkBoxClickedCallback();
    m_handled = true;
  }
  void visit(Image *node) override {
    auto path = node->src()->toString(&m_doc);
#if defined (Q_OS_ANDROID) || defined (Q_OS_UNIX)
    if (!path.startsWith("/")) {
      for (const auto& resPath: m_doc.m_setting->resPathList) {
        QString newImgPath = resPath + "/" + path;
        if (QFile(newImgPath).exists()) {
          path = newImgPath;
        }
      }
    }
#endif
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
#ifdef Q_OS_ANDROID
  m_renderSetting->docMargin.setLeft(0);
#endif
  m_linkClickedCallback = [](String s) { DEBUG << "click link" << s; };
  m_imageClickedCallback = [](String s) { DEBUG << "click image" << s; };
  m_copyCodeBtnClickedCallback = [](String s) { DEBUG << "click copy code btn" << s; };
  m_checkBoxClickedCallback = []() { DEBUG << "click check box"; };
}
void Editor::loadText(const String &text) {
  m_doc = std::make_shared<Document>(text, m_renderSetting);
  m_cursor = std::make_shared<Cursor>();
  m_doc->updateCursor(*m_cursor, m_cursor->coord());
  DEBUG << "load text done";
}
std::pair<bool, String> Editor::loadFile(const String &path) {
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
  loadText(mdText);
  return {true, this->title()};
}

bool Editor::saveToFile(const String &path) {
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
  SimpleMarkdownVisitor visitor(m_doc.get());
  m_doc->accept(&visitor);
  auto mdText = visitor.markdown();
  file.write(mdText.toUtf8());
  file.close();
  return true;
}
void Editor::drawSelection(Point offset, Painter &painter) {
  if (!m_hasSelection) return;
  DEBUG << "selection" << m_selectionInstructions.size();
  for (auto instruction : m_selectionInstructions) {
    instruction->run(painter, offset, m_doc.get());
  }
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
  for (int blockNo = 0; blockNo < m_doc->m_blocks.size(); ++blockNo) {
    const auto &block = m_doc->m_blocks[blockNo];
    auto h = block.height();
    // 把每个指令都画出来
    for (const auto &instruction : block) {
      instruction->run(painter, offset, m_doc.get());
    }
    offset.setY(offset.y() + h);
  }
#ifdef Q_OS_ANDROID
#else
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
#endif
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
  if (m_hasSelection) return;
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
void Editor::keyPressEvent(KeyEvent *event) {
  int key = event->key();
  if (key == Qt::Key_Tab) {
    return;
  }
  if (key == Qt::Key_Escape) {
    return;
  }
  if (event->modifiers() & Qt::Modifier::CTRL) {
    m_holdCtrl = true;
  }
  if (event->modifiers() & Qt::Modifier::SHIFT) {
    m_holdShift = true;
  }
  if (key == Qt::Key_A && m_holdCtrl) {
    this->selectAll();
    return;
  }
  if (key == Qt::Key_Z && m_holdCtrl) {
    DEBUG << "undo";
    this->m_doc->undo(*m_cursor);
    return;
  }
  if (m_holdCtrl) {
    if (key == Qt::Key_1) {
      m_doc->upgradeToHeader(*m_cursor, 1);
      m_doc->updateCursor(*m_cursor, m_cursor->coord());
      return;
    }
    if (key == Qt::Key_2) {
      m_doc->upgradeToHeader(*m_cursor, 2);
      m_doc->updateCursor(*m_cursor, m_cursor->coord());
      return;
    }
    if (key == Qt::Key_3) {
      m_doc->upgradeToHeader(*m_cursor, 3);
      m_doc->updateCursor(*m_cursor, m_cursor->coord());
      return;
    }
    if (key == Qt::Key_4) {
      m_doc->upgradeToHeader(*m_cursor, 4);
      m_doc->updateCursor(*m_cursor, m_cursor->coord());
      return;
    }
    if (key == Qt::Key_5) {
      m_doc->upgradeToHeader(*m_cursor, 5);
      m_doc->updateCursor(*m_cursor, m_cursor->coord());
      return;
    }
    if (key == Qt::Key_6) {
      m_doc->upgradeToHeader(*m_cursor, 6);
      m_doc->updateCursor(*m_cursor, m_cursor->coord());
      return;
    }
  }
  if (key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_Up || key == Qt::Key_Down) {
    if (m_hasSelection && !m_holdShift) {
      auto coord = m_selectionRange->caret.coord();
      m_doc->updateCursor(*m_cursor, coord);
      m_hasSelection = false;
      return;
    }
    if (key == Qt::Key_Left) {
      if (m_holdShift) {
        if (m_holdCtrl) {
          selectBol();
        } else {
          selectLeft();
        }
      } else {
        if (m_holdCtrl) {
          auto coord = m_doc->moveCursorToBol(m_cursor->coord());
          m_doc->updateCursor(*m_cursor, coord, true);
        } else {
          auto coord = m_doc->moveCursorToLeft(m_cursor->coord());
          m_doc->updateCursor(*m_cursor, coord);
        }
      }
      return;
    }
    if (key == Qt::Key_Right) {
      if (m_holdShift) {
        if (m_holdCtrl) {
          selectEol();
        } else {
          selectRight();
        }
      } else {
        if (m_holdCtrl) {
          auto [coord, x] = m_doc->moveCursorToEol(m_cursor->coord());
          m_cursor->setX(x);
          m_doc->updateCursor(*m_cursor, coord, false);
        } else {
          auto coord = m_doc->moveCursorToRight(m_cursor->coord());
          m_doc->updateCursor(*m_cursor, coord);
        }
      }
      return;
    }
    if (key == Qt::Key_Up) {
      if (m_holdShift) {
        selectUp();
      } else {
        CursorCoord coord;
        if (m_hasSelection) {
          coord = m_selectionRange->caret.coord();
          m_hasSelection = false;
        } else {
          coord = m_doc->moveCursorToUp(m_cursor->coord(), m_cursor->pos());
        }
        m_doc->updateCursor(*m_cursor, coord);
      }
      return;
    }
    if (key == Qt::Key_Down) {
      if (m_holdShift) {
        selectDown();
      } else {
        CursorCoord coord;
        if (m_hasSelection) {
          coord = m_selectionRange->caret.coord();
          m_hasSelection = false;
        } else {
          coord = m_doc->moveCursorToDown(m_cursor->coord(), m_cursor->pos());
        }
        m_doc->updateCursor(*m_cursor, coord);
      }
      return;
    }
  }
  if (key == Qt::Key_Backspace) {
    if (m_hasSelection) {
      removeSelection();
    } else {
      m_doc->removeText(*m_cursor);
    }
  } else if (key == Qt::Key_Return) {
    if (m_hasSelection) {
      removeSelection();
    }
    // 处理回车，要拆结点
    m_doc->insertReturn(*m_cursor);
  } else {
    if (m_hasSelection) {
      removeSelection();
    }
    auto text = event->text();
    m_doc->insertText(*m_cursor, text);
  }
}
Point Editor::cursorPos() const { return m_preediting ? m_preeditPos : m_cursor->pos(); }
Rect Editor::cursorRect() const {
  auto pos = cursorPos();
  auto h = m_cursor->height();
  return Rect(pos, Size(5, h));
}
void Editor::mousePressEvent(Point offset, MouseEvent *event) {
#ifdef Q_OS_ANDROID
  // 安卓有press，但是没有release
#else
  m_mousePressing = true;
#endif

  DEBUG << "shift" << m_holdShift << m_hasSelection;
  if (m_holdShift) {
    if (!m_hasSelection) {
      m_selectionRange = std::make_shared<SelectionRange>();
      m_selectionRange->anchor = *m_cursor;
      m_hasSelection = true;
    }
  } else {
    m_hasSelection = false;
  }
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
  auto coord = m_doc->moveCursorToPos(event->pos());
  m_doc->updateCursor(*m_cursor, coord);
  if (m_hasSelection) {
    m_doc->updateCursor(m_selectionRange->caret, coord);
    generateSelectionInstruction();
  }
}
void Editor::insertText(String str) {
  if (str.isEmpty()) return;
  auto strs = str.split("\n");
  for (int i = 0; i < strs.size() - 1; ++i) {
    m_doc->insertText(*m_cursor, strs[i]);
    m_doc->insertReturn(*m_cursor);
  }
  m_doc->insertText(*m_cursor, strs.back());
}
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
  s += QString("Offset: %1/%2").arg(coord.offset).arg(block.logicalLineAt(coord.lineNo).length());
  s += "\n";
  s += QString("Hold Ctrl: ");
  if (m_holdCtrl)
    s += "YES";
  else
    s += "NO";
  s += "\n";
  s += QString("Hold Shift: ");
  if (m_holdShift)
    s += "YES";
  else
    s += "NO";
  s += "\n";
  s += QString("Pre Editing: ");
  if (m_preediting)
    s += "YES";
  else
    s += "NO";
  s += "\n";
  if (m_hasSelection) {
    s += QString("Selection Range: ");
    auto [begin, end] = m_selectionRange->range();
    s += QString("(%1,%2,%3)").arg(begin.coord().blockNo).arg(begin.coord().lineNo).arg(begin.coord().offset);
    s += QString("->");
    s += QString("(%1,%2,%3)").arg(end.coord().blockNo).arg(end.coord().lineNo).arg(end.coord().offset);
  }
            return s;
  }

void Editor::setWidth(int w) {
        m_renderSetting->maxWidth = w;
}
void Editor::setResPathList(StringList pathList) {
    m_renderSetting->resPathList = pathList;
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
  if (event->key() == Qt::Key_Shift) {
    m_holdShift = false;
  }
}
void Editor::setPreedit(String str) {
  if (m_preediting) {
    for (int i = 0; i < m_preeditLength; ++i) {
      m_doc->removeText(*m_cursor);
    }
  } else {
    m_preeditPos = m_cursor->pos();
  }
  if (str.isEmpty()) {
    m_preediting = false;
    m_preeditLength = 0;
  } else {
    m_preediting = true;
    m_preeditLength = str.length();
    m_doc->insertText(*m_cursor, str);
  }
}
void Editor::commitString(String str) {
  if (m_preediting) {
    for (int i = 0; i < m_preeditLength; ++i) {
      m_doc->removeText(*m_cursor);
    }
  }
  m_doc->insertText(*m_cursor, str);
  m_preediting = false;
  m_preeditLength = 0;
}
String Editor::title() {
  if (m_doc->m_root->empty()) return "";
  auto node = m_doc->m_root->childAt(0);
  if (node->type() != NodeType::header) return "";
  auto header = (Header *)node;
  if (header->level() != 1) return "";
  String s;
  for (auto child : header->children()) {
    if (child->type() != NodeType::text) continue;
    auto textNode = (Text *)child;
    s += textNode->toString(m_doc.get());
  }
  return s;
}
void Editor::selectLeft() {
  CursorCoord coord;
  if (m_hasSelection) {
    coord = m_selectionRange->caret.coord();
  } else {
    m_selectionRange = std::make_shared<SelectionRange>();
    coord = m_cursor->coord();
    m_selectionRange->anchor = *m_cursor;
    m_hasSelection = true;
  }
  coord = m_doc->moveCursorToLeft(coord);
  m_doc->updateCursor(m_selectionRange->caret, coord);
  generateSelectionInstruction();
}
void Editor::selectDown() {
  CursorCoord coord;
  Point pos;
  if (m_hasSelection) {
    coord = m_selectionRange->caret.coord();
    pos = m_selectionRange->caret.pos();
  } else {
    m_selectionRange = std::make_shared<SelectionRange>();
    coord = m_cursor->coord();
    pos = m_cursor->pos();
    m_selectionRange->anchor = *m_cursor;
    m_hasSelection = true;
  }
  // 如果是视觉行开头，则移动到视觉行结尾
  if (m_doc->isBol(coord)) {
    auto [_coord, x] = m_doc->moveCursorToEol(coord);
    m_selectionRange->caret.setX(x);
    m_doc->updateCursor(m_selectionRange->caret, _coord, false);
  } else {
    coord = m_doc->moveCursorToDown(coord, pos);
    m_doc->updateCursor(m_selectionRange->caret, coord);
  }
  generateSelectionInstruction();
}
void Editor::selectUp() {
  CursorCoord coord;
  Point pos;
  if (m_hasSelection) {
    coord = m_selectionRange->caret.coord();
    pos = m_selectionRange->caret.pos();
  } else {
    m_selectionRange = std::make_shared<SelectionRange>();
    coord = m_cursor->coord();
    pos = m_cursor->pos();
    m_selectionRange->anchor = *m_cursor;
    m_hasSelection = true;
  }
  coord = m_doc->moveCursorToUp(coord, pos);
  m_doc->updateCursor(m_selectionRange->caret, coord);
  generateSelectionInstruction();
}
void Editor::selectRight() {
  CursorCoord coord;
  if (m_hasSelection) {
    coord = m_selectionRange->caret.coord();
  } else {
    m_selectionRange = std::make_shared<SelectionRange>();
    coord = m_cursor->coord();
    m_selectionRange->anchor = *m_cursor;
    m_hasSelection = true;
  }
  coord = m_doc->moveCursorToRight(coord);
  m_doc->updateCursor(m_selectionRange->caret, coord);
  generateSelectionInstruction();
}

void Editor::selectAll() {
  CursorCoord coord;
  m_hasSelection = true;
  m_selectionRange = std::make_shared<SelectionRange>();
  coord = m_doc->moveCursorToBeginOfDocument();
  m_doc->updateCursor(m_selectionRange->anchor, coord);
  coord = m_doc->moveCursorToEndOfDocument();
  m_doc->updateCursor(m_selectionRange->caret, coord);
  generateSelectionInstruction();
}
void Editor::selectBol() {
  CursorCoord coord;
  if (m_hasSelection) {
    coord = m_selectionRange->caret.coord();
  } else {
    m_selectionRange = std::make_shared<SelectionRange>();
    coord = m_cursor->coord();
    m_selectionRange->anchor = *m_cursor;
    m_hasSelection = true;
  }
  coord = m_doc->moveCursorToBol(coord);
  m_doc->updateCursor(m_selectionRange->caret, coord);
  generateSelectionInstruction();
}
void Editor::selectEol() {
  CursorCoord coord;
  if (m_hasSelection) {
    coord = m_selectionRange->caret.coord();
  } else {
    m_selectionRange = std::make_shared<SelectionRange>();
    coord = m_cursor->coord();
    m_selectionRange->anchor = *m_cursor;
    m_hasSelection = true;
  }
  auto [_coord, x] = m_doc->moveCursorToEol(coord);
  m_selectionRange->caret.setX(x);
  m_doc->updateCursor(m_selectionRange->caret, _coord, false);
  generateSelectionInstruction();
}
void Editor::generateSelectionInstruction() {
  m_selectionInstructions.clear();
  auto fillRect = [this](Point pos, int w, int h) {
    QColor bg(187, 214, 251);
    auto instruction = new render::FillRectInstruction(pos, Size(w, h), bg);
    m_selectionInstructions.push_back(instruction);
  };
  auto isBefore = [this](const CursorCoord &coord, SizeType blockNo, SizeType lineNo, SizeType visualLineNo) {
    if (blockNo < coord.blockNo) return true;
    if (blockNo > coord.blockNo) return false;
    if (lineNo < coord.lineNo) return true;
    if (lineNo > coord.lineNo) return false;
    const auto &line = this->m_doc->m_blocks[blockNo].logicalLineAt(lineNo);
    auto coordVisualLineNo = line.visualLineAt(coord.offset, this->m_doc.get());
    if (coordVisualLineNo > visualLineNo) return true;
    return false;
  };
  auto isSameVisualLine = [this](const CursorCoord &coord, SizeType blockNo, SizeType lineNo, SizeType visualLineNo) {
    if (blockNo < coord.blockNo) return false;
    if (blockNo > coord.blockNo) return false;
    if (lineNo < coord.lineNo) return false;
    if (lineNo > coord.lineNo) return false;
    const auto &line = this->m_doc->m_blocks[blockNo].logicalLineAt(lineNo);
    auto coordVisualLineNo = line.visualLineAt(coord.offset, this->m_doc.get());
    if (coordVisualLineNo < visualLineNo) return false;
    if (coordVisualLineNo > visualLineNo) return false;
    return true;
  };
  // 直接解构begin, end
  // lambda捕获不到
  auto selectionRange = m_selectionRange->range();
  auto [begin, end] = selectionRange;
  auto totalH = m_renderSetting->docMargin.top();
  for (int i = 0; i < begin.coord().blockNo; ++i) {
    const auto &block = m_doc->m_blocks[i];
    totalH += block.height();
  }
  bool drawDone = false;
  for (auto blockNo = begin.coord().blockNo; blockNo <= end.coord().blockNo; ++blockNo) {
    const auto &block = m_doc->m_blocks[blockNo];
    auto blockOffset = Point(0, totalH);
    for (int lineNo = 0; lineNo < block.countOfLogicalLine(); ++lineNo) {
      const auto &line = block.logicalLineAt(lineNo);
      for (int visualLineNo = 0; visualLineNo < line.countOfVisualLine(); ++visualLineNo) {
        const auto &visualLine = line.visualLineAt(visualLineNo);
        // 如果该视觉行在begin之前，直接跳过
        if (isBefore(begin.coord(), blockNo, lineNo, visualLineNo)) {
          continue;
        }
        auto fixW = [&visualLine, &selectionRange](int &w) {
          if (w == 0) {
            if (selectionRange.first.coord() == selectionRange.second.coord()) {
              // 不用管的确是0
            } else {
              // 整行都要画，因为end被调到另外一行到开头
              w = visualLine.width();
            }
          }
        };
        auto h = visualLine.height();
        if (isSameVisualLine(begin.coord(), blockNo, lineNo, visualLineNo)) {
          auto pos = Point(begin.x(), 0) + Point(0, visualLine.pos().y()) + blockOffset;
          if (isSameVisualLine(end.coord(), blockNo, lineNo, visualLineNo)) {
            // 同一个视觉行，从begin画到end
            auto w = end.pos().x() - begin.pos().x();
            fixW(w);
            fillRect(pos, w, h);
            drawDone = true;
            break;
          } else {
            // 从begin画到视觉行结束
            auto w = visualLine.width() + visualLine.pos().x() - begin.pos().x();
            fixW(w);
            fillRect(pos, w, h);
            continue;
          }
        }
        if (isSameVisualLine(end.coord(), blockNo, lineNo, visualLineNo)) {
          auto pos = visualLine.pos() + blockOffset;
          auto w = end.pos().x() - visualLine.pos().x();
          fixW(w);
          fillRect(pos, w, h);
          drawDone = true;
          break;
        }
        fillRect(visualLine.pos() + blockOffset, visualLine.width(), h);
      }
      if (drawDone) break;
    }
    totalH += block.height();
    if (drawDone) break;
  }
}
void Editor::mouseMoveEvent(Point offset, MouseEvent *event) {
  if (!m_mousePressing) return;
  if (!m_hasSelection) {
    m_selectionRange = std::make_shared<SelectionRange>();
    m_selectionRange->anchor = *m_cursor;
    m_hasSelection = true;
  }
  auto coord = m_doc->moveCursorToPos(event->pos() + offset);
  m_doc->updateCursor(m_selectionRange->caret, coord);
  generateSelectionInstruction();
}
void Editor::mouseReleaseEvent(Point offset, MouseEvent *event) {
    m_mousePressing = false;
}
void Editor::removeSelection() {
  if (!m_hasSelection) return;
  auto selectionRange = m_selectionRange->range();
  auto [begin, end] = selectionRange;
  while (begin.coord() != end.coord()) {
    m_doc->removeText(end);
  }
  m_hasSelection = false;
  m_doc->updateCursor(*m_cursor, begin.coord());
#if 0
  auto isBefore = [](const CursorCoord& coord, SizeType blockNo, SizeType lineNo) {
    if (blockNo < coord.blockNo) return true;
    if (blockNo > coord.blockNo) return false;
    if (lineNo < coord.lineNo) return true;
    return false;
  };
  auto isSameLine = [](const CursorCoord& coord, SizeType blockNo, SizeType lineNo) {
    if (blockNo < coord.blockNo) return false;
    if (blockNo > coord.blockNo) return false;
    return lineNo == coord.lineNo;
  };
  for (auto blockNo = begin.coord().blockNo; blockNo <= end.coord().blockNo; ++blockNo) {
    const auto &block = m_doc->m_blocks[blockNo];
    for (SizeType lineNo = 0; lineNo < block.countOfLogicalLine(); ++lineNo) {
      if (isBefore(begin.coord(), blockNo, lineNo)) continue;
      if (isSameLine(begin.coord(), blockNo, lineNo)) {
        if (isSameLine(end.coord(), blockNo, lineNo)) {
          // 将光标移动到end
          m_doc->updateCursor(*m_cursor, end.coord());
          for (SizeType offset = begin.coord().offset; offset < end.coord().offset; ++offset) {
            m_doc->removeText()
          }
        } else {

        }
      }
      if (isSameLine(end.coord(), blockNo, lineNo)) {

      }
    }
  }
#endif
}
}  // namespace md::editor
