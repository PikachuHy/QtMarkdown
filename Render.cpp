//
// Created by pikachu on 5/22/2021.
//

#include "Render.h"
#include <QDebug>
#include <QFontDatabase>

Render::Render(int w, int rightMargin, const QString &filePath) :
        m_painter(nullptr),
        m_maxWidth(w - rightMargin), m_filePath(filePath), m_justCalculate(false) {
//        qDebug() << "width: " << m_maxWidth;
    m_lastMaxWidth = w;
    reset(m_painter);
}

void Render::reset(QPainter *painter) {
    m_curX = 0;
    m_curY = 0;
    m_lastMaxHeight = 0;
    QFont font;
#ifdef Q_OS_WIN
    font.setFamily("微软雅黑");
#elif defined(Q_OS_MAC)
    font.setFamily("PingFang SC");
#else
#endif
    font.setPixelSize(16);
    if (painter) {
        m_painter = painter;
        m_painter->setFont(font);
    } else {
        m_font = font;
    }
}

void Render::save() {
    if (justCalculate()) {
        m_fonts.push(curFont());
    } else {
        m_painter->save();
    }
}

void Render::restore() {
    if (justCalculate()) {
        m_font = m_fonts.pop();
    } else {
        m_painter->restore();
    }
}

void Render::setFont(const QFont &font) {
    if (justCalculate()) {
        m_font = font;
    } else {
        m_painter->setFont(font);
    }
}

void Render::setPen(const QColor &color) {
    if (justCalculate()) {
        // do nothing
    } else {
        m_painter->setPen(color);
    }
}

QFontMetrics Render::fontMetrics() {
    if (justCalculate()) {
        QFontMetrics fm(curFont());
        return fm;
    } else {
        return m_painter->fontMetrics();
    }
}

void Render::drawText(const QRectF &r, const QString &text, const QTextOption &o) {
    if (justCalculate()) {
        // do nothing
    } else {
        m_painter->drawText(r, text, o);
    }
}

void Render::drawImage(const QRect &r, const QImage &image) {
    if (justCalculate()) {
        // do nothing
    } else {
        m_painter->drawImage(r, image);
    }
}

void Render::drawPixmap(const QRect &r, const QPixmap &pm) {
    if (justCalculate()) {
        // do nothing
    } else {
        m_painter->drawPixmap(r, pm);
    }
}

void Render::fillRect(const QRect &rect, const QBrush &b) {
    if (justCalculate()) {
        // do nothing
    } else {
        m_painter->fillRect(rect, b);
    }
}

QRect Render::textRect(const QString &text) {
    QFontMetrics metrics = fontMetrics();
    QRect textBoundingRect = metrics.boundingRect(QRect(m_curX, m_curY, m_maxWidth, 0), Qt::TextWordWrap, text);
    return textBoundingRect;
}

int Render::textWidth(const QString &text) {
    QFontMetrics metrics = fontMetrics();
    int w = metrics.horizontalAdvance(text);
    return w;
}

int Render::charWidth(const QChar &ch) {
    QFontMetrics metrics = fontMetrics();
    int w = metrics.horizontalAdvance(ch);
    return w;
}

int Render::textHeight() {
    return fontMetrics().height();
}

bool Render::currentLineCanDrawText(const QString &text) {
    auto needWidth = textWidth(text);
    // qDebug() << "need" << needWidth << text;
    if (m_curX + needWidth < m_maxWidth) {
        return true;
    } else {
        return false;
    }
}

int Render::countOfThisLineCanDraw(const QString &text) {
    // 计算这一行可以画多少个字符
    auto ch_w = charWidth(text.at(0));
    int left_w = m_maxWidth - m_curX;
    int may_ch_count = left_w / ch_w - 1;
    // 可能根本画不了
    if (may_ch_count <= 0) return 0;
    if (currentLineCanDrawText(text.left(may_ch_count + 1))) {
        while (currentLineCanDrawText(text.left(may_ch_count + 1))) {
            may_ch_count++;
        }
    } else {
        while (!currentLineCanDrawText(text.left(may_ch_count))) {
            may_ch_count--;
        }
    }
    return may_ch_count;
}

