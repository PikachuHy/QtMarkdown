//
// Created by PikachuHy on 2021/11/5.
//

#include "PaintPass.h"

namespace md::render {

PaintPass::PaintPass(const PaintRecordList& records)
    : m_records(records) {}

InstructionPtrList PaintPass::execute() {
    InstructionPtrList instructions;
    for (const auto& rec : m_records) {
        switch (rec.type) {
            case PaintRecord::Type::FromCell: {
                if (auto* textCell = dynamic_cast<TextCell*>(rec.cell)) {
                    instructions.push_back(std::make_unique<TextInstruction>(textCell));
                } else if (auto* latexCell = dynamic_cast<InlineLatexCell*>(rec.cell)) {
                    instructions.push_back(std::make_unique<LatexInstruction>(latexCell, rec.cellLatex, rec.cellFontSize));
                }
                break;
            }
            case PaintRecord::Type::FillRect:
                instructions.push_back(std::make_unique<FillRectInstruction>(rec.point, rec.size, rec.color));
                break;
            case PaintRecord::Type::Ellipse:
                instructions.push_back(std::make_unique<EllipseInstruction>(rec.point, rec.size, rec.color));
                break;
            case PaintRecord::Type::StaticText:
                instructions.push_back(std::make_unique<StaticTextInstruction>(rec.text, rec.point, rec.size, rec.textColor, rec.textFont));
                break;
            case PaintRecord::Type::Image:
                instructions.push_back(std::make_unique<ImageInstruction>(rec.path, rec.point, rec.size));
                break;
            case PaintRecord::Type::StaticImage:
                instructions.push_back(std::make_unique<StaticImageInstruction>(rec.path, rec.point, rec.size));
                break;
        }
    }
    return instructions;
}

}
