//
// Created by pikachu on 2021/10/28.
//

#ifndef QTMARKDOWN_EDITORDOCUMENT_H
#define QTMARKDOWN_EDITORDOCUMENT_H
#include <QFont>
#include <QRect>
#include <QString>
namespace Element {
struct Link {
  QString text;
  QString url;
  QList<QRect> rects;
};
struct Image {
  QString path;
  QRect rect;
};
struct CodeBlock {
  QString code;
  QRect rect;
};
} // namespace Element
class Cursor;
class Document;
class Render;
struct Cell {
  QFont font;
  QRect rect;
  QString text;
};
class LineData {
public:
  void appendCell(const Cell &cell);
  bool contains(Cursor &cursor);
  QList<Cell> &cells() { return m_cells; }
  std::pair<qsizetype, qsizetype> lastCoord();

private:
  QList<Cell> m_cells;
};
class EditorDocument {
public:
  explicit EditorDocument(QString text);
  void createNewLineData();
  void updateCursor(Cursor *cursor);
  void moveCursorLeft(Cursor *cursor);
  void moveCursorRight(Cursor *cursor);
  void moveCursorDown(Cursor *cursor);
  void moveCursorUp(Cursor *cursor);
  void fixCursorPos(Cursor *cursor);
  void draw(Render *render);
  void appendCell(const Cell& cell);
  void appendImage(Element::Image *image) { m_images.append(image); }
  void appendLink(Element::Link *link) { m_links.append(link); }
  void appendCodeBlock(Element::CodeBlock *code) { m_codes.append(code); }

  const QList<Element::Link *> &links() const { return m_links; };
  const QList<Element::Image *> &images() const { return m_images; };
  const QList<Element::CodeBlock *> &codes() const { return m_codes; };

  QList<LineData *>& lineData() { return m_lineData; }
private:
  QList<LineData *> m_lineData;
  Document* m_doc;
  QList<Element::Link *> m_links;
  QList<Element::Image *> m_images;
  QList<Element::CodeBlock *> m_codes;
};

#endif // QTMARKDOWN_EDITORDOCUMENT_H
