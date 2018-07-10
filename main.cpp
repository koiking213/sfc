#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
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

void compile(std::fstream &fs, std::string infile_name, std::string outfile_name) {
  std::string str, line;
  while (getline(fs, line)) {
    str += line + '\n';
  }
  std::unique_ptr<cst::Program> cst_program = parser::parse(str, infile_name);
  // std::cout << "=== CST ===" << std::endl;
  // cst_program->print();
  std::shared_ptr<ast::Program_unit> ast_program = cst_program->ASTgen();
  // std::cout << std::endl << "=== AST ===" << std::endl;
  // ast_program->print("");
  IR_generator::generate_IR(ast_program);
  // generate .o
  IR_generator::codeout(outfile_name);
}

void link(std::vector<std::string> file_list, std::vector<std::string> option_list) {
  std::string command="clang++";
  for (std::string file : file_list) {
    command = command + " " + file;
  }
  command = command + " " + "libfortio.a";
  for (std::string opt : option_list) {
    command = command + " " + opt;
  }
  system(command.c_str());
}

int main(int argc, char* argv[]) {
  std::vector<std::string> file_list;
  std::vector<std::string> option_list;
  bool link_flag = true;
  std::string output_name = "";
  int opt;
  while ((opt = getopt(argc, argv, "co:")) != -1) {
    switch (opt) {
    case 'c':
      link_flag = false;
      break;
    case 'o':
      option_list.push_back("-o " + std::string(optarg));
      break;
    }
  }
  for (int i=optind; i<argc; i++) {
    std::string filename = argv[i];
    if (filename.find(".f90", filename.size() - 4) != std::string::npos) {
      std::fstream fs;
      fs.open(filename, std::fstream::in);
      std::string outfile_name = filename.substr(0, filename.find_last_of(".")) + ".o";
      outfile_name = outfile_name.substr(outfile_name.find_first_of("/")+1, outfile_name.length());
      compile(fs, filename, outfile_name);
      file_list.push_back(outfile_name);
      fs.close();
    } else if (filename.find(".o", filename.size() - 4) != std::string::npos) {
      file_list.push_back(filename);
    } else if (filename.find(".f", filename.size() - 4) != std::string::npos) {
      std::cout << "error: fixed form is not supported yet!" << std::endl;
      return 0;
    }
  }
  if (file_list.size() == 0) {
    std::cout << "error: no input file" << std::endl;
    return 0;
  }
  if (link_flag) {
    link(file_list, option_list);
  }
  return 0;
}
