#include "libtanc.h"
#include "compiler.h"
#include "lexer.h"
#include "linker.h"
#include "parser.h"
#include "base.h"
#include "src/llvm_include.h"

#ifndef DEBUG
#define BEGIN_TRY try {
#else
#define BEGIN_TRY
#endif

#ifndef DEBUG
#define END_TRY                                                                \
  }                                                                            \
  catch (const std::exception &e) {                                            \
    std::cerr << "Error encountered in file " << files[i]                      \
              << ": " << e.what() << '\n';                                     \
    return false;                                                              \
  }
#else
#define END_TRY
#endif

static bool _link(std::vector<std::string> input_paths, TanCompilation *config) {
  using tanlang::Linker;
  Linker linker;
  linker.add_files(input_paths);
  linker.add_flag("-o" + std::string(config->out_file));
  if (config->type == EXE) {
    /// default flags
    linker.add_flags({"-fPIE"});
  } else if (config->type == SLIB) {
    // TODO: implement output type static lib
    TAN_ASSERT(false);
    // linker.add_flags({"-static-pie"});
  } else if (config->type == DLIB) {
    linker.add_flags({"-shared"});
  }
  return linker.link();
}

bool compile_files(unsigned n_files, char **input_paths, TanCompilation *config) {
  bool print_ir_code = config->verbose >= 1;
  bool print_ast = config->verbose >= 2;
  std::vector<std::string> files;
  files.reserve(n_files);
  for (size_t i = 0; i < n_files; ++i) {
    files.push_back(std::string(input_paths[i]));
  }
  std::vector<std::shared_ptr<Compiler>> compilers{};
  compilers.reserve(n_files);
  /// parse all files before generating IR code
  for (size_t i = 0; i < n_files; ++i) {
    BEGIN_TRY
    auto compiler = std::make_shared<Compiler>(files[i]);
    compilers.push_back(compiler);
    compiler->parse();
    if (print_ast) { compiler->dump_ast(); }
    END_TRY
  }
  for (size_t i = 0; i < n_files; ++i) {
    BEGIN_TRY
    compilers[i]->codegen();
    if (print_ir_code) { compilers[i]->dump_ir(); }
    /// prepare the filename for linking
    files[i] += ".o";
    files[i] = fs::path(files[i]).filename().string();
    std::cout << "Compiling TAN file: " << files[i] << "\n";
    compilers[i]->emit_object(files[i]);
    END_TRY
  }

  /// link
  for (size_t i = 0; i < config->n_link_files; ++i) {
    files.push_back(std::string(config->link_files[i]));
  }
  if (config->type != OBJ) {
    bool ret = _link(files, config);
    if (!ret) {
      std::cerr << "Error linking files\n";
    }
  }
  llvm::llvm_shutdown();
  return true;
}
