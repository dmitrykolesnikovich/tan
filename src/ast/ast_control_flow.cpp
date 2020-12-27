#include "src/ast/ast_ty.h"
#include "src/ast/ast_loop.h"
#include "compiler_session.h"
#include "parser.h"
#include "token.h"

namespace tanlang {

Value *ASTElse::_codegen(CompilerSession *cs) {
  cs->set_current_debug_location(_token->l, _token->c);
  return _children[0]->codegen(cs);
}

llvm::Value *ASTBreakContinue::_codegen(CompilerSession *cs) {
  auto *builder = cs->_builder;
  auto loop = cs->get_current_loop();
  if (!loop) { error("Any break/continue statement must be inside loop"); }
  auto s = loop->get_loop_start();
  auto e = loop->get_loop_end();
  if (_type == ASTType::BREAK) {
    builder->CreateBr(e);
  } else if (_type == ASTType::CONTINUE) {
    builder->CreateBr(s);
  } else { TAN_ASSERT(false); }
  return nullptr;
}

size_t ASTElse::nud() {
  _end_index = _start_index + 1; /// skip "else"
  auto else_clause = _parser->peek(_end_index);
  _end_index = else_clause->parse(_parser, _cs);
  _children.push_back(else_clause);
  return _end_index;
}

size_t ASTBreakContinue::nud() {
  _end_index = _start_index + 1;
  return _end_index;
}

ASTElse::ASTElse(Token *t, size_t ti) : ASTNode(ASTType::ELSE, op_precedence[ASTType::ELSE], 0, t, ti) {}

ASTBreakContinue::ASTBreakContinue(Token *t, size_t ti) : ASTNode(ASTType::INVALID, 0, 0, t, ti) {
  if (t->value == "break") { _type = ASTType::BREAK; }
  else if (t->value == "continue") { _type = ASTType::CONTINUE; }
}

} // namespace tanlang

