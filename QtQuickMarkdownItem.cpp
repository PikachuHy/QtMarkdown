//
// Created by pikachu on 5/22/2021.
//

#include "QtQuickMarkdownItem.h"
#include "Render.h"
#include "Parser.h"
#include <QFile>
#include <QDebug>
#include <QTimer>

QtQuickMarkdownItem::QtQuickMarkdownItem(QQuickItem* parent): QQuickPaintedItem(parent)
    ,m_render(nullptr)
    ,m_lastWidth(-1)
    ,m_lastImplicitWidth(-1)
{
    QTimer::singleShot(0, this, [this]() {
        qDebug() << "timeout";
        this->calculateHeight();
        this->update();
    });
}


void QtQuickMarkdownItem::paint(QPainter *painter) {
    if (!m_render) return;
    if (width() == 0) return;
    Document doc(m_text);
    m_render->setJustCalculate(false);
    m_render->reset(painter);
    doc.accept(m_render);
    connect(this, &QtQuickMarkdownItem::widthChanged, [this]() {
        if (this->m_lastWidth == this->width()) return ;
        this->m_lastWidth = this->width();
        qDebug() << "width change:" << this->width();
        calculateHeight();
    });
    connect(this, &QtQuickMarkdownItem::implicitWidthChanged, [this]() {
        if (this->m_lastImplicitWidth == this->implicitWidth()) return ;
        this->m_lastImplicitWidth = this->implicitWidth();
        qDebug() << "implicit width change:" << this->implicitWidth();
        calculateHeight();
    });
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
    qDebug() << width() << implicitWidth();
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
    qDebug() << width() << implicitHeight() << height() << implicitHeight();
}

QString QtQuickMarkdownItem::path() const {
    return m_path;
}

void QtQuickMarkdownItem::setPath(const QString &path) {
    m_path = path;
    calculateHeight();
    update();
}
