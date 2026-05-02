//
// Created by PikachuHy on 2021/11/5.
//

#include "EditorRenderer.h"
#include "QtAdapters.h"
#include "Cursor.h"
#include "Document.h"
#include "render/Instruction.h"

namespace md::editor {

EditorRenderer::EditorRenderer(Document& doc, const render::RenderSetting& setting)
    : m_doc(doc), m_setting(setting) {}

void EditorRenderer::drawDoc(core::AbstractPainter& /*painter*/, QPainter* nativePainter,
                              const core::Point& offset) {
    if (!nativePainter) return;
    QPoint qOffset = toQPoint(offset);
    qOffset.setY(qOffset.y() + m_setting.docMargin.top());
    for (const auto& block : m_doc.blocks()) {
        int h = block.height();
        for (const auto& instruction : block) {
            instruction->run(*nativePainter, qOffset, m_doc.parserDoc());
        }
        qOffset.setY(qOffset.y() + h + m_setting.blockSpacing);
    }
}

void EditorRenderer::drawCursor(core::AbstractPainter& painter, const core::Point& offset,
                                const Cursor& cursor, bool hasSelection) {
    if (hasSelection) return;
    core::Point pos = cursor.pos() + offset;
    painter.save();
    painter.setPen(core::Color(255, 0, 0));
    int x = pos.x;
    int y = pos.y;
    int h = cursor.height();
    int delta = 2;
    painter.drawLine(core::Point(x - delta, y), core::Point(x + delta, y));
    painter.drawLine(core::Point(x, y), core::Point(x, y + h));
    painter.drawLine(core::Point(x - delta, y + h), core::Point(x + delta, y + h));
    painter.restore();
}

void EditorRenderer::drawSelection(core::AbstractPainter& /*painter*/, QPainter* nativePainter,
                                    const core::Point& offset,
                                    const std::vector<InstructionPtr>& selectionInstructions,
                                    const Document& doc) {
    if (!nativePainter) return;
    if (selectionInstructions.empty()) return;
    QPoint qOffset = toQPoint(offset);
    for (const auto& instruction : selectionInstructions) {
        instruction->run(*nativePainter, qOffset, doc.parserDoc());
    }
}

int EditorRenderer::documentHeight() const {
    int h = 0;
    for (const auto& block : m_doc.blocks()) {
        h += block.height();
        h += m_setting.blockSpacing;
    }
    return h + m_setting.docMargin.top() + m_setting.docMargin.bottom();
}

int EditorRenderer::documentWidth() const {
    return m_setting.maxWidth;
}

} // namespace md::editor
