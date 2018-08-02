#include "parser.hpp"
#include "cst.hpp"
#include "Line.hpp"
#include <string>
#include <iostream>
#include <algorithm>
#include <memory>
#include <ctype.h>
#include <stack>

using namespace cst;

namespace parser{
  std::unique_ptr<Expression> parse_expression();
  // TODO: multiple Program Unit
  // TODO: fixed form
  // TODO: continuous line
  int row;
  std::string filename;
  bool error_occured;
  std::vector<Line*> source;
  std::stack<int> saved_ofs_stack;
  Line *current_line;
  void preprocess(std::string str, std::string name)
  {
    row = 0;
    error_occured = false;
    filename = name;
    source.clear();

    for (int head=0, tail=0, lineno=0; tail < str.size(); tail++) {
      if (str[tail] == '\n') {
        lineno++;
        source.push_back(new Line(lineno, str.substr(head, tail-head)));
        head = tail+1;
      }
    }
    current_line = source[0];
  }
  void error(std::string msg)
  {
    error_occured = true;
    std::cout << filename << ":" << row+1 << " error: " << msg << std::endl;
  }
  bool is_eof()
  {
    return source.size() == row;
  }
  void skip_this_line()
  {
    current_line = source[++row];
    assert(saved_stack_ofs.size() == 0);
  }
  void skip_blank_lines()
  {
    while (!is_eof() && current_line->is_end_of_line()) {
      skip_this_line();
    }
  }
  void save_ofs()
  {
    saved_ofs_stack.push(current_line->column);
  }
  void restore_ofs()
  {
    current_line->column = saved_ofs_stack.top();
    saved_ofs_stack.pop();
  }
  void discard_saved_ofs()
  {
    saved_ofs_stack.pop();
  }
  bool assert_end_of_line()
  {
    if (current_line->is_end_of_line()) {
      return true;
    } else {
      error("unexpected token in end of line");
      return false;
    }
  }
  bool is_end_of_line()
  {
    return current_line->is_end_of_line();
  }
  std::string read_name()
  {
    return current_line->read_name();
  }
  bool read_one_blank()
  {
    return current_line->read_one_blank();
  }
  bool read_token(const std::string str)
  {
    return current_line->read_token(str);
  }
  std::unique_ptr<Constant> read_constant()
  {
    std::string value;
    value = current_line->read_real_constant();
    if (value != "") {
      std::unique_ptr<Constant> cnt { new Constant(cst::Type_kind::Intrinsic, "real", value) };
      return std::move(cnt);
    }
    value = current_line->read_int_constant();
    if (value != "") {
      std::unique_ptr<Constant> cnt { new Constant(cst::Type_kind::Intrinsic, "integer", value) };
      return std::move(cnt);
    }
    return nullptr;
  }

  std::unique_ptr<Program> parse_program_stmt()
  {
    skip_blank_lines();
    std::string name = "";
    {
      read_token("program");
      if (!read_one_blank()) {
        error("missing whitespace after PROGRAM");
        goto exit;
      }
      name = read_name();
      if (name == "") {
        error("missing program name in program-stmt");
        goto exit;
      }
    }
    assert_end_of_line();
  exit:
    skip_this_line();
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
    if (!read_one_blank()) {
      error("missing whitespace after END");
      goto errexit;
    }
    if (!read_token("program")) {
      error("unexpected token in end-program-stmt");
      goto errexit;
    }
    if (is_end_of_line()) {
      return true;
    }
    if (!read_one_blank()) {
      error("missing whitespace after PROGRAM");
      goto errexit;
    }
    if (!read_token(name)) {
      error("name is different from the corresponding program-stmt");
      goto errexit;
    }
    if (!assert_end_of_line()) {
      goto errexit;
    }
    return true;
    
  errexit:
    skip_this_line();
    return false;
  }

  std::unique_ptr<Specification> parse_type_declaration()
  {
    skip_blank_lines();
    std::unique_ptr<Type_specification> spec;
    if (read_token("integer")) {
      spec = std::make_unique<Type_specification>(Type_kind::Intrinsic, "integer");
    } else if (read_token("real")) {
      spec = std::make_unique<Type_specification>(Type_kind::Intrinsic, "real");
    } else {
      return nullptr;
    }
    read_one_blank(); // TOOD: semicolon should be also accepted
    do {
      std::string name = read_name();
      spec->add_variable(name);
    } while (read_token(","));
    return std::move(spec);
  }
  
  std::unique_ptr<Specification> parse_declaration_construct()
  {
    return parse_type_declaration();
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
    save_ofs();
    std::unique_ptr<Assignment_statement> assignment_stmt {new Assignment_statement()};
    // only variable is acceptable as a left side of assignment statement
    std::string name = read_name();
    if (!read_token("=")) {
      restore_ofs();
      return nullptr;
    }
    std::unique_ptr<Variable> var { new Variable(name) };
    assignment_stmt->set_lhs(std::move(var));
    assignment_stmt->set_rhs(parse_expression());
    discard_saved_ofs();
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
  std::unique_ptr<Program> parse_main_program()
  {
    std::unique_ptr<Program> program = parse_program_stmt();
    std::unique_ptr<Specification> spec;
    while ((spec = parse_declaration_construct())) {
      program->add_specification(std::move(spec));
    }
    std::unique_ptr<Executable_construct> exec;
    while ((exec = parse_executable_constructs())) {
      program->add_executable_construct(std::move(exec));
    }
    parse_end_program_stmt(program->get_name());
    return std::move(program);
  }
  std::unique_ptr<Program> parse(const std::string str, const std::string name)
  {
    preprocess(str, name);
    std::unique_ptr<Program> program = parse_main_program();
    if (error_occured) {
      return nullptr;
    } else {
      return std::move(program);
    }
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
