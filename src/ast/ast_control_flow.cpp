#include "src/ast/ast_control_flow.h"
#include "parser.h"
#include "token.h"
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Function.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>

namespace tanlang {
using llvm::ConstantFP;
using llvm::ConstantInt;
using llvm::APFloat;
using llvm::APInt;
using llvm::Function;
using llvm::Type;
using llvm::PHINode;

// =================== if ===================//
void ASTIf::nud(tanlang::Parser *parser) {
  // condition
  auto condition = parser->advance(TokenType::PUNCTUATION, "(");
  condition->nud(parser);
  _children.push_back(condition);
  // if clause
  auto if_clause = parser->advance(TokenType::PUNCTUATION, "{");
  if_clause->nud(parser);
  _children.push_back(if_clause);

  // else clause
  Token *token = parser->get_curr_token();
  if (token->type == TokenType::KEYWORD && token->value == "else") {
    auto else_clause = parser->advance();
    else_clause->nud(parser);
    _children.push_back(else_clause); // else clause
    _has_else = true;
  }
}

Value *ASTIf::codegen(ParserContext *parser_context) {
  Value *condition = _children[0]->codegen(parser_context);
  if (!condition) {
    // TODO: report error
    return nullptr;
  }

  // convert condition to a bool by comparing non-equal to 0
  /* condition = parser_context->_builder->CreateICmpEQ(condition,
                                                     ConstantInt::get(*parser_context->_context, APInt(32, 0)),
                                                     "if");
                                                     */
  Function *func = parser_context->_builder->GetInsertBlock()->getParent();

  // Create blocks for the then (and else) clause
  // Insert the 'then' block at the end of the function.
  BasicBlock *then_bb = BasicBlock::Create(*parser_context->_context, "then", func);
  BasicBlock *else_bb = BasicBlock::Create(*parser_context->_context, "else");
  BasicBlock *merge_bb = BasicBlock::Create(*parser_context->_context, "fi");

  parser_context->_builder->CreateCondBr(condition, then_bb, else_bb);
  // Emit then value
  parser_context->_builder->SetInsertPoint(then_bb);
  Value *then = _children[1]->codegen(parser_context);
  if (!then)
    throw "SHIT";
  parser_context->_builder->CreateBr(merge_bb);
  // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
  then_bb = parser_context->_builder->GetInsertBlock();

  Value *else_ = nullptr;
  if (_has_else) {
    // Emit else block
    func->getBasicBlockList().push_back(else_bb);
    parser_context->_builder->SetInsertPoint(else_bb);

    else_ = _children[2]->codegen(parser_context);
    if (!else_)
      // TODO: report error
      throw "SHIT";

    parser_context->_builder->CreateBr(merge_bb);
    // codegen of 'Else' can change the current block, update ElseBB for the PHI.
    else_bb = parser_context->_builder->GetInsertBlock();
  }
  // Emit merge block
  func->getBasicBlockList().push_back(merge_bb);
  parser_context->_builder->SetInsertPoint(merge_bb);
  return nullptr;
}
// ============================================//

// =================== else ===================//
void ASTElse::nud(tanlang::Parser *parser) {
  auto else_clause = parser->advance(TokenType::PUNCTUATION, "{");
  else_clause->nud(parser);
  _children.push_back(else_clause);
}
// ============================================//
}

