#ifndef __TAN_SRC_AST_AST_COMPARE_H__
#define __TAN_SRC_AST_AST_COMPARE_H__
#include "src/ast/ast_infix_binary_op.h"

namespace tanlang {

class ASTCompare final : public ASTInfixBinaryOp {
public:
  ASTCompare() = delete;
  ASTCompare(ASTType type, Token *token, size_t token_index);

protected:
  llvm::Value *_codegen(CompilerSession *) override;
  size_t led(const ASTNodePtr &left) override;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_COMPARE_H__ */
