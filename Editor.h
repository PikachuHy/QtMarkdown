//
// Created by pikachu on 2021/3/19.
//

#ifndef QTMARKDOWNPARSER_EDITOR_H
#define QTMARKDOWNPARSER_EDITOR_H
#include <QTextEdit>
#include <QImage>
namespace Element {
    struct Link;
}
class Editor: public QWidget {
    Q_OBJECT
public:
    explicit Editor(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e) override;

    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QImage m_buffer;
    bool m_firstDraw;
    int m_rightMargin;
    QList<Element::Link*> m_links;
};


#endif //QTMARKDOWNPARSER_EDITOR_H
