#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#define private public
#define protected public
#include "editor/Cursor.h"
#include "editor/Document.h"
#include "editor/Editor.h"
#include "parser/Document.h"
#include "parser/Text.h"
#undef protected
#undef private
#include <QGuiApplication>

#include "debug.h"
using namespace md::editor;
using md::parser::NodeType;
TEST_CASE("testing insert newline on code block") {
  Editor editor;
  editor.loadText(R"(

```
code
```

)");
  auto doc = editor.document();
  auto& cursor = *editor.m_cursor;
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  doc->insertReturn(*editor.m_cursor);
  // https://github.com/PikachuHy/QtMarkdown/issues/9
  doc->insertReturn(*editor.m_cursor);
}
int main(int argc, char** argv) {
  // 必须加这一句
  // 不然调用字体(QFontMetric)时会崩溃
  QGuiApplication app(argc, argv);
  doctest::Context context;

  int res = context.run();  // run

  if (context.shouldExit())  // important - query flags (and --exit) rely on the user doing this
    return res;              // propagate the result of the tests

  int client_stuff_return_code = 0;
  // your program - if the testing framework is integrated in your production code

  return res + client_stuff_return_code;  // the result from doctest is propagated here as well
}