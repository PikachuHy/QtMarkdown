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
class IBufferProvider;
class Container;
}  // namespace parser
// NodePtrList is now defined in parser/Node.h as std::vector<std::unique_ptr<Node>>
// The alias here is kept for backward compatibility with external code
// Prefer including parser/Node.h for the canonical definition
// TokenList uses std::vector for iterator stability during parsing.
// StringList uses QStringList (QList<QString>) for Qt interop.
// This inconsistency is historical; both have their appropriate use cases.
using TokenList = std::vector<parser::Token>;
using String = QString;
using StringList = QStringList;
template <typename T>
using sptr = std::shared_ptr<T>;
using ContainerPtr = sptr<parser::Container>;
using Char = QChar;
using SizeType = qsizetype;
}  // namespace md
#endif  // QTMARKDOWN_MDDEF_H
