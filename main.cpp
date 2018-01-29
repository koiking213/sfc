#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "parser.hpp"
#include "IR_generator.hpp"

class Line {
public:
  int line_num;
  std::string raw_data;
  std::vector<std::string> tokens;
  Line(int line_num, std::string s) : line_num(line_num), raw_data(s) {}
  void print() { std::cout << line_num << " " << raw_data << std::endl; }
private:
};

void semantic_analysis(parser::Program &program) {
}

void compile(std::fstream &fs) {
  std::string str, line;
  while (getline(fs, line)) {
    str += line + '\n';
  }
  parser::Program program;
  parser::do_parse(str, program);
  parser::print_program(program);
  semantic_analysis(program);
  create_IR(program);
}

int main(int argc, char* argv[]) {
  std::cout << argv[1] << std::endl;
  std::fstream fs;
  fs.open(argv[1], std::fstream::in);
  compile(fs);
  fs.close();
  return 0;
}
