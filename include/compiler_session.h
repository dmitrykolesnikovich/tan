#ifndef TAN_INCLUDE_COMPILER_SESSION_H_
#define TAN_INCLUDE_COMPILER_SESSION_H_
#include "src/llvm_include.h"
#include "src/ast/scope.h"

namespace tanlang {

class CompilerSession final {
public:
  static void add_public_function(const std::string &filename, const std::string &name, std::shared_ptr<ASTNode> func);
  static std::vector<std::shared_ptr<ASTNode>> get_public_functions(const std::string &filename);
private:
  /// filename -> (function name -> ASTFunction)
  static std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<ASTNode>>> public_func;
public:
  CompilerSession &operator=(const CompilerSession &) = delete;
  CompilerSession(const CompilerSession &) = delete;
  CompilerSession() = delete;

  CompilerSession(const std::string &module_name, TargetMachine *target_machine);
  ~CompilerSession() = default;

  std::shared_ptr<Scope> get_current_scope();
  std::shared_ptr<Scope> push_scope();
  std::shared_ptr<Scope> pop_scope();
  DIScope *get_current_di_scope() const;
  void push_di_scope(DIScope *scope);
  void pop_di_scope();
  void set_code_block(BasicBlock *block);
  [[nodiscard]] BasicBlock *get_code_block() const;
  void add(const std::string &name, std::shared_ptr<ASTNode> value);
  void set(const std::string &name, std::shared_ptr<ASTNode> value);
  std::shared_ptr<ASTNode> get(const std::string &name);
  LLVMContext *get_context();
  std::unique_ptr<IRBuilder<>> &get_builder();
  std::unique_ptr<DIBuilder> &get_di_builder();
  std::unique_ptr<Module> &get_module();
  std::unique_ptr<FunctionPassManager> &get_function_pass_manager();
  void finalize_codegen();

  unsigned get_ptr_size() const { return _target_machine->getPointerSizeInBits(0); }

public:
  DIFile *get_di_file() const { return _di_file; }

  DICompileUnit *get_di_cu() const { return _di_cu; }

  void set_current_debug_location(size_t l, size_t c);

private:
  std::unique_ptr<LLVMContext> _context;
  std::unique_ptr<IRBuilder<>> _builder;
  std::unique_ptr<Module> _module;
  std::vector<std::shared_ptr<Scope>> _scope{};
  std::vector<DIScope *> _di_scope{};
  std::unique_ptr<FunctionPassManager> _fpm{};
  TargetMachine *_target_machine = nullptr;

  /// debug information
  std::unique_ptr<DIBuilder> _di_builder{};
  DICompileUnit *_di_cu = nullptr;
  DIFile *_di_file = nullptr;

private:
  void initialize_scope();
  void init_llvm();
};

} // namespace tanlang

#endif /*TAN_INCLUDE_COMPILER_SESSION_H_*/
