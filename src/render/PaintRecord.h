//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_PAINTRECORD_H
#define QTMARKDOWN_PAINTRECORD_H
#include "mddef.h"
#include "Cell.h"
#include <vector>

namespace md::render {

struct PaintRecord {
    enum class Type { FromCell, FillRect, Ellipse, StaticText, Image, StaticImage };
    Type type;

    // FromCell
    // Non-owning raw pointer. Ownership is in VisualLine::m_cells
    // (owned by Block::m_logicalLines). Valid only for the lifetime
    // of the PaintRecord's owning Block. Do not store, copy, or
    // dereference after Block destruction.
    Cell* cell = nullptr;
    String cellLatex;
    int cellFontSize = 0;

    // FillRect / Ellipse
    Point point;
    Size size;
    Color color = Qt::black;

    // StaticText
    String text;
    Color textColor;
    Font textFont;

    // Image / StaticImage
    String path;

    static PaintRecord fromCell(Cell* cell, String latex = "", int latexFontSize = 0);
    static PaintRecord fillRect(Point point, Size size, Color color);
    static PaintRecord ellipse(Point point, Size size, Color color);
    static PaintRecord staticText(String text, Point point, Size size, Color color, Font font);
    static PaintRecord image(String path, Point point, Size size);
    static PaintRecord staticImage(String path, Point point, Size size);
};

using PaintRecordList = std::vector<PaintRecord>;

}
#endif // QTMARKDOWN_PAINTRECORD_H
