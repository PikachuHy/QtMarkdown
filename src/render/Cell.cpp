//
// Created by PikachuHy on 2021/11/17.
//

#include "Cell.h"

#include <QFontMetrics>

#include "debug.h"
namespace md::render {
SizeType TextCell::length() { return m_length; }
int TextCell::width(SizeType length, DocPtr doc) const {
  ASSERT(length >= 0 && length <= m_length);
  auto s = m_text->toString(doc).mid(m_offset, m_length);
  QFontMetrics fm(m_font);
  auto w = fm.horizontalAdvance(s.left(length));
  return w;
}
SizeType StaticTextCell::length() { return m_text.length(); }
int StaticTextCell::width(SizeType length, DocPtr doc) const {
  ASSERT(length >= 0 && length <= m_text.size());
  QFontMetrics fm(m_font);
  auto w = fm.horizontalAdvance(m_text.left(length));
  return w;
}
SizeType ImageCell::length() { return 1; }
int ImageCell::width(SizeType length, DocPtr doc) const {
  if (length == 0) return 0;
  return m_size.width();
}
}  // namespace md::render