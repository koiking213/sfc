#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "parser.hpp"
#include "IR_generator.hpp"
#include "ast.hpp"
/* program -> CST -> AST -> IR */
/* CST -> ASTでエラーチェックを行い、AST -> IRは機械的に行う */

// class Line {
// public:
//   int line_num;
//   std::string raw_data;
//   std::vector<std::string> tokens;
//   Line(int line_num, std::string s) : line_num(line_num), raw_data(s) {}
//   void print() { std::cout << line_num << " " << raw_data << std::endl; }
// private:
// };

void compile(std::fstream &fs, std::string filename) {
  std::string str, line;
  while (getline(fs, line)) {
    str += line + '\n';
  }
  std::unique_ptr<cst::Program> cst_program = parser::parse(str, filename);
  // std::cout << "=== CST ===" << std::endl;
  // cst_program->print();
  std::shared_ptr<ast::Program_unit> ast_program = cst_program->ASTgen();
  // std::cout << std::endl << "=== AST ===" << std::endl;
  // ast_program->print("");
  IR_generator::generate_IR(ast_program);
}

int main(int argc, char* argv[]) {
  //std::cout << argv[1] << std::endl;
  std::fstream fs;
  fs.open(argv[1], std::fstream::in);
  compile(fs, argv[1]);
  fs.close();
  return 0;
}
