//
// Created by PikachuHy on 2021/11/3.
//

#ifndef QTMARKDOWN_MDDEF_H
#define QTMARKDOWN_MDDEF_H
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "MdString.h"
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
// StringList uses std::vector<String> for UTF-8 compatibility.
using TokenList = std::vector<parser::Token>;
using StringList = std::vector<String>;
template <typename T>
using sptr = std::shared_ptr<T>;
using ContainerPtr = sptr<parser::Container>;
using Char = char;
using SizeType = int64_t;
}  // namespace md
#endif  // QTMARKDOWN_MDDEF_H
