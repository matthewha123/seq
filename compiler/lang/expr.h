#pragma once

#include "lang/ops.h"
#include "lang/patterns.h"
#include "types/ref.h"
#include "types/types.h"
#include "util/common.h"

namespace seq {
/**
 * Class from which all Seq expressions derive.
 *
 * An "expression" can be thought of as an AST node,
 * potentially containing sub-expressions. Every
 * expression has a type.
 */
class Expr : public SrcObject {
private:
  /// Expression type if fixed, or null if not
  types::Type *type;

  /// Enclosing try-catch, or null of none
  TryCatch *tc;

protected:
  /// Human-readable name for this expression,
  /// mainly for debugging
  std::string name;

public:
  /// Constructs an expression of fixed type.
  /// @param type fixed type of expression
  explicit Expr(types::Type *type);

  /// Constructs an expression without a fixed type.
  Expr();

  /// Sets the enclosing try-catch statement.
  void setTryCatch(TryCatch *tc);

  /// Returns the enclosing try-catch statement.
  TryCatch *getTryCatch();

  /// Delegates to \ref codegen0() "codegen0()"; catches
  /// exceptions and fills in source information.
  llvm::Value *codegen(BaseFunc *base, llvm::BasicBlock *&block);

  /// Delegates to \ref getType0() "getType0()"; catches
  /// exceptions and fills in source information.
  types::Type *getType() const;

  /// Returns the given name of this expression.
  std::string getName() const;

  /// Performs type resolution on this expression and all
  /// sub-expressions/statements/etc. This is called prior
  /// to \ref Expr::getType() "Expr::getType()".
  virtual void resolveTypes();

  /// Performs code generation for this expression.
  /// @param base the function containing this expression
  /// @param block reference to block where code should be
  ///              generated; possibly modified to point
  ///              to a new block where codegen should resume
  /// @return value representing expression result; possibly
  ///         null if type is void
  virtual llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) = 0;

  /// Determines and returns the type of this expression
  virtual types::Type *getType0() const;

  /// Ensures that this expression has the specified type.
  /// Throws an exception if this is not the case.
  /// @param type required type of this expression
  virtual void ensure(types::Type *type);

  /// Clones this expression. \p ref is used internally to
  /// keep track of cloned objects, and to make sure we
  /// don't clone certain objects twice.
  /// @param ref generic object that is being cloned
  /// @return cloned expression
  virtual Expr *clone(Generic *ref);
};

struct Const {
  enum Type { NONE, INT, FLOAT, BOOL, STR, SEQ };
  Type type;
  seq_int_t ival;
  double fval;
  bool bval;
  std::string sval;
  Const() : type(Type::NONE), ival(0), fval(0.0), bval(false), sval() {}
  explicit Const(seq_int_t ival)
      : type(Type::INT), ival(ival), fval(0.0), bval(false), sval() {}
  explicit Const(double fval)
      : type(Type::FLOAT), ival(0), fval(fval), bval(false), sval() {}
  explicit Const(bool bval)
      : type(Type::BOOL), ival(0), fval(0.0), bval(bval), sval() {}
  explicit Const(std::string sval, bool seq = false)
      : type(seq ? Type::SEQ : Type::STR), ival(0), fval(0.0), bval(false),
        sval(std::move(sval)) {}
};

class ConstExpr : public Expr {
public:
  explicit ConstExpr(types::Type *type) : Expr(type) {}
  virtual Const constValue() const = 0;
};

class TypeExpr : public Expr {
public:
  explicit TypeExpr(types::Type *type);
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
};

class ValueExpr : public Expr {
private:
  llvm::Value *val;

public:
  ValueExpr(types::Type *type, llvm::Value *val);
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
};

class NoneExpr : public ConstExpr {
public:
  NoneExpr();
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  Const constValue() const override;
};

class IntExpr : public ConstExpr {
private:
  seq_int_t n;

public:
  explicit IntExpr(seq_int_t n);
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  Const constValue() const override;
  seq_int_t value() const;
};

class FloatExpr : public ConstExpr {
private:
  double f;

public:
  explicit FloatExpr(double f);
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  Const constValue() const override;
  double value() const;
};

class BoolExpr : public ConstExpr {
private:
  bool b;

public:
  explicit BoolExpr(bool b);
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  Const constValue() const override;
  bool value() const;
};

class StrExpr : public ConstExpr {
private:
  std::string s;

public:
  explicit StrExpr(std::string s);
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  Const constValue() const override;
  std::string value() const;
};

class SeqExpr : public ConstExpr {
private:
  std::string s;

public:
  explicit SeqExpr(std::string s);
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  Const constValue() const override;
  std::string value() const;
};

