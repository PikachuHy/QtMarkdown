//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_RENDER_H
#define QTMARKDOWN_RENDER_H
#include "Instruction.h"
#include "mddef.h"
#include "parser/Document.h"
namespace md::render {
struct RenderSetting {
  bool highlightCurrentLine = false;
  int lineSpacing = 10;
  int maxWidth = 800;
  int latexFontSize = 20;
  String zhTextFont = "苹方-简";
  String enTextFont = "Times New Roman";
  std::array<int, 6> headerFontSize = {36, 28, 24, 20, 16, 14};
  QMargins docMargin = QMargins(100, 20, 20, 20);
  QMargins codeMargin = QMargins(10, 20, 20, 10);
  QMargins listMargin = QMargins(15, 20, 20, 10);
  QMargins checkboxMargin = QMargins(15, 20, 20, 10);
  QMargins quoteMargin = QMargins(10, 20, 20, 10);
  [[nodiscard]] int contentMaxWidth() const { return maxWidth - docMargin.left() - docMargin.right(); }
};
class Render {
 public:
  static Block render(parser::Node* node, sptr<RenderSetting> setting, DocPtr doc);

 private:
};
}  // namespace md::render
#endif  // QTMARKDOWN_RENDER_H
