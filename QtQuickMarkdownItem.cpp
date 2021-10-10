//
// Created by pikachu on 5/22/2021.
//

#include "QtQuickMarkdownItem.h"
#include "Render.h"
#include "Parser.h"
#include <QFile>
#include <QDebug>
#include <QTimer>
#include <QCursor>

QtQuickMarkdownItem::QtQuickMarkdownItem(QQuickItem* parent): QQuickPaintedItem(parent)
    ,m_render(nullptr)
    ,m_lastWidth(-1)
    ,m_lastImplicitWidth(-1)
{
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setFlag(ItemAcceptsInputMethod, true);
    QTimer::singleShot(0, this, [this]() {
        // qDebug() << "timeout";
        this->calculateHeight();
        this->update();
    });
    connect(this, &QtQuickMarkdownItem::widthChanged, [this]() {
        if (this->m_lastWidth == this->width()) return ;
        this->m_lastWidth = this->width();
        // qDebug() << "width change:" << this->width();
        calculateHeight();
    });
    connect(this, &QtQuickMarkdownItem::implicitWidthChanged, [this]() {
        if (this->m_lastImplicitWidth == this->implicitWidth()) return ;
        this->m_lastImplicitWidth = this->implicitWidth();
        // qDebug() << "implicit width change:" << this->implicitWidth();
        calculateHeight();
    });
}


void QtQuickMarkdownItem::paint(QPainter *painter) {
    if (!m_render) return;
    if (width() == 0) return;
    Document doc(m_text);
    m_render->setJustCalculate(false);
    m_render->reset(painter);
    doc.accept(m_render);
}

void QtQuickMarkdownItem::setText(const QString &text) {
//    qDebug() << "set text:" << text;
    if (text == m_text) return ;
    this->m_text = text;
    calculateHeight();
}

QString QtQuickMarkdownItem::text() const { return m_text; }

QString QtQuickMarkdownItem::source() const {
    return m_source;
}

void QtQuickMarkdownItem::setSource(const QString &source) {
    qDebug() << "load source:" << source;
    this->m_source = source;
    QFile file(source);
    if (file.exists()) {
        bool ok = file.open(QIODevice::ReadOnly);
        if (ok) {
            setText(file.readAll());
        } else {
            qWarning() << "file open fail." << source;
        }
    } else {
        qWarning() << "file not exist." << source;
    }
}

void QtQuickMarkdownItem::calculateHeight() {
    // qDebug() << width() << implicitWidth();
    int w = width();
    if (w == 0) return;
    delete m_render;
    Document doc(m_text);
    if (w < 200) {
        w = 400;
    }
    m_render = new Render(w, 0, m_path);
    m_render->setJustCalculate(true);
    doc.accept(m_render);
    setImplicitHeight(m_render->realHeight());
    setHeight(m_render->realHeight());
    emit heightChanged();
    // qDebug() << width() << implicitHeight() << height() << implicitHeight();
}

QString QtQuickMarkdownItem::path() const {
    return m_path;
}

void QtQuickMarkdownItem::setPath(const QString &path) {
    m_path = path;
    calculateHeight();
    update();
}

void QtQuickMarkdownItem::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) {
        return;
    }
    auto pos = event->pos();
    for(auto link: m_render->links()) {
        for(auto rect: link->rects) {
            if (rect.contains(pos)) {
                qDebug() << "click link:" << link->url;
                emit linkClicked(link->url);
            }
        }
    }
    for(auto image: m_render->images()) {
        if (image->rect.contains(pos)) {
            qDebug() << "click image:" << image->path;
            emit imageClicked(image->path);
        }
    }
    for (const auto &item : m_render->codes()) {
        if (item->rect.contains(pos)) {
            qDebug() << "copy code:" << item->code;
            emit codeCopied(item->code);
        }
    }
}


void QtQuickMarkdownItem::hoverMoveEvent(QHoverEvent *event) {
    // 消除警告
    auto posf= event->position();
    QPoint pos(posf.x(), posf.y());
    for(auto link: m_render->links()) {
        for(auto rect: link->rects) {
            if (rect.contains(pos)) {
                setCursor(QCursor(Qt::PointingHandCursor));
                return;
            }
        }
    }
    for(auto image: m_render->images()) {
        if (image->rect.contains(pos)) {
            setCursor(QCursor(Qt::PointingHandCursor));
            return;
        }
    }
    for(auto code: m_render->codes()) {
        if (code->rect.contains(pos)) {
            setCursor(QCursor(Qt::PointingHandCursor));
            return;
        }
    }

    setCursor(QCursor(Qt::ArrowCursor));
}
