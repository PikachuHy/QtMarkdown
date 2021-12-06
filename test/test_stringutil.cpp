//
// Created by PikachuHy on 2021/11/14.
//
#include <catch2/catch_test_macros.hpp>
#include "render/StringUtil.h"
using namespace md;
using namespace md::render;
TEST_CASE( "Split String", "[string]" ) {
  md::String s = "我非常喜欢😍用";
  auto stringList = StringUtil::split(s);
  REQUIRE( stringList.size() == 3 );
  REQUIRE( stringList[0].type == RenderString::Chinese );
  REQUIRE( stringList[1].type == RenderString::Emoji );
  REQUIRE( stringList[2].type == RenderString::Chinese );
}