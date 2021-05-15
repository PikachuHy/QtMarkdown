//
// Created by pikachu on 2021/3/19.
//

#include "Editor.h"
#include <QApplication>
#include <QDate>
#include <QFile>
#include <QDebug>
#include <QTextFrame>
#include <QTextList>
#include <QTextBlock>
#include <QTextTable>
#include <QLabel>
#include <QMovie>
#include "QtMarkdownParser"
#include <QPainter>
#include <QScrollArea>
#include <QMessageBox>
#include <QDesktopServices>
#include <QDebug>
#include <QFontDatabase>
#include <QStyle>
#include <QTimer>
#include <QtConcurrent>
#include <array>
#include <QProcess>
#include <QTemporaryFile>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace Element {
    struct Link {
        QString text;
        QString url;
        QList<QRect> rects;
    };
    struct Image {
        QString path;
        QRect rect;
    };
}


class EditorWidget: public QWidget {
Q_OBJECT
public:
    explicit EditorWidget(Editor* parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

public:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void loadFile(const QString& path);
    void reload();
private:
    void drawInBackground();
    void drawAsync();
private:
    QImage m_buffer;
    bool m_needDraw;
    int m_rightMargin;
    QList<Element::Link*> m_links;
    QList<Element::Image*> m_images;
    Editor* m_editor;
    bool m_isDrawing;
    int m_maxWidth;
    QSize m_fixedSize;
    QString m_filePath;
};

struct DefaultEditorVisitor: MultipleVisitor<Header,
        Text, ItalicText, BoldText, ItalicBoldText,
        Image, Link, CodeBlock, InlineCode, Paragraph,
        CheckboxList, CheckboxItem,
        UnorderedList, UnorderedListItem,
        OrderedList, OrderedListItem,
        LatexBlock, InlineLatex,
        Hr, QuoteBlock, Table> {
    explicit DefaultEditorVisitor(int w, int rightMargin, const QString& filePath):
        m_painter(nullptr),
            m_maxWidth(w - rightMargin), m_filePath(filePath), m_justCalculate(false) {
//        qDebug() << "width: " << m_maxWidth;
        m_lastMaxWidth = w;
        reset(m_painter);
    }
    void reset(QPainter* painter) {
        m_curX = 0;
        m_curY = 0;
        m_lastMaxHeight = 0;
        QFont font;
        font.setFamily("微软雅黑");
        font.setPixelSize(16);
        if (painter) {
            m_painter = painter;
            m_painter->setFont(font);
        } else {
            m_font = font;
        }
    }
    // 隔离QPainter
    void save() {
        if (justCalculate()) {
            m_fonts.push(curFont());
        } else {
            m_painter->save();
        }
    }
    void restore() {
        if (justCalculate()) {
            m_font = m_fonts.pop();
        } else {
            m_painter->restore();
        }
    }
    void setFont(const QFont& font) {
        if (justCalculate()) {
            m_font = font;
        } else {
            m_painter->setFont(font);
        }
    }
    void setPen(const QColor &color) {
        if (justCalculate()) {
            // do nothing
        } else {
            m_painter->setPen(color);
        }
    }
    QFontMetrics fontMetrics() {
        if (justCalculate()) {
            QFontMetrics fm(curFont());
            return fm;
        } else {
            return m_painter->fontMetrics();
        }
    }
    // 所有的绘制走自己写的绘制函数
    void drawText(const QRectF &r, const QString &text, const QTextOption &o = QTextOption()) {
        if (justCalculate()) {
            // do nothing
        } else {
            m_painter->drawText(r, text, o);
        }
    }
    void drawImage(const QRect &r, const QImage &image) {
        if (justCalculate()) {
            // do nothing
        } else {
            m_painter->drawImage(r, image);
        }
    }
    void drawPixmap(const QRect &r, const QPixmap &pm) {
        if (justCalculate()) {
            // do nothing
        } else {
            m_painter->drawPixmap(r, pm);
        }
    }
    void fillRect(const QRect &rect, const QBrush &b) {
        if (justCalculate()) {
            // do nothing
        } else {
            m_painter->fillRect(rect, b);
        }
    }
    QRect textRect(const QString& text) {
        QFontMetrics metrics = fontMetrics();
        QRect textBoundingRect = metrics.boundingRect(QRect(m_curX, m_curY, m_maxWidth, 0), Qt::TextWordWrap, text);
        return textBoundingRect;
    }
    int textWidth(const QString& text) {
        QFontMetrics metrics = fontMetrics();
        int w = metrics.horizontalAdvance(text);
        return w;
    }
    int charWidth(const QChar& ch) {
        QFontMetrics metrics = fontMetrics();
        int w = metrics.horizontalAdvance(ch);
        return w;
    }
    int textHeight() {
        return fontMetrics().height();
    }
    bool currentLineCanDrawText(const QString& text) {
        auto needWidth = textWidth(text);
        // qDebug() << "need" << needWidth << text;
        if (m_curX + needWidth < m_maxWidth) {
            return true;
        } else {
            return false;
        }
    }
    int countOfThisLineCanDraw(const QString& text) {
        // 计算这一行可以画多少个字符
        auto ch_w = charWidth(text.at(0));
        int left_w = m_maxWidth - m_curX;
        int may_ch_count = left_w / ch_w - 1;
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
    QList<QRect> drawText(QString text) {
        if (text == "\r") return {};
//         qDebug() << "draw" << text;
        if (text.isEmpty()) return {};
        QList<QRect> rects;
        while (!currentLineCanDrawText(text)) {
            // qDebug() << text;
            auto count = countOfThisLineCanDraw(text);
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
    QRect drawTextInCurrentLine(const QString& text, bool adjustX = true, bool adjustY = true) {
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
    void moveToNewLine() {
        m_curY += m_lastMaxHeight;
        m_curX = 0;
        m_lastMaxHeight = 0;
    }
    void visit(Header *node) override {
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
    void visit(Text *node) override {
        drawText(node->str());
    }
    void visit(ItalicText *node) override {
        save();
        QFont font = curFont();
        font.setItalic(true);
        setFont(font);
        drawText(node->str());
        restore();
    }
    void visit(BoldText *node) override {
        save();
        QFont font = curFont();
        font.setBold(true);
        setFont(font);
        drawText(node->str());
        restore();
    }
    void visit(ItalicBoldText *node) override {
        save();
        QFont font = curFont();
        font.setItalic(true);
        font.setBold(true);
        setFont(font);
        drawText(node->str());
        restore();
    }
    void visit(Image *node) override {
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
                        qDebug() << rect << playIconRect << playImage.size();
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
    void visit(Link *node) override {
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
    void drawCodeBlock(const String& code) {
        moveToNewLine();
        auto y = m_curY;
        m_curY += 10;
        m_curX += 20;
        save();
        // #f9f9f9
//        m_painter.setBackground(QBrush(QColor(249, 249, 249)));
        QFont font;
        font.setPixelSize(20);
        font.setFamily("Cascadia Code");
        setFont(font);
        auto rect = textRect(code);
        fillRect(QRect(0, y, m_maxWidth, rect.height()), QBrush(QColor(249, 249, 249)));
        drawText(rect, code);
        restore();
        m_curY += rect.height();
        m_curY += 10;
    }
    void visit(CodeBlock *node) override {
        if (node && node->code()) {
            auto code = node->code()->str();
            drawCodeBlock(code);
        }
    }
    void visit(InlineCode *node) override {
        save();
        // #f9f9f9
//        m_painter.setBackground(QBrush(QColor(249, 249, 249)));
        QFont font;
        font.setPixelSize(16);
        font.setFamily("Cascadia Code");
        setFont(font);
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
    void visit(LatexBlock *node) override {
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
    void visit(InlineLatex *node) override {
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
    void visit(Paragraph *node) override {
        save();
        moveToNewLine();
        for(auto it: node->children()) {
            it->accept(this);
        }
        m_curY += 5;
        restore();
    }
    void visit(CheckboxList *node) override {
        for (const auto &item : node->children()) {
            moveToNewLine();
            m_curX += 16;
            item->accept(this);
            m_curY += 5;
        }
        m_curY += 10;
    }
    void visit(CheckboxItem *node) override {
        save();
        auto font = curFont();
        // 计算高度偏移
        auto h1 = textHeight();
        save();
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
        restore();
        font = curFont();
        font.setStrikeOut(node->isChecked());
        setFont(font);
        for (const auto &item : node->children()) {
            item->accept(this);
        }
        restore();
    }
    void visit(UnorderedList *node) override {
        for (const auto &item : node->children()) {
            moveToNewLine();
            m_curX += 32;
            drawText("●  ");
            item->accept(this);
        }
        m_curY += 10;
    }
    void visit(UnorderedListItem *node) override {
        for (const auto &item : node->children()) {
            item->accept(this);
        }
    }
    void visit(OrderedList *node) override {
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
    void visit(OrderedListItem *node) override {
        for (const auto &item : node->children()) {
            item->accept(this);
        }
    }
    void visit(Hr *node) override {
    }
    void visit(QuoteBlock *node) override {
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
    void visit(Table *node) override {
    }
    int realHeight() const {
        return m_curY + m_lastMaxHeight;
    }
    int realWidth() const {
        return m_lastMaxWidth;
    }
    const QList<Element::Link*>& links() {
        return m_links;
    }
    const QList<Element::Image*>& images() {
        return m_images;
    }
    [[nodiscard]] inline bool justCalculate() const {
        return m_justCalculate;
    }
    void setJustCalculate(bool flag) {
        m_justCalculate = flag;
    }
    QFont curFont() {
        return m_font;
    }
    void setPainter(QPainter* painter) {
        m_painter = painter;
    }
private:
    QPainter* m_painter;
    int m_curX;
    int m_curY;
    int m_lastMaxHeight;
    int m_lastMaxWidth;
    int m_maxWidth;
    QList<Element::Link*> m_links;
    QList<Element::Image*> m_images;
    const QString& m_filePath;
    QMap<QString, QString> m_cacheLatexImage;
    bool m_justCalculate;
    QStack<QFont> m_fonts;
    QFont m_font;
};

template<typename T>
void checkFuture(QFuture<T> future, std::function<void(T)> callback) {
    if (!future.isFinished()) {
        QTimer::singleShot(100, [future, callback](){
            checkFuture(future, callback);
        });
    } else {
        callback(future.result());
    }
}
EditorWidget::EditorWidget(Editor *parent)
    : QWidget(parent)
    , m_needDraw(true)
    , m_editor(parent)
    {
    m_rightMargin = 0;
    setMouseTracking(true);
    m_buffer = QImage(QSize(800, 600), QImage::Format_RGB32);
    m_buffer.fill(Qt::white);
}

void EditorWidget::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e);
//    qDebug() << e;
    if (m_needDraw) {
        m_needDraw = false;
        this->drawInBackground();
    }
    QPainter painter(this);
    painter.drawImage(0, 0, m_buffer);
}

void EditorWidget::mouseMoveEvent(QMouseEvent *event) {
    auto pos = event->pos();
    for(auto link: m_links) {
        for(auto rect: link->rects) {
            if (rect.contains(pos)) {
                setCursor(QCursor(Qt::PointingHandCursor));
                return;
            }
        }
    }
    for(auto image: m_images) {
        if (image->rect.contains(pos)) {
            setCursor(QCursor(Qt::PointingHandCursor));
            return;
        }
    }

    setCursor(QCursor(Qt::ArrowCursor));
}

void EditorWidget::mousePressEvent(QMouseEvent *event) {
    auto pos = event->pos();
    for(auto link: m_links) {
        for(auto rect: link->rects) {
            if (rect.contains(pos)) {
                auto ret = QMessageBox::question(this, tr("Open URL"),
                                         QString("%1").arg(link->url)
                                         );
                if (ret == QMessageBox::Yes) {
                    QDesktopServices::openUrl(QUrl(link->url));
                }
            }
        }
    }
    for(auto image: m_images) {
        if (image->rect.contains(pos)) {
            QDialog dialog;
            dialog.setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
            auto hbox = new QHBoxLayout();
            auto imgLabel = new QLabel();
            if (image->path.endsWith(".gif")) {
                auto m = new QMovie(image->path);
                imgLabel->setMovie(m);
                m->start();
            } else {
                imgLabel->setPixmap(QPixmap(image->path));
            }
            hbox->addWidget(imgLabel);
            hbox->setContentsMargins(0, 0, 0, 0);
            dialog.setLayout(hbox);
            dialog.exec();
        }
    }
}

void EditorWidget::resizeEvent(QResizeEvent *event) {
//    qDebug() << __FUNCTION__ ;
//    m_needDraw = true;
    this->update();
}

bool EditorWidget::eventFilter(QObject *watched, QEvent *event) {
//    qDebug() << event->type() << watched << m_editor;
    if (watched == m_editor) {
        if (event->type() == QEvent::Resize) {
            QResizeEvent* e = dynamic_cast<QResizeEvent *>(event);
//            qDebug() << e->size();
            auto scrollBarWidth = this->style()->pixelMetric(QStyle::PM_ScrollBarSliderMin);
            this->m_maxWidth = qMax(600, e->size().width()) - scrollBarWidth - 1;
            if (!m_isDrawing) {
                m_needDraw = true;
//                qDebug() << __FUNCTION__ ;
                this->update();
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

void EditorWidget::loadFile(const QString &path)
{
    m_filePath = path;
    m_needDraw = true;
}

void EditorWidget::reload()
{
    m_needDraw = true;
    update();
}

void EditorWidget::drawInBackground() {
//    m_maxWidth = qMax(600, parentWidget()->width());
    auto ret = QtConcurrent::run([this](int){
        this->m_isDrawing = true;
        this->drawAsync();
        return 0;
    }, 0);
    checkFuture<int>(ret, [this](int) {
        setFixedSize(m_fixedSize);
        this->m_isDrawing = false;
        this->update();
    });
}

void EditorWidget::drawAsync() {
    QFile mdFile(m_filePath);
    if (!mdFile.exists()) {
        qDebug() << "file not exist:" << mdFile.fileName();
        return;
    }
    mdFile.open(QIODevice::ReadOnly);
    auto mdText = mdFile.readAll();
    mdFile.close();
    Document doc(mdText);
    DefaultEditorVisitor visitor(m_maxWidth, m_rightMargin, m_filePath);
    visitor.setJustCalculate(true);
    doc.accept(&visitor);
    int h = visitor.realHeight();
    if (h < 0) {
        h = 600;
    }
    h = std::max(h, this->height());
    auto w = qMax(m_maxWidth, visitor.realWidth());
    // qDebug() << "set size:" << w << h;
    m_fixedSize = QSize(w, h);
    m_buffer = QImage(m_fixedSize, QImage::Format_RGB32);
    QPainter painter(&m_buffer);
    painter.setRenderHint(QPainter::Antialiasing);
    m_buffer.fill(Qt::white);
    visitor.setJustCalculate(false);
    visitor.reset(&painter);
    doc.accept(&visitor);
    m_links = visitor.links();
    m_images = visitor.images();
}


Editor::Editor(QWidget *parent) : QScrollArea(parent) {
    setWidgetResizable(true);
    m_editorWidget = new EditorWidget(this);
    setWidget(m_editorWidget);
    installEventFilter(m_editorWidget);
}

void Editor::loadFile(const QString &path) {
    m_editorWidget->loadFile(path);
}

void Editor::reload()
{
    m_editorWidget->reload();
}

#include "Editor.moc"
