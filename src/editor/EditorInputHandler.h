//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_EDITORINPUTHANDLER_H
#define QTMARKDOWN_EDITORINPUTHANDLER_H

#include <memory>
#include <vector>

#include "core/Event.h"
#include "core/Types.h"
#include "mddef.h"
#include "Editor.h"

namespace md::render {
class RenderSetting;
} // namespace md::render

namespace md::editor {
class Document;
class Cursor;
class SelectionRange;
struct CursorCoord;

class EditorInputHandler {
public:
    EditorInputHandler(Editor& editor, Document& doc, Cursor& cursor,
                       const render::RenderSetting& setting);

    void keyPressEvent(const core::KeyEvent& event);
    void keyReleaseEvent(const core::KeyEvent& event);
    void mousePressEvent(const core::Point& offset, const core::MouseEvent& event);
    void mouseMoveEvent(const core::Point& offset, const core::MouseEvent& event);
    void mouseReleaseEvent(const core::Point& offset, const core::MouseEvent& event);
    CursorShape cursorShape(const core::Point& offset, const core::Point& pos);

    // IME support
    void setPreedit(const String& str);
    void commitString(const String& str);
    bool isPreediting() const;
    core::Point preeditPos() const;

private:
    // Selection helpers
    void selectUp();
    void selectDown();
    void selectLeft();
    void selectRight();
    void selectBol();
    void selectEol();
    void selectAll();
    void removeSelection();
    void generateSelectionInstruction();

    // IME state
    bool m_preediting = false;
    int m_preeditLength = 0;
    core::Point m_preeditPos;

    // Shared state references
    Editor& m_editor;
    Document& m_doc;
    Cursor& m_cursor;
    const render::RenderSetting& m_setting;
};

} // namespace md::editor
#endif // QTMARKDOWN_EDITORINPUTHANDLER_H
