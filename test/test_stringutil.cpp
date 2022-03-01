//
// Created by PikachuHy on 2021/11/14.
//
#include "gtest/gtest.h"
#include "render/StringUtil.h"
using namespace md;
using namespace md::render;
TEST(StringUtilTest, SplitWithEmoji) {
  md::String s = "我非常喜欢😍用";
  auto stringList = StringUtil::split(s);
  ASSERT_EQ(stringList.size(), 3);
  ASSERT_EQ(stringList[0].type, RenderString::Chinese);
  ASSERT_EQ(stringList[1].type, RenderString::Emoji);
  ASSERT_EQ(stringList[2].type, RenderString::Chinese);
}