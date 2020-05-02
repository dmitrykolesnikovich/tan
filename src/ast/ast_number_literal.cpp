#include "src/ast/ast_number_literal.h"
#include "src/ast/ast_ty.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

Value *ASTNumberLiteral::codegen(CompilerSession *compiler_session) {
  if (_is_float) {
    _llvm_value = ConstantFP::get(*compiler_session->get_context(), APFloat(_fvalue));
  } else {
    _llvm_value = ConstantInt::get(*compiler_session->get_context(), APInt(32, static_cast<uint64_t>(_ivalue), true));
  }
  return _llvm_value;
}

size_t ASTNumberLiteral::nud() {
  _end_index = _start_index + 1;
  return _end_index;
}

bool tanlang::ASTNumberLiteral::is_float() const { return _is_float; }

ASTNumberLiteral::ASTNumberLiteral(const std::string &str, bool is_float, Token *token, size_t token_index)
    : ASTLiteral(ASTType::NUM_LITERAL, op_precedence[ASTType::NUM_LITERAL], 0, token, token_index) {
  _is_float = is_float;
  if (is_float) { _fvalue = std::stof(str); }
  else { _ivalue = std::stoi(str); }
  _is_unsigned = token->is_unsigned;
}

llvm::Value *ASTNumberLiteral::get_llvm_value(CompilerSession *) const { return _llvm_value; }

std::string ASTNumberLiteral::get_type_name() const {
  if (_is_float) { return "float"; }
  else if (_is_unsigned) { return "u32"; }
  else { return "i32"; }
}

llvm::Type *ASTNumberLiteral::to_llvm_type(CompilerSession *compiler_session) const {
  if (_is_float) {
    return compiler_session->get_builder()->getFloatTy();
  } else {
    return compiler_session->get_builder()->getInt32Ty();
  }
}

std::string ASTNumberLiteral::to_string(bool print_prefix) const {
  std::string ret = "";
  if (print_prefix) {
    ret += ASTLiteral::to_string(print_prefix) + " ";
  }
  if (_is_float) { ret += std::to_string(_fvalue); } else { ret += std::to_string(_ivalue); }
  return ret;
}

ASTNumberLiteral::ASTNumberLiteral(int value, size_t token_index, bool is_unsigned) : ASTLiteral(ASTType::NUM_LITERAL,
    op_precedence[ASTType::NUM_LITERAL],
    0,
    nullptr,
    token_index) {
  _ivalue = value;
  _is_float = false;
  _is_unsigned = is_unsigned;
}

ASTNumberLiteral::ASTNumberLiteral(size_t value, size_t token_index, bool is_unsigned)
    : ASTLiteral(ASTType::NUM_LITERAL, op_precedence[ASTType::NUM_LITERAL], 0, nullptr, token_index) {
  _ivalue = static_cast<int>(value);
  _is_float = false;
  _is_unsigned = is_unsigned;
}

ASTNumberLiteral::ASTNumberLiteral(float value, size_t token_index) : ASTLiteral(ASTType::NUM_LITERAL,
    op_precedence[ASTType::NUM_LITERAL],
    0,
    nullptr,
    token_index) {
  _fvalue = value;
  _is_float = true;
}

std::shared_ptr<ASTTy> tanlang::ASTNumberLiteral::get_ty() const {
  Ty t = Ty::INVALID;
  std::shared_ptr<ASTTy> ret = nullptr;
  if (_is_float) {
    t = Ty::FLOAT;
    ret = ASTTy::Create(t);
    ret->_default_value = _fvalue;
  } else if (_is_unsigned) {
    t = TY_OR3(Ty::INT, Ty::BIT32, Ty::UNSIGNED);
    ret = ASTTy::Create(t);
    ret->_default_value = static_cast<uint64_t>(_ivalue);
  } else {
    t = TY_OR(Ty::INT, Ty::BIT32);
    ret = ASTTy::Create(t);
    ret->_default_value = static_cast<uint64_t>(_ivalue);
  }
  return ret;
}

} // namespace tanlang