QList<QRect> Render::drawText(QString text) {
    if (text == "\r") return {};
//         qDebug() << "draw" << text;
    if (text.isEmpty()) return {};
    QList<QRect> rects;
    while (!currentLineCanDrawText(text)) {
        // qDebug() << text;
        auto count = countOfThisLineCanDraw(text);
        // 处理画不了的特殊情况
        if (count == 0) {
            moveToNewLine();
            continue;
        }
        auto rect = drawTextInCurrentLine(text.left(count));
        rects.append(rect);
        text = text.right(text.size() - count);
        m_curX = 0;
        m_curY += rect.height();
        m_curY += 5;
    }
    if (!text.isEmpty()) {
        auto rect = drawTextInCurrentLine(text);
        rects.append(rect);
    }
    return rects;
}

QRect Render::drawTextInCurrentLine(const QString &text, bool adjustX, bool adjustY) {
//        qDebug() << "cur";
    auto rect = textRect(text);
//        qDebug() << rect << text;
    drawText(rect, text);
    if (adjustX) {
        m_curX += rect.width();
    }
    if (adjustY) {
        m_lastMaxHeight = qMax(m_lastMaxHeight, rect.height());
    }
//        m_painter.drawRect(rect);
    return rect;
}

void Render::moveToNewLine() {
    m_curY += m_lastMaxHeight;
    m_curX = 0;
    m_lastMaxHeight = 0;
}

void Render::visit(Header *node) {
    std::array<int, 6> fontSize = {
            36, 28,24, 20, 16, 14
    };
    save();
    auto font = curFont();
    font.setPixelSize(fontSize[node->level() - 1]);
    font.setBold(true);
    setFont(font);
    moveToNewLine();
//        QString hn = "h" + String::number(node->level());
//        drawText(hn);
    m_curX += 10;
    for(auto it: node->children()) {
        it->accept(this);
    }
    m_curY += 10;
    restore();
}

void Render::visit(Text *node) {
    drawText(node->str());
}

void Render::visit(ItalicText *node) {
    save();
    QFont font = curFont();
    font.setItalic(true);
    setFont(font);
    drawText(node->str());
    restore();
}

void Render::visit(BoldText *node) {
    save();
    QFont font = curFont();
    font.setBold(true);
    setFont(font);
    drawText(node->str());
    restore();
}

void Render::visit(ItalicBoldText *node) {
    save();
    QFont font = curFont();
    font.setItalic(true);
    font.setBold(true);
    setFont(font);
    drawText(node->str());
    restore();
}

void Render::visit(Image *node) {
    moveToNewLine();
    QString imgPath = node->src()->str();
    QDir dir(imgPath);
    if (dir.isRelative()) {
        QDir basePath(m_filePath);
        basePath.cdUp();
        imgPath = QString("%1/%2").arg(basePath.absolutePath(), imgPath);
    }
    QFile file(imgPath);
    if (file.exists()) {
        QImage image(imgPath);
        int imageMaxWidth = qMin(1080, m_maxWidth);
        if (image.width() > imageMaxWidth) {
            image = image.scaledToWidth(imageMaxWidth);
        }
        QRect rect(QPoint(m_curX, m_curY), image.size());
//            qDebug() << "image rect" << rect;
        drawImage(rect, image);
        m_lastMaxHeight = rect.height();
        if (justCalculate()) {

        } else {
            auto img = new Element::Image();
            img->path = imgPath;
            img->rect = rect;
            m_images.append(img);
            if (imgPath.endsWith(".gif")) {
                QString playIconPath = ":/icon/play_64x64.png";
                QFile playIconFile(playIconPath);
                if (playIconFile.exists()) {
                    QImage playImage(playIconPath);
                    // 计算播放图标所在位置
                    // 播放图标放在中心位置
                    int x = (rect.width() - playImage.width()) / 2;
                    int y = (rect.height() - playImage.height()) / 2;
                    QRect playIconRect(
                            QPoint(
                                    rect.x() + x,
                                    rect.y() + y),
                            playImage.size()
                    );
                    // qDebug() << rect << playIconRect << playImage.size();
                    drawImage(playIconRect, playImage);
                } else {
                    qWarning() << "play icon file not exist.";
                }
            }
        }
    } else {
        qWarning() << "image not exist." << imgPath;
    }
}

void Render::visit(Link *node) {
    save();
    setPen(Qt::blue);
    auto font = curFont();
    font.setUnderline(true);
    setFont(font);
    auto rects = drawText(node->content()->str());
    if (justCalculate()) {

    } else {
        auto link = new Element::Link();
        link->text = node->content()->str();
        link->url = node->href()->str();
        link->rects = rects;
        m_links.append(link);
    }
    restore();
}

