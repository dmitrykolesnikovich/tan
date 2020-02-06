#ifndef TAN_SRC_AST_AST_STATEMENT_H_
#define TAN_SRC_AST_AST_STATEMENT_H_
#include "astnode.h"

namespace tanlang {

class ASTProgram final : public ASTNode {
 public:
  ASTProgram();
  Value *codegen(ParserContext *parser_context) override;
  void nud(Parser *parser) override;
};

class ASTStatement final : public ASTNode {
 public:
  ASTStatement() = delete;
  explicit ASTStatement(Token *token);
  ASTStatement(bool is_compound, Token *token);
  void nud(Parser *parser) override;
 public:
  bool _is_compound = false;
};

}

#endif /* TAN_SRC_AST_AST_STATEMENT_H_ */