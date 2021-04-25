//
// Created by pikachu on 2021/3/19.
//

#ifndef QTMARKDOWNPARSER_EDITOR_H
#define QTMARKDOWNPARSER_EDITOR_H
#include <QTextEdit>

class Editor: public QWidget {
    Q_OBJECT
public:
    explicit Editor(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e) override;

private:

};


#endif //QTMARKDOWNPARSER_EDITOR_H
