#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "parser.hpp"

class Line {
public:
  int line_num;
  std::string raw_data;
  std::vector<std::string> tokens;
  Line(int line_num, std::string s) : line_num(line_num), raw_data(s) {}
  void print() { std::cout << line_num << " " << raw_data << std::endl; }
private:
};

class AST {
  class Instruction *inst;
  class AST *op1;
  class AST *op2;
};

void lexical_analysis(std::vector<Line> lines) {
  enum program_position {
    outside,
    inside_program_unit,
    inside_interface_body
  };
  for (std::vector<Line>::iterator it = lines.begin(); it != lines.end(); it++) {
    
  }
}

void syntax_analysis(std::vector<Line> lines) {
}

void semantic_analysis(parser::Program &program) {
}

std::vector<AST> create_IR(const parser::Program &program) {
  std::vector<AST> ast;
  return ast;
}

std::vector<AST> frontend(std::string input) {
  parser::Program program;
  parser::do_parse(input, program);
  parser::print_program(program);
  semantic_analysis(program);
  return create_IR(program);
}

void compile(std::fstream &fs) {
  int line_num = 1;
  std::string str, line;
  while (getline(fs, line)) {
    str += line + '\n';
  }
  std::vector<AST> ast = frontend(str);
}

int main(int argc, char* argv[]) {
  std::cout << argv[1] << std::endl;
  std::fstream fs;
  fs.open(argv[1], std::fstream::in);
  compile(fs);
  fs.close();
  return 0;
}
