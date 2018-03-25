#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "parser.hpp"
#include "IR_generator.hpp"
#include "ast.hpp"

class Line {
public:
  int line_num;
  std::string raw_data;
  std::vector<std::string> tokens;
  Line(int line_num, std::string s) : line_num(line_num), raw_data(s) {}
  void print() { std::cout << line_num << " " << raw_data << std::endl; }
private:
};

// make AST from parsed information
std::vector<ast::ProgramUnit *> *semantic_analysis(parser::Program &program) {
  auto *program_units = new std::vector<ast::ProgramUnit *>();
  program_units->push_back(program.ASTgen());
  return program_units;
}

void compile(std::fstream &fs) {
  std::string str, line;
  while (getline(fs, line)) {
    str += line + '\n';
  }
  parser::Program program;
  parser::do_parse(str, program);
  parser::print_program(program);
  auto programs = semantic_analysis(program);
  for (auto p : *programs) {
    IR_generator::generate_IR(*p);
  }
}

int main(int argc, char* argv[]) {
  std::cout << argv[1] << std::endl;
  std::fstream fs;
  fs.open(argv[1], std::fstream::in);
  compile(fs);
  fs.close();
  return 0;
}