void Render::drawCodeBlock(const String &code) {
    moveToNewLine();
    auto y = m_curY;
    m_curY += 10;
    m_curX += 20;
    save();
    // #f9f9f9
//        m_painter.setBackground(QBrush(QColor(249, 249, 249)));
    setFont(codeFont());
    auto rect = textRect(code);
    const QRect bgRect = QRect(0, y, m_maxWidth, rect.height());
    fillRect(bgRect, QBrush(QColor(249, 249, 249)));
    drawText(rect, code);
    restore();
    m_curY += rect.height();
    m_curY += 10;
    if (!justCalculate()) {
        auto e = new Element::CodeBlock();
        e->code = code;
        QString copyBtnFilePath = ":icon/copy_32x32.png";
        QFile copyBtnFile(copyBtnFilePath);
        if (copyBtnFile.exists()) {
            QPixmap copyBtnImg(copyBtnFilePath);
            QRect copyBtnRect(QPoint(bgRect.width() - copyBtnImg.width(), bgRect.y()), copyBtnImg.size());
            e->rect = copyBtnRect;
            m_codes.append(e);
            drawPixmap(copyBtnRect, copyBtnImg);
        } else {
            qWarning() << "copy btn file not exist." << copyBtnFilePath;
        }
    }
}

void Render::visit(CodeBlock *node) {
    if (node && node->code()) {
        auto code = node->code()->str();
        drawCodeBlock(code);
    }
}

void Render::visit(InlineCode *node) {
    save();
    // #f9f9f9
//        m_painter.setBackground(QBrush(QColor(249, 249, 249)));
    setFont(codeFont());
    QString code = node->code() ? node->code()->str() : " ";
    if (currentLineCanDrawText(code)) {
        auto rect = textRect(code);
        fillRect(rect, QBrush(QColor(249, 249, 249)));
        drawText(rect, code);
        m_curX += rect.width();
        m_lastMaxHeight = qMax(m_lastMaxHeight, rect.height());
    } else {
        drawCodeBlock(code);
    }
    restore();
}

void Render::visit(LatexBlock *node) {
    // qDebug() << node->code()->str();
    moveToNewLine();
    m_curY += 10;
    QTemporaryFile tmpFile;
    if (tmpFile.open()) {
        tmpFile.write(node->code()->str().toUtf8());
        tmpFile.close();
        QStringList args;
        QString imgFilename = tmpFile.fileName() + ".png";
        args << tmpFile.fileName() << imgFilename;
        QProcess p;
        p.start("latex2png.exe", args);
        auto ok = p.waitForFinished();
        if (!ok) {
            qDebug() << "LaTex.exe run fail";
            return;
        }
        if (!QFile(imgFilename).exists()) {
            qDebug() << "file not exist." << imgFilename;
            return;
        }
        QPixmap image(imgFilename);
        auto x = (m_maxWidth - image.size().width())/2;
        const QRect rect = QRect(QPoint(x, m_curY), image.size());
//            m_painter.drawRect(rect);
        drawPixmap(rect, image);
        m_curY += image.height();
        m_curY += 10;
        if (justCalculate()) {

        } else {
            auto img = new Element::Image();
            img->path = imgFilename;
            img->rect = rect;
            m_images.append(img);
        }
    } else {
        qDebug() << "tmp file open fail." << tmpFile.fileName();
    }
}

void Render::visit(InlineLatex *node) {
    QString key = node->code()->str();
    qDebug() << key;
    QString imgFilename;
    if (m_cacheLatexImage.contains(key) && QFile(m_cacheLatexImage[key]).exists()) {
        imgFilename = m_cacheLatexImage[key];
    } else {
        //return;
        QTemporaryFile tmpFile;
        if (tmpFile.open()) {
            tmpFile.write(node->code()->str().toUtf8());
            tmpFile.close();
            QStringList args;
            imgFilename = tmpFile.fileName() + ".png";
            args << tmpFile.fileName() << imgFilename;
//                qDebug() << args;
            QProcess p;
            p.start("latex2png.exe", args);
            auto ok = p.waitForFinished();
            if (!ok) {
                qDebug() << "LaTex.exe run fail";
                return;
            }
            if (!QFile(imgFilename).exists()) {
                qDebug() << "file not exist." << imgFilename;
                return;
            }
        } else {
            qDebug() << "tmp file open fail." << tmpFile.fileName();
            return;
        }
    }
    QPixmap image(imgFilename);
    // 如果画不下，硬画的话会卡死
    // 所以要换行
    if (m_curX + image.width() + 5 + 5 < m_maxWidth) {
        m_curX += 5;
    } else {
        moveToNewLine();
    }
    const QRect rect = QRect(QPoint(m_curX, m_curY), image.size());
    m_curX += image.width();
    m_curX += 5;
    m_lastMaxHeight = qMax(m_lastMaxHeight, image.height());
    drawPixmap(rect, image);
}

