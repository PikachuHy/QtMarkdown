//
// Created by PikachuHy on 2021/11/5.
//

#include "PaintRecord.h"

namespace md::render {

PaintRecord PaintRecord::fromCell(Cell* cell, String latex, int latexFontSize) {
    PaintRecord rec;
    rec.type = Type::FromCell;
    rec.cell = cell;
    rec.cellLatex = std::move(latex);
    rec.cellFontSize = latexFontSize;
    return rec;
}

PaintRecord PaintRecord::fillRect(Point point, Size size, Color color) {
    PaintRecord rec;
    rec.type = Type::FillRect;
    rec.point = point;
    rec.size = size;
    rec.color = color;
    return rec;
}

PaintRecord PaintRecord::ellipse(Point point, Size size, Color color) {
    PaintRecord rec;
    rec.type = Type::Ellipse;
    rec.point = point;
    rec.size = size;
    rec.color = color;
    return rec;
}

PaintRecord PaintRecord::staticText(String text, Point point, Size size, Color color, Font font) {
    PaintRecord rec;
    rec.type = Type::StaticText;
    rec.text = std::move(text);
    rec.point = point;
    rec.size = size;
    rec.textColor = color;
    rec.textFont = font;
    return rec;
}

PaintRecord PaintRecord::image(String path, Point point, Size size) {
    PaintRecord rec;
    rec.type = Type::Image;
    rec.path = std::move(path);
    rec.point = point;
    rec.size = size;
    return rec;
}

PaintRecord PaintRecord::staticImage(String path, Point point, Size size) {
    PaintRecord rec;
    rec.type = Type::StaticImage;
    rec.path = std::move(path);
    rec.point = point;
    rec.size = size;
    return rec;
}

} // namespace md::render
