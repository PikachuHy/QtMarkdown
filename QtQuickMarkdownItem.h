//
// Created by pikachu on 5/22/2021.
//

#ifndef QTMARKDOWN_QTQUICKMARKDOWNITEM_H
#define QTMARKDOWN_QTQUICKMARKDOWNITEM_H
#include "QtMarkdown_global.h"
#include <QQuickPaintedItem>
class Render;
class QTMARKDOWNSHARED_EXPORT QtQuickMarkdownItem : public QQuickPaintedItem{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QString source READ source WRITE setSource)
    Q_PROPERTY(QString path READ path WRITE setPath)
    QML_ELEMENT
    QML_NAMED_ELEMENT(QtQuickMarkdownItem)
public:
    explicit QtQuickMarkdownItem(QQuickItem* parent = nullptr);
    void paint(QPainter *painter) override;
    [[nodiscard]] QString text() const;
    void setText(const QString& text);
    [[nodiscard]] QString source() const;
    void setSource(const QString& source);
    [[nodiscard]] QString path() const;
    void setPath(const QString& path);

private:
    void calculateHeight();
private:
    QString m_text;
    QString m_source;
    QString m_path;
    Render* m_render;
    int m_lastWidth;
    int m_lastImplicitWidth;
};


#endif //QTMARKDOWN_QTQUICKMARKDOWNITEM_H
