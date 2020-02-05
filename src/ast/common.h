#ifndef TAN_SRC_AST_COMMON_H_
#define TAN_SRC_AST_COMMON_H_
#include "parser.h"
#include "src/ast/astnode.h"
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Function.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Instruction.h>

namespace tanlang {
using llvm::AllocaInst;
using llvm::Function;
using llvm::Type;

/**
 * \brief Create an `alloca` instruction in the specified block. This is used for mutable variables etc.
 */
AllocaInst *create_block_alloca(BasicBlock *block, Type *type, const std::string &name);

bool is_ast_type_in(ASTType t, std::initializer_list<ASTType> list);

Type *typename_to_llvm_type(const std::string &type_name, ParserContext *parser_context);

}

#endif /* TAN_SRC_AST_COMMON_H_ */
