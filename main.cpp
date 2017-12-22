#include <string>
#include <iostream>
#include <fstream>
#include <vector>

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

void semantic_analysis(std::vector<Line> lines) {
}

std::vector<AST> create_IR(std::vector<Line> lines) {
  std::vector<AST> ast;
  return ast;
}

std::vector<AST> frontend(std::vector<Line> lines) {
  lexical_analysis(lines);
  syntax_analysis(lines);
  semantic_analysis(lines);
  return create_IR(lines);
}

void compile(std::fstream &fs) {
  int line_num = 1;
  std::vector<Line>  lines;
  std::string str;
  while (getline(fs, str)) {
    lines.push_back(Line(line_num, str));
    line_num++;
  }
  for (std::vector<Line>::iterator  it = lines.begin(); it != lines.end(); it++) {
    it->print();
  }
  std::vector<AST> ast = frontend(lines);
}

int main(int argc, char* argv[]) {
  std::cout << argv[1] << std::endl;
  std::fstream fs;
  fs.open(argv[1], std::fstream::in);
  compile(fs);
  fs.close();
  return 0;
}
