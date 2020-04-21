#include "ast_import.h"
#include "compiler_session.h"
#include "src/ast/ast_func.h"
#include "parser.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace tanlang {

ASTImport::ASTImport(Token *token, size_t token_index) : ASTNode(ASTType::IMPORT, 0, 0, token, token_index) {}

std::string ASTImport::to_string(bool print_prefix) const {
  std::string ret = "";
  if (print_prefix) {
    ret += ASTNode::to_string(print_prefix) + " ";
  }
  ret += _file;
  return ret;
}

Value *ASTImport::codegen(CompilerSession *compiler_session) {
  // TODO: import path resolve system
  // TODO: path containing non-ASCII characters?
  auto import_path = fs::path(_file);
  import_path = fs::relative(import_path);
  auto path = import_path.string();
  auto funcs = CompilerSession::get_public_functions(import_path.string());
  if (funcs.empty()) {
    if (!fs::exists(path)) { report_code_error(_token, "Cannot find import file " + _file); }
    Reader reader;
    reader.open(path);
    auto tokens = tanlang::tokenize(&reader);
    tanlang::Parser parser(tokens, path);
    parser.parse();
  }
  funcs = CompilerSession::get_public_functions(import_path.string());
  for (auto &n: funcs) {
    ast_cast<ASTFunction>(n)->codegen_prototype(compiler_session);
  }
  return nullptr;
}

} // namespace tanlang
