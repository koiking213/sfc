#pragma once
#include "parser.hpp"

namespace IR_generator {
  void generate_IR(const std::shared_ptr<ast::Program_unit> program);
  void codeout(std::string outfile_name);
}
