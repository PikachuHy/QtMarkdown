#ifndef QTMARKDOWN_EDITORRENDERER_H
#define QTMARKDOWN_EDITORRENDERER_H

#include <memory>
#include <vector>

#include "core/AbstractPainter.h"
#include "core/Types.h"
#include "mddef.h"

namespace md::render {
class RenderSetting;
class Block;
} // namespace md::render

namespace md::editor {
class Document;
class Cursor;

class EditorRenderer {
public:
    EditorRenderer(Document& doc, const render::RenderSetting& setting);

    // -- Main paint entry points --
    void drawDoc(core::AbstractPainter& painter, QPainter* nativePainter,
                 const core::Point& offset);
    void drawCursor(core::AbstractPainter& painter, const core::Point& offset,
                    const Cursor& cursor, bool hasSelection);
    void drawSelection(core::AbstractPainter& painter, QPainter* nativePainter,
                       const core::Point& offset,
                       const std::vector<InstructionPtr>& selectionInstructions,
                       const Document& doc);

    int documentHeight() const;
    int documentWidth() const;

private:
    Document& m_doc;
    const render::RenderSetting& m_setting;
};

} // namespace md::editor
#endif // QTMARKDOWN_EDITORRENDERER_H