void Render::visit(Paragraph *node) {
    save();
    moveToNewLine();
    for(auto it: node->children()) {
        it->accept(this);
    }
    m_curY += 5;
    restore();
}

void Render::visit(CheckboxList *node) {
    for (const auto &item : node->children()) {
        moveToNewLine();
        m_curX += 16;
        item->accept(this);
        m_curY += 5;
    }
    m_curY += 10;
}

void Render::visit(CheckboxItem *node) {
    save();
    auto font = curFont();
    // 计算高度偏移
    auto h1 = textHeight();
    save();
    if (node->isChecked()) {
        QPixmap image(":icon/checkbox-selected_64x64.png");
        const QRect rect = QRect(QPoint(m_curX, m_curY), QSize(h1, h1));
        drawPixmap(rect, image);
    } else {
        QPixmap image(":icon/checkbox-unselected_64x64.png");
        const QRect rect = QRect(QPoint(m_curX, m_curY), QSize(h1, h1));
        drawPixmap(rect, image);
    }
    m_curX += h1 + 10;
    /*
    font.setPixelSize(36);
    QFontMetrics fm(font);
    auto h2 = fm.height();
    setFont(font);
    auto y = m_curY;
    // 偏移绘制点，使得框框和文字是在同一条线
    m_curY-= (h2 - h1) / 2;
    if (node->isChecked()) {
        auto rect = drawTextInCurrentLine("▣", true, false);
    } else {
        drawTextInCurrentLine("▢", true, false);
    }
    m_curY = y;
    m_curX += 5;
    */
    restore();
    font = curFont();
    font.setStrikeOut(node->isChecked());
    setFont(font);
    for (const auto &item : node->children()) {
        item->accept(this);
    }
    restore();
}

void Render::visit(UnorderedList *node) {
    for (const auto &item : node->children()) {
        moveToNewLine();
        m_curX += 32;
        drawText("●  ");
        item->accept(this);
    }
    m_curY += 10;
}

void Render::visit(UnorderedListItem *node) {
    for (const auto &item : node->children()) {
        item->accept(this);
    }
}

void Render::visit(OrderedList *node) {
    int i = 0;
    for (const auto &item : node->children()) {
        i++;
        moveToNewLine();
        m_curX += 32;
        QString numStr = QString("%1.  ").arg(i);
        drawText(numStr);
        item->accept(this);
    }
    m_curY += 10;
}

void Render::visit(OrderedListItem *node) {
    for (const auto &item : node->children()) {
        item->accept(this);
    }
}

void Render::visit(Hr *node) {
}

void Render::visit(QuoteBlock *node) {
    moveToNewLine();
    int startY = m_curY;
    for(auto child: node->children()) {
        m_curX += 10;
        child->accept(this);
        moveToNewLine();
    }
    int endY = m_curY;
    const QRect rect = QRect(2, startY, 5, endY - startY);
    // qDebug() << rect;
    // #eee
    fillRect(rect, QBrush(QColor(238, 238, 238)));
}

void Render::visit(Table *node) {
}

int Render::realHeight() const {
    return m_curY + m_lastMaxHeight;
}

int Render::realWidth() const {
    return m_lastMaxWidth;
}

const QList<Element::Link *> &Render::links() {
    return m_links;
}

const QList<Element::Image *> &Render::images() {
    return m_images;
}

const QList<Element::CodeBlock *> &Render::codes() {
    return m_codes;
}

bool Render::justCalculate() const {
    return m_justCalculate;
}

void Render::setJustCalculate(bool flag) {
    m_justCalculate = flag;
}

QFont Render::curFont() {
    return m_font;
}

void Render::setPainter(QPainter *painter) {
    m_painter = painter;
}

QFont Render::codeFont() {
    auto font = curFont();
#ifdef Q_OS_WiN
    font.setFamily("Cascadia Code");
    font.setPixelSize(20);
#else
    font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPixelSize(20);
#endif
    return font;
}
