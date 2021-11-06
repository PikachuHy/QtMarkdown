//
// Created by PikachuHy on 2021/11/3.
//

#ifndef QTMARKDOWN_MDDEF_H
#define QTMARKDOWN_MDDEF_H
#include <QList>
#include <QString>
#include <QStringList>
#include <memory>
namespace md {
namespace parser {
class Node;
class Token;
class Document;
class Container;
}  // namespace parser
using NodePtrList = QList<parser::Node*>;
using TokenList = std::vector<parser::Token>;
using String = QString;
using StringList = QStringList;
template <typename T>
using sptr = std::shared_ptr<T>;
using DocPtr = parser::Document*;
using ContainerPtr = sptr<parser::Container>;
using Char = QChar;
using SizeType = qsizetype;
}  // namespace md
#endif  // QTMARKDOWN_MDDEF_H
