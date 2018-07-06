#include "parser.hpp"
#include "cst.hpp"
#include <string>
#include <iostream>
#include <algorithm>
#include <memory>
#include <ctype.h>

using namespace cst;

namespace parser{
  std::unique_ptr<Expression> parse_expression();
  // TODO: multiple Program Unit
  // TODO: fixed form
  // TODO: continuous line
  int ofs;
  std::string source; // lower
  std::string source_orig; // same as user described, for error message
  std::string filename;
  int32_t get_column()
  {
    return std::count(source.begin(), source.begin()+ofs, '\n') + 1;
  }
  void error(std::string msg)
  {
    std::cout << filename << ":" << get_column() << " error: " << msg << std::endl;
  }
  bool is_blank(char c)
  {
    return (c == ' ' || c == '\t');
  }
  void skip_blanks()
  {
    while (is_blank(source[ofs])) {
      ofs++;
    }
  }
  std::string read_name()
  {
    skip_blanks();
    if (!isalpha(source[ofs])) {
      return "";
    }
    int begin = ofs;
    while (isalpha(source[ofs]) ||
	   isdigit(source[ofs]) ||
	   source[ofs] == '_') {
      ofs++;
    }
    return source.substr(begin, ofs-begin);
  }
  bool read_one_blank()
  {
    if (is_blank(source[ofs])) {
      ofs++;
      return true;
    }
    return false;
  }
  bool is_end_of_line()
  {
    skip_blanks();
    return source[ofs] == '\n';
  }
  void skip_blank_lines()
  {
    while (is_blank(source[ofs]) || source[ofs] == '\n' ) {
      ofs++;
    }
  }
  bool read_token(const std::string str)
  {
    skip_blanks();
    if (std::equal(str.begin(), str.end(), source.begin()+ofs)) {
      ofs += str.size();
      return true;
    } else {
      return false;
    }
  }
  bool assert_end_of_line()
  {
    if (is_end_of_line()) {
      return true;
    } else {
      error("unexpected token in end of line");
      return false;
    }
  }

  std::unique_ptr<Constant> read_constant()
  {
    if (!isdigit(source[ofs])) {
      return nullptr;
    }
    int begin = ofs;
    // TOOD: other than integer
    while (isdigit(source[ofs])) {
      ofs++;
    }
    std::string value = source.substr(begin, ofs-begin);
    std::unique_ptr<Constant> cnt { new Constant(cst::Type_kind::Intrinsic, "integer", value) };
    return std::move(cnt);
  }
  std::unique_ptr<Program> parse_program_stmt()
  {
    skip_blank_lines();
    read_token("program");
    read_one_blank();
    std::string name = read_name();
    if (name == "") {
      error("missing program name in program-stmt");
    }
    assert_end_of_line();
    std::unique_ptr<Program> program {new Program(name)};
    return std::move(program);
  }
  bool parse_end_program_stmt(const std::string name)
  {
    skip_blank_lines();
    read_token("end");
    if (is_end_of_line()) {
      return true;
    }
    read_one_blank();
    if (!read_token("program")) {
      error("unexpected token in end-program-stmt");
      return false;
    }
    if (is_end_of_line()) {
      return true;
    }
    read_one_blank();
    if (!read_token(name)) {
      error("name is different from the corresponding program-stmt");
      return false;
    }
    return assert_end_of_line();
  }

  std::unique_ptr<Specification> parse_type_declaration()
  {
    skip_blank_lines();
    std::unique_ptr<Type_specification> spec {new Type_specification};
    if (read_token("integer")) {
      spec->type_kind = Type_kind::Intrinsic;
      spec->type_name = "integer";
    } else {
      return nullptr;
    }
    read_one_blank(); // TOOD: semicolon should be also accepted
    std::string name = read_name();
    spec->variables.push_back(name);
    while (read_token(",")) {
      name = read_name();
      spec->variables.push_back(name);
    }
    return std::move(spec);
  }
  
  std::unique_ptr<Specification> parse_declaration_construct()
  {
    return parse_type_declaration();
  }
  
