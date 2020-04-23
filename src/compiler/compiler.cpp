#include "compiler.h"
#include "libtanc.h"
#include "compiler_session.h"
#include "intrinsic.h"

namespace tanlang {

Compiler::~Compiler() {
  delete _llvm_module;
  delete _target_machine;
}

Compiler::Compiler(std::string filename, ASTNodePtr ast, TanCompilation *config) : _ast(ast) {
  { /// target machine and data layout
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    auto target_triple = llvm::sys::getDefaultTargetTriple();
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(target_triple, error);
    if (!target) { throw std::runtime_error(error); }
    auto CPU = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    /// relocation model
    auto RM = llvm::Reloc::Model::PIC_;
    _target_machine = target->createTargetMachine(target_triple, CPU, features, opt, RM);
  }
  _compiler_session = new CompilerSession(filename, _target_machine);
  _llvm_module = _compiler_session->get_module().get();
}

void Compiler::emit_object(const std::string &filename) {
  std::error_code ec;
  llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);

  if (ec) {
    throw std::runtime_error("Could not open file: " + ec.message());
  }
  llvm::legacy::PassManager pass;
  auto file_type = llvm::LLVMTargetMachine::CGFT_ObjectFile;

  if (_target_machine->addPassesToEmitFile(pass, dest, nullptr, file_type)) {
    throw std::runtime_error("Target machine can't emit a file of this type");
  }

  pass.run(*_llvm_module);
  dest.flush();
}

Value *Compiler::codegen() {
  Intrinsic::InitCodegen(_compiler_session);
  auto *ret = _ast->codegen(_compiler_session);
  _compiler_session->finalize_codegen();
  llvm::verifyModule(*_llvm_module);
  return ret;
}

} // namespace tanlang
