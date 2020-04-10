#include "src/ast/ast_loop.h"
#include "compiler_session.h"
#include "intrinsic.h"

namespace tanlang {

llvm::Value *ASTLoop::codegen(CompilerSession *compiler_session) {
  if (_loop_type == ASTLoopType::WHILE) {
    Function *func = compiler_session->get_builder()->GetInsertBlock()->getParent();
    BasicBlock *loop_bb = BasicBlock::Create(*compiler_session->get_context(), "loop", func);
    BasicBlock *body_bb = BasicBlock::Create(*compiler_session->get_context(), "loop_body", func);
    BasicBlock *after_bb = BasicBlock::Create(*compiler_session->get_context(), "after_loop", func);

    /// enter loop_bb
    compiler_session->get_builder()->CreateBr(loop_bb);

    /// start insertion in loop_bb
    compiler_session->get_builder()->SetInsertPoint(loop_bb);
    /// condition
    // TODO: use ASTTy::convert_to
    auto *cond = _children[0]->codegen(compiler_session);
    unsigned condition_bits = cond->getType()->getPrimitiveSizeInBits();
    if (condition_bits != 1) {
      auto *op_type = compiler_session->get_builder()->getIntNTy(condition_bits);
      if (!cond->getType()->isIntegerTy()) {
        cond = compiler_session->get_builder()->CreateBitCast(cond, op_type);
      }
      cond = compiler_session->get_builder()->CreateICmpNE(cond, ConstantInt::get(op_type, 0, true));
    }
    compiler_session->get_builder()->CreateCondBr(cond, body_bb, after_bb);

    /// loop body
    compiler_session->get_builder()->SetInsertPoint(body_bb);
    _children[1]->codegen(compiler_session);

    /// back to loop start
    compiler_session->get_builder()->CreateBr(loop_bb);

    /// after loop
    compiler_session->get_builder()->SetInsertPoint(after_bb);
    compiler_session->get_builder()->CreateCall(Intrinsic::GetIntrinsic(IntrinsicType::NOOP, compiler_session));
  } else {
    assert(false);
  }
  return nullptr;
}

} // namespace tanlang


