#include "src/ast/ast_statement.h"
#include "parser.h"

namespace tanlang {

ASTStatement::ASTStatement(bool is_compound, Token *token, size_t token_index) : ASTNode(ASTType::STATEMENT,
    op_precedence[ASTType::STATEMENT],
    0,
    token,
    token_index) {
  _is_compound = is_compound;
}

ASTStatement::ASTStatement(Token *token, size_t token_index) : ASTNode(ASTType::STATEMENT,
    op_precedence[ASTType::STATEMENT],
    0,
    token,
    token_index) {}

size_t ASTStatement::nud(Parser *parser) {
  _end_index = _start_index;
  if (_is_compound) { /// compound statement
    ++_end_index; /// skip "{"
    while (!parser->eof(_end_index)) {
      auto node = parser->peek(_end_index);
      while (node) { /// stops at a terminal token
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

} // namespace tanlang