  void parse_specification_part(std::vector<std::unique_ptr<Specification>> &specifications)
  {
    std::unique_ptr<Specification> spec;
    while ((spec = parse_declaration_construct())) {
      specifications.push_back(std::move(spec));
    }
  }
  std::unique_ptr<Expression> parse_mult_operand()
  {
    std::string name = read_name();
    if (name != "") {
      return static_cast<std::unique_ptr<Expression>>(new Variable(name));
    }
    std::unique_ptr<Constant> value = read_constant();
    if (value != nullptr) {
      return static_cast<std::unique_ptr<Expression>>(std::move(value));
    }
    if (read_token("(")) {
      std::unique_ptr<Expression> exp = parse_expression();
      read_token(")");
      return std::move(exp);
    }
    return nullptr;
  }
  std::unique_ptr<Expression> parse_add_operand()
  {
    std::unique_ptr<Expression> exp { new Expression() };
    std::unique_ptr<Expression> left = parse_mult_operand();
    if (read_token("*")) {
      exp->set_operator("*");
    } else if (read_token("/")) {
      exp->set_operator("/");
    } else {
      return std::move(left);
    }
    exp->add_operand(std::move(left));
    exp->add_operand(parse_expression());
    return std::move(exp);
  }
  std::unique_ptr<Expression> parse_expression()
  {
    std::unique_ptr<Expression> exp { new Expression() };
    std::unique_ptr<Expression> left = parse_add_operand();
    if (read_token("+")) {
      exp->set_operator("+");
    } else if (read_token("-")) {
      exp->set_operator("-");
    } else {
      return std::move(left);
    }
    exp->add_operand(std::move(left));
    exp->add_operand(parse_expression());
    return std::move(exp);
  }
  std::unique_ptr<Assignment_statement> parse_assignment_stmt()
  {
    int saved_offset = ofs;
    std::unique_ptr<Assignment_statement> assignment_stmt {new Assignment_statement()};
    // only variable is acceptable as a left side of assignment statement
    std::string name = read_name();
    if (!read_token("=")) {
      ofs = saved_offset; // roll back
      return nullptr;
    }
    std::unique_ptr<Variable> var { new Variable(name) };
    assignment_stmt->set_lhs(std::move(var));
    assignment_stmt->set_rhs(parse_expression());
    return std::move(assignment_stmt);
  }
  std::unique_ptr<Print_statement> parse_print_stmt()
  {
    std::unique_ptr<Print_statement> print_stmt { new Print_statement() };
    // "print" is already read
    read_token("*");
    read_token(",");
    print_stmt->add_element(parse_expression());
    while (read_token(",")) {
      print_stmt->add_element(parse_expression());
    }
    return std::move(print_stmt);
  }
  std::unique_ptr<Executable_construct> parse_action_stmt()
  {
    skip_blank_lines();
    std::unique_ptr<Executable_construct> exec = parse_assignment_stmt();
    if (exec) {
      return exec;
    }
    if (read_token("print")) {
      return parse_print_stmt();
    }
    return nullptr;
  }
  std::unique_ptr<Executable_construct> parse_executable_constructs()
  {
    return parse_action_stmt();
  }
  void parse_executable_part(std::vector<std::unique_ptr<Executable_construct>> &executable_constructs)
  {
    std::unique_ptr<Executable_construct> exec;
    while ((exec = parse_executable_constructs())) {
      executable_constructs.push_back(std::move(exec));
    }
  }
  std::unique_ptr<Program> parse_main_program()
  {
    std::unique_ptr<Program> program = parse_program_stmt();
    parse_specification_part(program->specifications);
    parse_executable_part(program->executable_constructs);
    parse_end_program_stmt(program->name);
    return std::move(program);
  }
  std::unique_ptr<Program> parse(const std::string str, const std::string name)
  {
    ofs = 0;
    source_orig = str;
    source = str;
    filename = name;
    std::transform(source.begin(), source.end(), source.begin(), tolower);
    return std::move(parse_main_program());
  }
}

#ifdef PARSER_MAIN
int main()
{
  std::string str;
  std::cin.unsetf(std::ios::skipws);
  std::copy(std::istreambuf_iterator<char>(std::cin),
	    std::istreambuf_iterator<char>(),
	    std::back_inserter(str));
  std::unique_ptr<Program> program = parser::parse(str);
  program->print();
  return 0;
}
#endif
