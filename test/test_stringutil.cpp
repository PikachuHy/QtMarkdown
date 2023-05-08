//
// Created by PikachuHy on 2021/11/14.
//
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "render/StringUtil.h"
using namespace md;
using namespace md::render;
TEST_CASE("testing split Emoji") {
  md::String s = "我非常喜欢😍用";
  auto stringList = StringUtil::split(s);
  CHECK(stringList.size() == 3);
  CHECK(stringList[0].type == RenderString::Chinese);
  CHECK(stringList[1].type == RenderString::Emoji);
  CHECK(stringList[2].type == RenderString::Chinese);
}