//
// Created by PikachuHy on 2021/11/3.
//

#ifndef QTMARKDOWN_MDDEF_H
#define QTMARKDOWN_MDDEF_H
#include <memory>
#include <QString>
#include <QStringList>
#include <QList>
namespace md::parser {
class Node;
using NodePtrList = QList<Node*>;
class Token;
using TokenList = std::vector<Token>;
using String = QString;
using StringList = QStringList;
template<typename T>
using sptr = std::shared_ptr<T>;
class Document;
using DocPtr = sptr<Document>;
using Char = QChar;
using SizeType = qsizetype;
}
#endif  // QTMARKDOWN_MDDEF_H
