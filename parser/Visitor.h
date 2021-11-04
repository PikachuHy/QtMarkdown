//
// Created by pikachu on 2021/2/1.
//

#ifndef MD_VISITOR_H
#define MD_VISITOR_H
#include "QtMarkdown_global.h"
namespace md::parser {
template <typename T>
struct Visitor {
  virtual void visit(T*) = 0;
};

struct VisitorNode {
  virtual ~VisitorNode() = default;
};

template <class... T>
struct MultipleVisitor : public VisitorNode, public Visitor<T>... {};
}  // namespace md::parser
#endif  // MD_VISITOR_H
