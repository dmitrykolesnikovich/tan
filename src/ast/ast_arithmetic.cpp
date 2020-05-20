#include "src/ast/ast_ty.h"
#include "src/common.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

Value *ASTArithmetic::_codegen(CompilerSession *cs) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(_token->l, _token->c);
  /// unary plus/minus
  if (_children.size() == 1) {
    if (!is_ast_type_in(_type, {ASTType::SUM, ASTType::SUBTRACT})) {
      error("Invalid unary operation");
    }
    if (_type == ASTType::SUM) {
      return _children[0]->codegen(cs);
    } else {
      auto *r = _children[0]->codegen(cs);
      if (_children[0]->is_lvalue()) { r = builder->CreateLoad(r); }
      if (r->getType()->isFloatingPointTy()) { return builder->CreateFNeg(r); }
      return builder->CreateNeg(r);
    }
  }

  /// binary operator
  auto lhs = _children[0];
  auto rhs = _children[1];
  Value *l = _children[0]->codegen(cs);
  Value *r = _children[1]->codegen(cs);
  TAN_ASSERT(l && r);
  TAN_ASSERT(_children.size() > _dominant_idx);

  if (_dominant_idx == 0) {
    r = TypeSystem::ConvertTo(cs, r, rhs->get_ty(), lhs->get_ty());
    l = TypeSystem::ConvertTo(cs, l, lhs->get_ty(), lhs->get_ty());
  } else {
    l = TypeSystem::ConvertTo(cs, l, lhs->get_ty(), rhs->get_ty());
    r = TypeSystem::ConvertTo(cs, r, rhs->get_ty(), rhs->get_ty());
  }

  if (l->getType()->isFloatingPointTy()) {
    /// float arithmetic
    if (_type == ASTType::MULTIPLY) {
      _llvm_value = builder->CreateFMul(l, r, "mul_tmp");
    } else if (_type == ASTType::DIVIDE) {
      _llvm_value = builder->CreateFDiv(l, r, "div_tmp");
    } else if (_type == ASTType::SUM) {
      _llvm_value = builder->CreateFAdd(l, r, "sum_tmp");
    } else if (_type == ASTType::SUBTRACT) {
      _llvm_value = builder->CreateFSub(l, r, "sub_tmp");
    } else if (_type == ASTType::MOD) {
      _llvm_value = builder->CreateFRem(l, r, "mod_tmp");
    } else { TAN_ASSERT(false); }
  } else {
    /// integer arithmetic
    if (_type == ASTType::MULTIPLY) {
      _llvm_value = builder->CreateMul(l, r, "mul_tmp");
    } else if (_type == ASTType::DIVIDE) {
      auto ty = _children[0]->get_ty();
      if (ty->is_unsigned()) { _llvm_value = builder->CreateUDiv(l, r, "div_tmp"); }
      else { _llvm_value = builder->CreateSDiv(l, r, "div_tmp"); }
    } else if (_type == ASTType::SUM) {
      _llvm_value = builder->CreateAdd(l, r, "sum_tmp");
    } else if (_type == ASTType::SUBTRACT) {
      _llvm_value = builder->CreateSub(l, r, "sub_tmp");
    } else if (_type == ASTType::MOD) {
      auto ty = _children[0]->get_ty();
      if (ty->is_unsigned()) { _llvm_value = builder->CreateURem(l, r, "mod_tmp"); }
      else { _llvm_value = builder->CreateSRem(l, r, "mod_tmp"); }
    } else { TAN_ASSERT(false); }
  }
  return _llvm_value;
}

} // namespace tanlang