class ListExpr : public Expr {
private:
  std::vector<Expr *> elems;
  types::Type *listType;

public:
  ListExpr(std::vector<Expr *> elems, types::Type *listType);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  ListExpr *clone(Generic *ref) override;
};

class SetExpr : public Expr {
private:
  std::vector<Expr *> elems;
  types::Type *setType;

public:
  SetExpr(std::vector<Expr *> elems, types::Type *setType);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  SetExpr *clone(Generic *ref) override;
};

class DictExpr : public Expr {
private:
  std::vector<Expr *> elems;
  types::Type *dictType;

public:
  DictExpr(std::vector<Expr *> elems, types::Type *dictType);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  DictExpr *clone(Generic *ref) override;
};

class For;

class ListCompExpr : public Expr {
private:
  Expr *val;
  For *body;
  types::Type *listType;
  bool realize;

public:
  ListCompExpr(Expr *val, For *body, types::Type *listType,
               bool realize = true);
  void setBody(For *body);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  ListCompExpr *clone(Generic *ref) override;
};

class SetCompExpr : public Expr {
private:
  Expr *val;
  For *body;
  types::Type *setType;
  bool realize;

public:
  SetCompExpr(Expr *val, For *body, types::Type *setType, bool realize = true);
  void setBody(For *body);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  SetCompExpr *clone(Generic *ref) override;
};

class DictCompExpr : public Expr {
private:
  Expr *key;
  Expr *val;
  For *body;
  types::Type *dictType;
  bool realize;

public:
  DictCompExpr(Expr *key, Expr *val, For *body, types::Type *dictType,
               bool realize = true);
  void setBody(For *body);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  DictCompExpr *clone(Generic *ref) override;
};

class GenExpr : public Expr {
private:
  Expr *val;
  For *body;
  std::vector<Var *> captures;

public:
  GenExpr(Expr *val, For *body, std::vector<Var *> captures);
  void setBody(For *body);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  GenExpr *clone(Generic *ref) override;
};

class VarExpr : public Expr {
private:
  Var *var;
  bool atomic;

public:
  explicit VarExpr(Var *var, bool atomic = false);
  Var *getVar();
  void setAtomic();
  Var *getVar() const;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  VarExpr *clone(Generic *ref) override;
};

class VarPtrExpr : public Expr {
private:
  Var *var;

public:
  explicit VarPtrExpr(Var *var);
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  VarPtrExpr *clone(Generic *ref) override;
};

class FuncExpr : public Expr {
private:
  BaseFunc *func;
  std::vector<types::Type *> types;
  Expr *orig; // original expression before type deduction, if any
public:
  FuncExpr(BaseFunc *func, Expr *orig, std::vector<types::Type *> types = {});
  explicit FuncExpr(BaseFunc *func, std::vector<types::Type *> types = {});
  BaseFunc *getFunc();
  std::vector<types::Type *> getTypes() const;
  bool isRealized() const;
  void setRealizeTypes(std::vector<types::Type *> types);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  Expr *clone(Generic *ref) override;
};

class ArrayExpr : public Expr {
private:
  Expr *count;
  bool doAlloca;

public:
  ArrayExpr(types::Type *type, Expr *count, bool doAlloca = false);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  ArrayExpr *clone(Generic *ref) override;
};

class RecordExpr : public Expr {
private:
  std::vector<Expr *> exprs;
  std::vector<std::string> names;

public:
  explicit RecordExpr(std::vector<Expr *> exprs,
                      std::vector<std::string> names = {});
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  RecordExpr *clone(Generic *ref) override;
};

class IsExpr : public Expr {
private:
  Expr *lhs;
  Expr *rhs;

public:
  IsExpr(Expr *lhs, Expr *rhs);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  IsExpr *clone(Generic *ref) override;
};

class UOpExpr : public Expr {
private:
  Op op;
  Expr *lhs;

public:
  UOpExpr(Op op, Expr *lhs);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  UOpExpr *clone(Generic *ref) override;
};

class BOpExpr : public Expr {
private:
  Op op;
  Expr *lhs;
  Expr *rhs;
  bool inPlace;

public:
  BOpExpr(Op op, Expr *lhs, Expr *rhs, bool inPlace = false);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  BOpExpr *clone(Generic *ref) override;
};

class AtomicExpr : public Expr {
public:
  enum Op {
    XCHG,
    ADD,
    SUB,
    AND,
    NAND,
    OR,
    XOR,
    MAX,
    MIN,
  };

private:
  AtomicExpr::Op op;
  Var *lhs;
  Expr *rhs;

public:
  AtomicExpr(AtomicExpr::Op op, Var *lhs, Expr *rhs);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  AtomicExpr *clone(Generic *ref) override;
};

class ArrayLookupExpr : public Expr {
private:
  Expr *arr;
  Expr *idx;

public:
  ArrayLookupExpr(Expr *arr, Expr *idx);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  ArrayLookupExpr *clone(Generic *ref) override;
};

