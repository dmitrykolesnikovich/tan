#include "src/ast/ast_array.h"
#include "src/ast/common.h"

namespace tanlang {

Value *ASTArrayLiteral::codegen(CompilerSession *compiler_session) {
  using llvm::Constant;
  auto sub = ast_cast<ASTLiteral>(_children[0]);
  sub->codegen(compiler_session);
  auto e_type = sub->to_llvm_type(compiler_session);
  std::vector<Constant *> constants;
  size_t n = _children.size();
  constants.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    constants.push_back((Constant *) _children[i]->codegen(compiler_session));
  }
  auto ret = llvm::ConstantArray::get((llvm::ArrayType *) e_type, constants);
  _llvm_type = ret->getType(); // crucial
  return ret;
}

llvm::Value *ASTArrayLiteral::get_llvm_value(CompilerSession *) const {
  return _llvm_value;
}

std::string ASTArrayLiteral::get_type_name() const {
  // TODO: add element type name
  return "array";
}

llvm::Type *ASTArrayLiteral::to_llvm_type(CompilerSession *) const {
  return _llvm_type;
}

} // namespace tanlang