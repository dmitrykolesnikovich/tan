#include "src/analysis/analysis.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_ty.h"
#include "compiler_session.h"
#include "compiler.h"
#include "token.h"

namespace tanlang {

static void error(CompilerSession *cs, const str &error_message);

ASTNodePtr get_id_referred(CompilerSession *cs, ASTNodePtr p) { return cs->get(p->_name); }

/// \section General

str get_name(ASTNodePtr p) { return p->_name; }

size_t get_n_children(ASTNodePtr p) { return p->_children.size(); }

/// \section Factory

ASTNodePtr ast_create_string_literal(CompilerSession *cs) {
  auto ret = make_ptr<ASTNode>(ASTType::STRING_LITERAL, op_precedence[ASTType::STRING_LITERAL], 0);
  ret->_is_valued = true;
  ret->_is_typed = true;
  ret->_ty = create_ty(cs, Ty::STRING);
  ret->_ty->_is_lvalue = true;
  return ret;
}

ASTNodePtr ast_create_string_literal(CompilerSession *cs, const str &s) {
  auto ret = ast_create_string_literal(cs);
  ret->_value = s;
  return ret;
}

ASTNodePtr ast_create_array_literal(CompilerSession *cs) {
  auto ret = make_ptr<ASTNode>(ASTType::ARRAY_LITERAL, 0, 0);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_numeric_literal(CompilerSession *cs) {
  auto ret = make_ptr<ASTNode>(ASTType::NUM_LITERAL, 0, 0);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_var_decl(CompilerSession *cs) {
  auto ret = make_ptr<ASTNode>(ASTType::ARG_DECL, 0, 0);
  ret->_is_typed = true;
  ret->_is_valued = true;
  ret->_is_named = true;
  return ret;
}

ASTNodePtr ast_create_var_decl(CompilerSession *cs, const str &name, ASTTyPtr ty) {
  auto ret = ast_create_arg_decl();
  ret->_ty = make_ptr<ASTTy>(*ty);
  ret->_ty->_is_lvalue = true;
  ret->_name = name;
  return ret;
}

ASTNodePtr ast_create_arg_decl(CompilerSession *cs) {
  auto ret = make_ptr<ASTNode>(ASTType::ARG_DECL, 0, 0);
  ret->_is_named = true;
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_arg_decl(CompilerSession *cs, const str &name, ASTTyPtr ty) {
  auto ret = ast_create_arg_decl();
  ret->_ty = make_ptr<ASTTy>(*ty);
  ret->_ty->_is_lvalue = true;
  ret->_name = name;
  return ret;
}

ASTNodePtr ast_create_arithmetic(CompilerSession *cs, const str &op) {
  auto ret = make_ptr<ASTNode>(ASTType::INVALID, 0, 0);
  switch (hashed_string{op.c_str()}) {
    case "+"_hs:
      ret->_type = ASTType::SUM;
      break;
    case "-"_hs:
      ret->_type = ASTType::SUBTRACT;
      break;
    case "*"_hs:
      ret->_type = ASTType::MULTIPLY;
      break;
    case "/"_hs:
      ret->_type = ASTType::DIVIDE;
      break;
    case "%"_hs:
      ret->_type = ASTType::MOD;
      break;
    default:
      return nullptr;
  }
  ret->_lbp = op_precedence[ret->_type];
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_comparison(CompilerSession *cs, const str &op) {
  auto ret = make_ptr<ASTNode>(ASTType::INVALID, 0, 0);
  switch (hashed_string{op.c_str()}) {
    case ">"_hs:
      ret->_type = ASTType::GT;
      break;
    case ">="_hs:
      ret->_type = ASTType::GE;
      break;
    case "<"_hs:
      ret->_type = ASTType::LT;
      break;
    case "<="_hs:
      ret->_type = ASTType::LE;
      break;
    case "=="_hs:
      ret->_type = ASTType::EQ;
      break;
    case "!="_hs:
      ret->_type = ASTType::NE;
      break;
    default:
      return nullptr;
  }
  ret->_lbp = op_precedence[ret->_type];
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_assignment(CompilerSession *cs) {
  auto ret = make_ptr<ASTNode>(ASTType::ASSIGN, op_precedence[ASTType::ASSIGN], 0);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_program(CompilerSession *cs) {
  auto ret = make_ptr<ASTNode>(ASTType::PROGRAM, 0, 0);
  return ret;
}

ASTNodePtr ast_create_statement(CompilerSession *cs) { return make_ptr<ASTNode>(ASTType::STATEMENT, 0, 0); }

ASTNodePtr ast_create_identifier(CompilerSession *cs) {
  auto ret = make_ptr<ASTNode>(ASTType::ID, 0, 0);
  ret->_is_named = true;
  return ret;
}

ASTNodePtr ast_create_identifier(CompilerSession *cs, const str &name) {
  auto ret = ast_create_identifier(cs);
  ret->_name = name;
  return ret;
}

ASTTyPtr ast_create_ty(CompilerSession *cs) {
  auto ret = make_ptr<ASTTy>();
  ret->_is_typed = true;
  ret->_is_valued = true; /// every type has its default value
  ret->_ty = ret;
  return ret;
}

/// \section Types

str get_type_name(ASTNodePtr p) { return p->_ty->_type_name; }

ASTTyPtr create_ty(CompilerSession *cs, Ty t, vector<ASTNodePtr> sub_tys, bool is_lvalue) {
  // TODO: cache
  auto ret = make_ptr<ASTTy>();
  ret->_tyty = t;
  ret->_is_lvalue = is_lvalue;
  ret->_children.insert(ret->_children.begin(), sub_tys.begin(), sub_tys.end());
  resolve_ty(cs, ret);
  return ret;
}

void resolve_ty(CompilerSession *cs, ASTTyPtr p) {
  Ty base = TY_GET_BASE(p->_tyty);
  Ty qual = TY_GET_QUALIFIER(p->_tyty);
  if (p->_resolved) {
    if (base == Ty::STRUCT) {
      if (!p->_is_forward_decl) { return; }
    } else { return; }
  }
  p->_ty = p;
  /// resolve_ty children if they are ASTTy
  for (auto c: p->_children) {
    auto t = ast_cast<ASTTy>(c);
    if (t && t->_type == ASTType::TY && !t->_resolved) { resolve_ty(cs, t); }
  }
  // FIXME: can't use p->_cs here, cuz some ty are created by create_ty() without its _cs being set
  auto *tm = Compiler::GetDefaultTargetMachine();
  switch (base) {
    case Ty::INT: {
      p->_size_bits = 32;
      p->_type_name = "i32";
      p->_is_int = true;
      p->_default_value.emplace<uint64_t>(0);
      if (TY_IS(qual, Ty::BIT8)) {
        p->_size_bits = 8;
        p->_type_name = "i8";
      } else if (TY_IS(qual, Ty::BIT16)) {
        p->_size_bits = 16;
        p->_type_name = "i16";
      } else if (TY_IS(qual, Ty::BIT64)) {
        p->_size_bits = 64;
        p->_type_name = "i64";
      }
      if (TY_IS(qual, Ty::UNSIGNED)) {
        p->_is_unsigned = true;
        if (p->_size_bits == 8) {
          p->_dwarf_encoding = llvm::dwarf::DW_ATE_unsigned_char;
        } else {
          p->_dwarf_encoding = llvm::dwarf::DW_ATE_unsigned;
        }
      } else {
        if (p->_size_bits == 8) {
          p->_dwarf_encoding = llvm::dwarf::DW_ATE_signed_char;
        } else {
          p->_dwarf_encoding = llvm::dwarf::DW_ATE_signed;
        }
      }
      break;
    }
    case Ty::CHAR:
      p->_type_name = "char";
      p->_size_bits = 8;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_unsigned_char;
      p->_is_unsigned = true;
      p->_default_value.emplace<uint64_t>(0);
      p->_is_int = true;
      break;
    case Ty::BOOL:
      p->_type_name = "bool";
      p->_size_bits = 1;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_boolean;
      p->_default_value.emplace<uint64_t>(0);
      p->_is_bool = true;
      break;
    case Ty::FLOAT:
      p->_type_name = "float";
      p->_size_bits = 32;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_float;
      p->_default_value.emplace<float>(0);
      p->_is_float = true;
      break;
    case Ty::DOUBLE:
      p->_type_name = "double";
      p->_size_bits = 64;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_float;
      p->_default_value.emplace<double>(0);
      p->_is_double = true;
      break;
    case Ty::STRING:
      p->_type_name = "u8*";
      p->_size_bits = tm->getPointerSizeInBits(0);
      p->_default_value.emplace<str>("");
      p->_align_bits = 8;
      p->_is_ptr = true;
      break;
    case Ty::VOID:
      p->_type_name = "void";
      p->_size_bits = 0;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_signed;
      break;
    case Ty::ENUM: {
      auto sub = ast_cast<ASTTy>(p->_children[0]);
      TAN_ASSERT(sub);
      p->_size_bits = sub->_size_bits;
      p->_align_bits = sub->_align_bits;
      p->_dwarf_encoding = sub->_dwarf_encoding;
      p->_default_value = sub->_default_value;
      p->_is_unsigned = sub->_is_unsigned;
      p->_is_int = sub->_is_int;
      p->_is_enum = true;
      /// _type_name, however, is set by ASTEnum::nud
      break;
    }
    case Ty::STRUCT: {
      /// align size is the max element size, if no element, 8 bits
      /// size is the number of elements * align size
      if (p->_is_forward_decl) {
        auto real = ast_cast<ASTTy>(cs->get(p->_type_name));
        if (!real) { error(cs, "Incomplete type"); }
        *p = *real;
        p->_is_forward_decl = false;
      } else {
        p->_align_bits = 8;
        size_t n = p->_children.size();
        for (size_t i = 0; i < n; ++i) {
          auto et = ast_cast<ASTTy>(p->_children[i]);
          auto s = get_size_bits(cs, et);
          if (s > p->_align_bits) { p->_align_bits = s; }
        }
        p->_size_bits = n * p->_align_bits;
        p->_is_struct = true;
      }
      break;
    }
    case Ty::ARRAY: {
      if (p->_children.empty()) { error(cs, "Invalid type"); }
      auto et = ast_cast<ASTTy>(p->_children[0]);
      auto s = ast_cast<ASTNumberLiteral>(p->_children[1]);
      TAN_ASSERT(et);
      TAN_ASSERT(s);
      p->_n_elements = p->_children.size();
      p->_type_name = "[" + get_type_name(et) + ", " + std::to_string(p->_n_elements) + "]";
      p->_is_ptr = true;
      p->_is_array = true;
      p->_size_bits = tm->getPointerSizeInBits(0);
      p->_align_bits = get_size_bits(cs, et);
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_address;
      break;
    }
    case Ty::POINTER: {
      if (p->_children.empty()) { error(cs, "Invalid type"); }
      auto e = ast_cast<ASTTy>(p->_children[0]);
      TAN_ASSERT(e);
      resolve_ty(cs, e);
      p->_type_name = get_type_name(e) + "*";
      p->_size_bits = tm->getPointerSizeInBits(0);
      p->_align_bits = get_size_bits(cs, e);
      p->_is_ptr = true;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_address;
      break;
    }
    default:
      error(cs, "Invalid type");
  }
  p->_resolved = true;
}

ASTTyPtr get_ptr_to(CompilerSession *cs, ASTTyPtr p) { return create_ty(cs, Ty::POINTER, {p->_ty}, false); }

bool is_lvalue(ASTNodePtr p) { return p->_ty->_is_lvalue; }

ASTTyPtr get_contained_ty(CompilerSession *cs, ASTTyPtr p) {
  if (p->_tyty == Ty::STRING) { return create_ty(cs, Ty::CHAR, vector<ASTNodePtr>(), false); }
  else if (p->_is_ptr) {
    TAN_ASSERT(p->_children.size());
    auto ret = ast_cast<ASTTy>(p->_children[0]);
    TAN_ASSERT(ret);
    return ret;
  } else { return nullptr; }
}

ASTTyPtr get_struct_member_ty(ASTTyPtr p, size_t i) {
  TAN_ASSERT(p->_tyty == Ty::STRUCT);
  return p->_children[i]->_ty;
}

size_t get_struct_member_index(ASTTyPtr p, str name) {
  auto search = p->_member_indices.find(name);
  if (search == p->_member_indices.end()) {
    return (size_t) (-1);
  }
  return search->second;
}

void set_is_lvalue(ASTTyPtr p, bool is_lvalue) { p->_is_lvalue = is_lvalue; }

/// \section Analysis

void analyze(CompilerSession *cs, ASTNodePtr p) {
  switch (p->_type) {
    case ASTType::SUM:
    case ASTType::SUBTRACT:
    case ASTType::MULTIPLY:
    case ASTType::DIVIDE:
    case ASTType::MOD:
      // TODO: p->_ty = std::make_shared<ASTTy>(*_children[_dominant_idx]->get_ty());
      // p->_ty->set_is_lvalue(false);
      break;
    case ASTType::GT:
    case ASTType::GE:
    case ASTType::LT:
    case ASTType::LE:
    case ASTType::EQ:
    case ASTType::NE:
      p->_ty = create_ty(cs, Ty::BOOL);
      break;
    case ASTType::ASSIGN: {
      /// special case for variable declaration
      auto lhs = p->_children[0];
      if (lhs->_type == ASTType::ID) {
        TAN_ASSERT(lhs->_is_named);
        lhs = get_id_referred(cs, lhs);
      }
      if (lhs->_type == ASTType::VAR_DECL) {
        if (!lhs->_ty) {
          auto ty = p->_children[1]->_ty;
          ty = std::make_shared<ASTTy>(*ty); // copy
          ty->_is_lvalue = true;
          lhs->_ty = ty;
        }
      }
      p->_ty = p->_children[0]->_ty;
      if (TypeSystem::CanImplicitCast(p->_ty, p->_children[1]->_ty) != 0) {
        error(cs, "Cannot perform implicit type conversion");
      }
      break;
    }
    case ASTType::NUM_LITERAL:
      if (p->_token->type == TokenType::INT) {
        auto tyty = Ty::INT;
        if (p->_token->is_unsigned) { tyty = TY_OR(tyty, Ty::UNSIGNED); }
        p->_ty = create_ty(cs, tyty);
      } else if (p->_token->type == TokenType::FLOAT) {
        p->_ty = create_ty(cs, Ty::FLOAT);
      }
      break;
    default:
      break;
  }
}

/// \section Other

static void error(CompilerSession *cs, const str &error_message) {
  if (cs && cs->_current_token) {
    report_error(cs->_filename, cs->_current_token, error_message);
  } else { report_error(error_message); }
}

} // namespace tanlang
