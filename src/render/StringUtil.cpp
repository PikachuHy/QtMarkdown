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
    // emoji判定 — covers common surrogate-pair ranges
    if (i + 1 < text.size()) {
      auto ch1 = text[i].unicode();
      auto ch2 = text[i + 1].unicode();
      auto isEmojiSurrogate = [](ushort high, ushort low) -> bool {
        // Miscellaneous Symbols and Pictographs (U+1F300–U+1F5FF)
        if (high == 0xd83c && (low >= 0xdf00 && low <= 0xdfff)) return true;
        // Emoticons (U+1F600–U+1F64F)
        if (high == 0xd83d && (low >= 0xde00 && low <= 0xde4f)) return true;
        // Transport and Map Symbols (U+1F680–U+1F6FF)
        if (high == 0xd83d && (low >= 0xde80 && low <= 0xdeff)) return true;
        // Supplemental Symbols and Pictographs (U+1F900–U+1F9FF)
        if (high == 0xd83e && (low >= 0xdd00 && low <= 0xddff)) return true;
        // Dingbats (U+2700–U+27BF, some are single-BMP)
        // Regional Indicator Symbols (U+1F1E6–U+1F1FF) — flags
        if (high == 0xd83c && (low >= 0xdde6 && low <= 0xddff)) return true;
        // Miscellaneous Symbols (U+2600–U+26FF) — some emoji are here as single BMP chars
        return false;
      };
      if (isEmojiSurrogate(ch1, ch2)) {
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
    // variation selector (U+FE0F) and ZWJ (U+200D) — treat as part of previous run
    if (i < text.size() && (text[i].unicode() == 0xfe0f || text[i].unicode() == 0x200d)) {
      i++;
      continue;
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