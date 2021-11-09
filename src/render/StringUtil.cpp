//
// Created by PikachuHy on 2021/11/14.
//

#include "StringUtil.h"
namespace md::render {
std::vector<RenderString> StringUtil::split(const String& text) {
  std::vector<RenderString> stringList;
  SizeType i = 0;
  SizeType offset = i;
  auto isAlphaNum = [](QChar ch) -> bool { return ch.unicode() < 128; };
  while (i < text.size()) {
    // 英文判定
    if (isAlphaNum(text[i]) || text[i].isSpace()) {
      if (i > offset) {
        stringList.emplace_back(RenderString::Chinese, offset, i - offset);
        offset = i;
      }
      while (i < text.size() && (isAlphaNum(text[i]) || text[i].isSpace())) {
        i++;
      }
      stringList.emplace_back(RenderString::English, offset, i - offset);
      offset = i;
      continue;
    }
    // emoji判定
    if (i + 1 < text.size()) {
      auto ch1 = text[i].unicode();
      auto ch2 = text[i + 1].unicode();
      // emoji还有更多可能，暂时只考虑这些
      if (ch1 == 0xd83d && ch2 >= 0xdc00 && ch2 <= 0xde4f) {
        if (i > offset) {
          stringList.emplace_back(RenderString::Chinese, offset, i - offset);
          offset = i;
        }
        stringList.emplace_back(RenderString::Emoji, i, 2);
        i += 2;
        offset = i;
        continue;
      }
      if (ch1 == 0xd83c && ch2 >= 0xdf00 && ch2 <= 0xdfff) {
        if (i > offset) {
          stringList.emplace_back(RenderString::Chinese, offset, i - offset);
          offset = i;
        }
        stringList.emplace_back(RenderString::Emoji, i, 2);
        i += 2;
        offset = i;
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