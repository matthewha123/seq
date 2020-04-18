#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "parser/ast/ast.h"
#include "parser/ast/visitor.h"
#include "parser/common.h"
#include "parser/context.h"

namespace seq {
namespace ast {

class TransformStmtVisitor;

class TransformExprVisitor : public ExprVisitor {
  ExprPtr result{nullptr};
  std::vector<StmtPtr> &prependStmts;
  friend class TransformStmtVisitor;

public:
  TransformExprVisitor(std::vector<StmtPtr> &prepend);
  ExprPtr transform(const Expr *e);
  std::vector<ExprPtr> transform(const std::vector<ExprPtr> &e);

  template <typename T>
  auto transform(const std::unique_ptr<T> &t) -> decltype(transform(t.get())) {
    return transform(t.get());
  }

  void visit(const EmptyExpr *) override;
  void visit(const BoolExpr *) override;
  void visit(const IntExpr *) override;
  void visit(const FloatExpr *) override;
  void visit(const StringExpr *) override;
  void visit(const FStringExpr *) override;
  void visit(const KmerExpr *) override;
  void visit(const SeqExpr *) override;
  void visit(const IdExpr *) override;
  void visit(const UnpackExpr *) override;
  void visit(const TupleExpr *) override;
  void visit(const ListExpr *) override;
  void visit(const SetExpr *) override;
  void visit(const DictExpr *) override;
  void visit(const GeneratorExpr *) override;
  void visit(const DictGeneratorExpr *) override;
  void visit(const IfExpr *) override;
  void visit(const UnaryExpr *) override;
  void visit(const BinaryExpr *) override;
  void visit(const PipeExpr *) override;
  void visit(const IndexExpr *) override;
  void visit(const CallExpr *) override;
  void visit(const DotExpr *) override;
  void visit(const SliceExpr *) override;
  void visit(const EllipsisExpr *) override;
  void visit(const TypeOfExpr *) override;
  void visit(const PtrExpr *) override;
  void visit(const LambdaExpr *) override;
  void visit(const YieldExpr *) override;
};

class TransformStmtVisitor : public StmtVisitor {
  std::vector<StmtPtr> prependStmts;
  StmtPtr result{nullptr};

  StmtPtr addAssignment(const Expr *lhs, const Expr *rhs,
                        const Expr *type = nullptr, bool force = false);
  void processAssignment(const Expr *lhs, const Expr *rhs,
                         std::vector<StmtPtr> &stmts, bool force = false);

public:
  void prepend(StmtPtr s);

  StmtPtr transform(const Stmt *stmt);
  ExprPtr transform(const Expr *stmt);
  PatternPtr transform(const Pattern *stmt);

  template <typename T>
  auto transform(const std::unique_ptr<T> &t) -> decltype(transform(t.get())) {
    return transform(t.get());
  }

  virtual void visit(const SuiteStmt *) override;
  virtual void visit(const PassStmt *) override;
  virtual void visit(const BreakStmt *) override;
  virtual void visit(const ContinueStmt *) override;
  virtual void visit(const ExprStmt *) override;
  virtual void visit(const AssignStmt *) override;
  virtual void visit(const DelStmt *) override;
  virtual void visit(const PrintStmt *) override;
  virtual void visit(const ReturnStmt *) override;
  virtual void visit(const YieldStmt *) override;
  virtual void visit(const AssertStmt *) override;
  virtual void visit(const TypeAliasStmt *) override;
  virtual void visit(const WhileStmt *) override;
  virtual void visit(const ForStmt *) override;
  virtual void visit(const IfStmt *) override;
  virtual void visit(const MatchStmt *) override;
  virtual void visit(const ExtendStmt *) override;
  virtual void visit(const ImportStmt *) override;
  virtual void visit(const ExternImportStmt *) override;
  virtual void visit(const TryStmt *) override;
  virtual void visit(const GlobalStmt *) override;
  virtual void visit(const ThrowStmt *) override;
  virtual void visit(const FunctionStmt *) override;
  virtual void visit(const ClassStmt *) override;
  virtual void visit(const DeclareStmt *) override;
  virtual void visit(const AssignEqStmt *) override;
  virtual void visit(const YieldFromStmt *) override;
  virtual void visit(const WithStmt *) override;
  virtual void visit(const PyDefStmt *) override;
};

class TransformPatternVisitor : public PatternVisitor {
  TransformStmtVisitor &stmtVisitor;
  PatternPtr result;
  friend TransformStmtVisitor;

public:
  TransformPatternVisitor(TransformStmtVisitor &);
  PatternPtr transform(const Pattern *ptr);
  std::vector<PatternPtr> transform(const std::vector<PatternPtr> &pats);

  template <typename T>
  auto transform(const std::unique_ptr<T> &t) -> decltype(transform(t.get())) {
    return transform(t.get());
  }

  void visit(const StarPattern *) override;
  void visit(const IntPattern *) override;
  void visit(const BoolPattern *) override;
  void visit(const StrPattern *) override;
  void visit(const SeqPattern *) override;
  void visit(const RangePattern *) override;
  void visit(const TuplePattern *) override;
  void visit(const ListPattern *) override;
  void visit(const OrPattern *) override;
  void visit(const WildcardPattern *) override;
  void visit(const GuardedPattern *) override;
  void visit(const BoundPattern *) override;
};

} // namespace ast
} // namespace seq
