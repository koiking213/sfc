#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
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

bool compile(std::fstream &fs, std::string infile_name, std::string outfile_name) {
  std::string str, line;
  while (getline(fs, line)) {
    str += line + '\n';
  }
  std::unique_ptr<cst::Program> cst_program = parser::parse(str, infile_name);
  if (!cst_program) return false;
#ifdef DEBUG_MODE
  std::cout << "=== CST ===" << std::endl;
  cst_program->print();
#endif
  std::shared_ptr<ast::Program_unit> ast_program = cst_program->ASTgen();
#ifdef DEBUG_MODE
  std::cout << std::endl << "=== AST ===" << std::endl;
  ast_program->print("");
#endif
  IR_generator::generate_IR(ast_program);
  // generate .o
  IR_generator::codeout(outfile_name);
  return true;
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
  std::vector<std::string> outfile_list;
  std::vector<std::string> objfile_list;
  std::vector<std::string> option_list;
  bool link_flag = true;
  std::string output_name = "";
  bool success = true;
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
  char* tmpdir_env = std::getenv("TMPDIR");
  std::string tmpdir;
  if (tmpdir_env) {
    tmpdir = tmpdir_env;
  } else {
    tmpdir = "/tmp";
  }
  for (int i=optind; i<argc; i++) {
    std::string filename = argv[i];
    if (filename.find(".f90", filename.size() - 4) != std::string::npos) {
      std::fstream fs;
      fs.open(filename, std::fstream::in);
      std::string outfile_name = filename.substr(0, filename.find_last_of(".")) + ".o";
      outfile_name = outfile_name.substr(outfile_name.find_last_of("/")+1, outfile_name.length());
      if (link_flag) {
	outfile_name = tmpdir + "/" + outfile_name;
      }
      success &= compile(fs, filename, outfile_name);
      outfile_list.push_back(outfile_name);
      fs.close();
    } else if (filename.find(".o", filename.size() - 2) != std::string::npos) {
      objfile_list.push_back(filename);
    } else if (filename.find(".f", filename.size() - 2) != std::string::npos) {
      std::cout << "error: fixed form is not supported yet!" << std::endl;
      return 0;
    }
  }
  if (outfile_list.size() == 0 && objfile_list.size() == 0) {
    std::cout << "error: no input file" << std::endl;
    return 0;
  }
  if (link_flag && success) {
    std::vector<std::string> file_list;
    file_list.insert(file_list.end(), objfile_list.begin(), objfile_list.end());
    file_list.insert(file_list.end(), outfile_list.begin(), outfile_list.end());
    link(file_list, option_list);
    for (std::string file : outfile_list) {
      std::remove(file.c_str());
    }
  }
  if (success) {
    return 0;
  } else {
    return 1;
  }
}