class ArrayContainsExpr : public Expr {
private:
  Expr *val;
  Expr *arr;

public:
  ArrayContainsExpr(Expr *val, Expr *arr);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  ArrayContainsExpr *clone(Generic *ref) override;
};

class GetElemExpr : public Expr {
private:
  Expr *rec;
  std::string memb;
  std::vector<types::Type *> types;
  GetElemExpr *orig;

public:
  GetElemExpr(Expr *rec, std::string memb, GetElemExpr *orig,
              std::vector<types::Type *> types = {});
  GetElemExpr(Expr *rec, std::string memb,
              std::vector<types::Type *> types = {});
  GetElemExpr(Expr *rec, unsigned memb, std::vector<types::Type *> types = {});
  Expr *getRec();
  std::string getMemb();
  bool isRealized() const;
  void setRealizeTypes(std::vector<types::Type *> types);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  GetElemExpr *clone(Generic *ref) override;
};

class GetStaticElemExpr : public Expr {
private:
  types::Type *type;
  std::string memb;
  std::vector<types::Type *> types;
  GetStaticElemExpr *orig;

public:
  GetStaticElemExpr(types::Type *type, std::string memb,
                    GetStaticElemExpr *orig,
                    std::vector<types::Type *> types = {});
  GetStaticElemExpr(types::Type *type, std::string memb,
                    std::vector<types::Type *> types = {});
  types::Type *getTypeInExpr() const;
  std::string getMemb() const;
  bool isRealized() const;
  void setRealizeTypes(std::vector<types::Type *> types);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  GetStaticElemExpr *clone(Generic *ref) override;
};

class CallExpr : public Expr {
private:
  mutable Expr *func;
  std::vector<Expr *> args;
  std::vector<std::string> names;

public:
  CallExpr(Expr *func, std::vector<Expr *> args,
           std::vector<std::string> names = {});
  Expr *getFuncExpr() const;
  std::vector<Expr *> getArgs() const;
  void setFuncExpr(Expr *func);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  CallExpr *clone(Generic *ref) override;
};

class PartialCallExpr : public Expr {
private:
  mutable Expr *func;
  std::vector<Expr *> args;
  std::vector<std::string> names;

public:
  PartialCallExpr(Expr *func, std::vector<Expr *> args,
                  std::vector<std::string> names = {});
  Expr *getFuncExpr() const;
  std::vector<Expr *> getArgs() const;
  void setFuncExpr(Expr *func);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::PartialFuncType *getType0() const override;
  PartialCallExpr *clone(Generic *ref) override;
};

class CondExpr : public Expr {
private:
  Expr *cond;
  Expr *ifTrue;
  Expr *ifFalse;

public:
  CondExpr(Expr *cond, Expr *ifTrue, Expr *ifFalse);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  CondExpr *clone(Generic *ref) override;
};

class MatchExpr : public Expr {
private:
  Expr *value;
  std::vector<Pattern *> patterns;
  std::vector<Expr *> exprs;

public:
  MatchExpr();
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  MatchExpr *clone(Generic *ref) override;
  void setValue(Expr *value);
  void addCase(Pattern *pattern, Expr *expr);
};

class ConstructExpr : public Expr {
private:
  mutable types::Type *type;
  mutable types::Type *type0; // type before deduction, saved for clone
  std::vector<Expr *> args;
  std::vector<std::string> names;

public:
  ConstructExpr(types::Type *type, std::vector<Expr *> args,
                std::vector<std::string> names = {});
  types::Type *getConstructType();
  std::vector<Expr *> getArgs();
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  ConstructExpr *clone(Generic *ref) override;
};

class MethodExpr : public Expr {
private:
  Expr *self;
  Func *func;

public:
  MethodExpr(Expr *self, Func *method);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::MethodType *getType0() const override;
  MethodExpr *clone(Generic *ref) override;
};

class OptExpr : public Expr {
private:
  Expr *val;

public:
  explicit OptExpr(Expr *val);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  OptExpr *clone(Generic *ref) override;
};

class YieldExpr : public Expr {
private:
  BaseFunc *base;

public:
  YieldExpr(BaseFunc *base);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  types::Type *getType0() const override;
  YieldExpr *clone(Generic *ref) override;
};

class DefaultExpr : public Expr {
public:
  explicit DefaultExpr(types::Type *type);
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  DefaultExpr *clone(Generic *ref) override;
};

class TypeOfExpr : public Expr {
private:
  Expr *val;

public:
  explicit TypeOfExpr(Expr *val);
  void resolveTypes() override;
  llvm::Value *codegen0(BaseFunc *base, llvm::BasicBlock *&block) override;
  TypeOfExpr *clone(Generic *ref) override;
};

} // namespace seq
