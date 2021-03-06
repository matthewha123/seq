#include "util/fmt/format.h"
#include "util/fmt/ostream.h"
#include <memory>
#include <ostream>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "lang/seq.h"
#include "parser/ast/codegen/pattern.h"
#include "parser/ast/expr.h"
#include "parser/ast/stmt.h"
#include "parser/ast/visitor.h"
#include "parser/common.h"
#include "parser/context.h"

using fmt::format;
using std::get;
using std::make_unique;
using std::move;
using std::ostream;
using std::stack;
using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::unordered_set;
using std::vector;

#define RETURN(T, ...)                                                         \
  this->result = new T(__VA_ARGS__);                                           \
  return

namespace seq {
namespace ast {

CodegenPatternVisitor::CodegenPatternVisitor(CodegenStmtVisitor &stmtVisitor)
    : stmtVisitor(stmtVisitor), result(nullptr) {}

seq::Pattern *CodegenPatternVisitor::transform(const PatternPtr &ptr) {
  CodegenPatternVisitor v(stmtVisitor);
  ptr->accept(v);
  if (v.result) {
    v.result->setSrcInfo(ptr->getSrcInfo());
    if (auto t = stmtVisitor.getContext().getTryCatch()) {
      v.result->setTryCatch(t);
    }
  }
  return v.result;
}

void CodegenPatternVisitor::visit(const StarPattern *pat) {
  RETURN(seq::StarPattern, );
}

void CodegenPatternVisitor::visit(const IntPattern *pat) {
  RETURN(seq::IntPattern, pat->value);
}

void CodegenPatternVisitor::visit(const BoolPattern *pat) {
  RETURN(seq::BoolPattern, pat->value);
}

void CodegenPatternVisitor::visit(const StrPattern *pat) {
  RETURN(seq::StrPattern, pat->value);
}

void CodegenPatternVisitor::visit(const SeqPattern *pat) {
  RETURN(seq::SeqPattern, pat->value);
}

void CodegenPatternVisitor::visit(const RangePattern *pat) {
  RETURN(seq::RangePattern, pat->start, pat->end);
}

void CodegenPatternVisitor::visit(const TuplePattern *pat) {
  vector<seq::Pattern *> result;
  for (auto &p : pat->patterns) {
    result.push_back(transform(p));
  }
  RETURN(seq::RecordPattern, result);
}

void CodegenPatternVisitor::visit(const ListPattern *pat) {
  vector<seq::Pattern *> result;
  for (auto &p : pat->patterns) {
    result.push_back(transform(p));
  }
  RETURN(seq::ArrayPattern, result);
}

void CodegenPatternVisitor::visit(const OrPattern *pat) {
  vector<seq::Pattern *> result;
  for (auto &p : pat->patterns) {
    result.push_back(transform(p));
  }
  RETURN(seq::OrPattern, result);
}

void CodegenPatternVisitor::visit(const WildcardPattern *pat) {
  auto p = new seq::Wildcard();
  if (pat->var.size()) {
    stmtVisitor.getContext().add(pat->var, p->getVar());
  }
  this->result = p;
}

void CodegenPatternVisitor::visit(const GuardedPattern *pat) {
  RETURN(seq::GuardedPattern, transform(pat->pattern),
         stmtVisitor.transform(pat->cond));
}

void CodegenPatternVisitor::visit(const BoundPattern *pat) {
  error(pat->getSrcInfo(), "unexpected bound pattern");
}

} // namespace ast
} // namespace seq
