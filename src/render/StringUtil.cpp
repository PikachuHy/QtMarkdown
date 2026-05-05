//
// Created by PikachuHy on 2021/11/14.
//

#include "StringUtil.h"
#include "../core/Utf8Util.h"
namespace md::render {
std::vector<RenderString> StringUtil::split(const String& text) {
  std::vector<RenderString> stringList;
  SizeType i = 0;
  SizeType offset = i;
  auto isAlphaNum = [](char ch) -> bool { return static_cast<unsigned char>(ch) < 128; };
  while (i < text.size()) {
    // 英文判定
    if (isAlphaNum(text[i]) || text[i] == ' ') {
      if (i > offset) {
        stringList.emplace_back(RenderString::Chinese, offset, i - offset);
        offset = i;
      }
      while (i < text.size() && (isAlphaNum(text[i]) || text[i] == ' ')) {
        i++;
      }
      stringList.emplace_back(RenderString::English, offset, i - offset);
      offset = i;
      continue;
    }
    // emoji判定 — check for 4-byte UTF-8 sequences (lead byte 0xF0)
    if (utf8SequenceLength(text[i]) == 4) {
      if (i > offset) {
        stringList.emplace_back(RenderString::Chinese, offset, i - offset);
        offset = i;
      }
      stringList.emplace_back(RenderString::Emoji, i, 4);
      i += 4;
      offset = i;
      continue;
    }
    // variation selector (U+FE0F) and ZWJ (U+200D) — treat as part of previous run
    if (i < text.size()) {
      auto cp = codePointAt(text.toStdString(), i);
      if (cp == 0xfe0f || cp == 0x200d) {
        i += utf8SequenceLength(text[i]);
        continue;
      }
    }
    // 余下的是中文
    i++;
  }
  if (i > offset) {
    stringList.emplace_back(RenderString::Chinese, offset, i - offset);
  }
  return stringList;
}
}  // namespace md::render