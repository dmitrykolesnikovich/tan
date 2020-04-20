#include "src/ast/ast_control_flow.h"
#include "src/ast/ast_loop.h"
#include "src/ast/ast_expr.h"
#include "src/ast/ast_import.h"
#include "src/parser/token_check.h"
#include "parser.h"

namespace tanlang {

size_t ASTIf::nud(tanlang::Parser *parser) {
  _end_index = _start_index + 1; /// skip "if"
  /// condition
  auto condition = parser->peek(_end_index, TokenType::PUNCTUATION, "(");
  _end_index = condition->parse(parser);
  _children.push_back(condition);
  /// if clause
  auto if_clause = parser->peek(_end_index, TokenType::PUNCTUATION, "{");
  _end_index = if_clause->parse(parser);
  _children.push_back(if_clause);

  /// else clause, if any
  auto *token = parser->at(_end_index);
  if (token->type == TokenType::KEYWORD && token->value == "else") {
    auto else_clause = parser->peek(_end_index);
    _end_index = else_clause->parse(parser);
    _children.push_back(else_clause);
    _has_else = true;
  }
  return _end_index;
}

size_t ASTElse::nud(tanlang::Parser *parser) {
  _end_index = _start_index + 1; /// skip "else"
  auto else_clause = parser->peek(_end_index);
  _end_index = else_clause->parse(parser);
  _children.push_back(else_clause);
  return _end_index;
}

size_t ASTStatement::nud(Parser *parser) {
  _end_index = _start_index;
  if (_is_compound) { /// compound statement
    ++_end_index; /// skip "{"
    while (!parser->eof(_end_index)) {
      auto node = parser->peek(_end_index);
      while (node) { /// stops at a terminal token
        // FIXME: call parser->parse<ASTType::STATEMENT>(_end_index, true) instead of next expression
        _children.push_back(parser->next_expression(_end_index, 0));
        node = parser->peek(_end_index);
      }
      if (parser->at(_end_index)->value == "}") {
        ++_end_index; /// skip "}"
        break;
      }
      ++_end_index;
    }
  } else { /// single statement
    auto node = parser->peek(_end_index);
    while (node) { /// stops at a terminal token
      _children.push_back(parser->next_expression(_end_index, 0));
      node = parser->peek(_end_index);
    }
    ++_end_index; /// skip terminal token
  }
  return _end_index;
}

size_t ASTLoop::nud(tanlang::Parser *parser) {
  if (parser->at(_start_index)->value == "for") {
    _loop_type = ASTLoopType::FOR;
    assert(false);
  } else if (parser->at(_start_index)->value == "while") {
    _loop_type = ASTLoopType::WHILE;
  } else {
    assert(false);
  }
  _end_index = _start_index + 1; /// skip 'while'/'for'/...
  switch (_loop_type) {
    case ASTLoopType::WHILE:
      parser->peek(_end_index, TokenType::PUNCTUATION, "(");
      _children.push_back(parser->next_expression(_end_index)); /// condition
      parser->peek(_end_index, TokenType::PUNCTUATION, "{");
      _children.push_back(parser->next_expression(_end_index)); /// loop body
      break;
    case ASTLoopType::FOR:
      assert(false);
      break;
    default:
      break;
  }
  return _end_index;
}

size_t ASTImport::nud(Parser *parser) {
  _end_index = _start_index + 1; /// skip "import"
  auto rhs = parser->peek(_end_index);
  if (rhs->_type != ASTType::STRING_LITERAL) {
    report_code_error(_token, "Invalid import statement");
  }
  _end_index = rhs->parse(parser);
  _file = ast_cast<ASTStringLiteral>(rhs)->get_string();
  return _end_index;
}

} // namespace tanlang
