//
// Created by PikachuHy on 2021/11/14.
//

#ifndef QTMARKDOWN_STRINGUTIL_H
#define QTMARKDOWN_STRINGUTIL_H
#include <vector>

#include "mddef.h"
namespace md::render {
class RenderString {
 public:
  enum Type { Chinese, English, Emoji };
  RenderString(RenderString::Type type, SizeType offset, SizeType length)
      : type(type), offset(offset), length(length) {}
  Type type;
  SizeType offset;
  SizeType length;
};
class StringUtil {
 public:
  static std::vector<RenderString> split(const String& text);
};
}  // namespace md::render
#endif  // QTMARKDOWN_STRINGUTIL_H
