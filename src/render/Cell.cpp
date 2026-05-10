//
// Created by PikachuHy on 2021/11/17.
//

#include "Cell.h"

#include "FontMetricsProvider.h"
#include "debug.h"
namespace md::render {
SizeType TextCell::length() { return m_length; }
int TextCell::ascent() const { return m_fm->ascent(m_font); }
int TextCell::width(SizeType length, const parser::IBufferProvider& doc) const {
  ASSERT(length >= 0 && length <= m_length);
  auto s = m_text->toString(doc).mid(m_offset, m_length);
  auto w = m_fm->horizontalAdvance(m_font, s.left(length));
  return w;
}
int InlineLatexCell::width(SizeType length, const parser::IBufferProvider& /*doc*/) const {
  if (length == 0) return 0;
  return m_size.width;
}
}  // namespace md::render
