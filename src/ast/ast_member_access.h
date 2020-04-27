#ifndef __TAN_SRC_AST_AST_MEMBER_ACCESS_H__
#define __TAN_SRC_AST_AST_MEMBER_ACCESS_H__
#include "src/ast/astnode.h"

namespace tanlang {

class ASTMemberAccess final : public ASTNode {
public:
  ASTMemberAccess() = delete;
  ASTMemberAccess(Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;
  bool is_lvalue() const override;
  llvm::Value *get_llvm_value(CompilerSession *) const override;
  bool is_typed() const override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;
  std::string get_type_name() const override;
  std::shared_ptr<ASTTy> get_ty() const override;

protected:
  size_t led(const ASTNodePtr &left, Parser *parser) override;
  Value *codegen_dot_member_variable(CompilerSession *compiler_session, ASTNodePtr lhs, ASTNodePtr rhs);

private:
  bool _is_bracket = false;
  Type *_llvm_type = nullptr;
  Value *_llvm_value = nullptr;
  std::shared_ptr<ASTTy> _ty = nullptr;
  std::string _type_name = "";
};

}

#endif /*__TAN_SRC_AST_AST_MEMBER_ACCESS_H__*/
