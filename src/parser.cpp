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
  enum class err_kind : int {
    end_of_line,
    name,
    character
  };
  // prototype declarations for recursive constructs
  std::unique_ptr<Executable_construct> parse_action_stmt();
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
  void error(std::string msg, enum err_kind kind)
  {
    error_occured = true;
    std::cout << filename << ":" << row+1 << ":" << current_line->get_column()+1 << " error: " << msg << std::endl;
    std::cout << "    " << current_line->get_content_orig() << std::endl;
    std::cout << "    " << std::string(current_line->get_column(), ' ');
    int length=1;
    switch (kind) {
    case err_kind::end_of_line:
      length = current_line->get_content().size() - current_line->get_column();
      break;
    case err_kind::name:
      {
        std::string name = current_line->read_name();
        length = name.size();
        break;
      }
    case err_kind::character:
      break;
    }
    std::cout << std::string(length, '^') << std::endl;
  }
  bool is_eof()
  {
    return source.size() == row;
  }
  void skip_this_line()
  {
    current_line = source[++row];
    assert(saved_ofs_stack.size() == 0);
  }
  void skip_blank_lines()
  {
    while (!is_eof() && current_line->is_end_of_line()) {
      skip_this_line();
    }
  }
  void save_ofs()
  {
    saved_ofs_stack.push(current_line->get_column());
  }
  void restore_ofs()
  {
    current_line->set_column(saved_ofs_stack.top());
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
      current_line->skip_blanks();
      error("unexpected token in end of line", err_kind::end_of_line);
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
  bool read_one_blank(bool output_error=true)
  {
    if (current_line->read_one_blank()) {
      return true;
    } else {
      if (output_error) {
        error("missing whitespace", err_kind::character);
      }
      return false;
    }
  }
  bool read_token(const std::string str)
  {
    return current_line->read_token(str);
  }
  bool read_operator(const std::string str)
  {
    return current_line->read_operator(str);
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
    value = current_line->read_logical_constant();
    if (value != "") {
      std::unique_ptr<Constant> cnt { new Constant(cst::Type_kind::Intrinsic, "logical", value) };
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
        goto exit;
      }
      name = read_name();
      if (name == "") {
        error("missing program name in program-stmt", err_kind::character);
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
      goto errexit;
    }
    if (!read_token("program")) {
      error("unexpected token in end-program-stmt", err_kind::character);
      goto errexit;
    }
    if (is_end_of_line()) {
      return true;
    }
    if (!read_one_blank()) {
      goto errexit;
    }
    if (!read_token(name)) {
      error("name is different from the corresponding program-stmt", err_kind::name);
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
    } else if (read_token("logical")) {
      spec = std::make_unique<Type_specification>(Type_kind::Intrinsic, "logical");
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

  // mult-operand is level-1-expr [ power-op mult-operand ]
  // for now, level-1-expr := name | constant | (expression)
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

  // add-operand is [ add-operand mult-op ] mult-operand
  std::unique_ptr<Expression> parse_add_operand()
  {
    std::unique_ptr<Expression> exp { new Expression() };
    std::unique_ptr<Expression> operand = parse_mult_operand();
    while (true) {
      if (read_operator("*")) {
        exp->add_operator("*");
      } else if (read_operator("/")) {
        exp->add_operator("/");
      } else {
        break;
      }
      exp->add_operand(std::move(operand));
      operand = parse_mult_operand();
    }
    if (exp->get_operator_count() == 0) {
      return std::move(operand);
    }
    exp->add_operand(std::move(operand));
    return std::move(exp);
  }

  // level-2-expr is [ [ level-2-expr ] add-op ] add-operand
  std::unique_ptr<Expression> parse_level2_expr()
  {
    std::unique_ptr<Expression> exp { new Expression() };
    std::unique_ptr<Expression> operand = parse_add_operand();
    while (true) {
      if (read_operator("+")) {
        exp->add_operator("+");
      } else if (read_operator("-")) {
        exp->add_operator("-");
      } else {
        break;
      }
      exp->add_operand(std::move(operand));
      operand = parse_level2_expr();
    }
    if (exp->get_operator_count() == 0) {
      return std::move(operand);
    }
    exp->add_operand(std::move(operand));
    return std::move(exp);
  }

  // level-4-expr is [ level-3-expr rel-op ] level-3-expr
  std::unique_ptr<Expression> parse_expression()
  {
    // level-4-expr
    std::unique_ptr<Expression> exp { new Expression() };
    std::unique_ptr<Expression> operand = parse_level2_expr();
    while (true) {
      if (read_operator(".eq.")) {
        exp->add_operator("==");
      } else if (read_operator(".ne.")) {
        exp->add_operator("/=");
      } else if (read_operator(".lt.")) {
        exp->add_operator("<");
      } else if (read_operator(".le.")) {
        exp->add_operator("<=");
      } else if (read_operator(".gt.")) {
        exp->add_operator(">");
      } else if (read_operator(".ge.")) {
        exp->add_operator(">=");
      } else if (read_operator("==")) {
        exp->add_operator("==");
      } else if (read_operator("/=")) {
        exp->add_operator("/=");
      } else if (read_operator("<")) {
        exp->add_operator("<");
      } else if (read_operator("<=")) {
        exp->add_operator("<=");
      } else if (read_operator(">")) {
        exp->add_operator(">");
      } else if (read_operator(">=")) {
        exp->add_operator(">=");
      } else {
        break;
      }
      exp->add_operand(std::move(operand));
      operand = parse_expression();
    }
    if (exp->get_operator_count() == 0) {
      return std::move(operand);
    }
    exp->add_operand(std::move(operand));
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
    if (!read_token("print")) return nullptr;
    if (!read_token("*")) return nullptr;
    if (!read_token(",")) return nullptr;
    print_stmt->add_element(parse_expression());
    while (read_token(",")) {
      print_stmt->add_element(parse_expression());
    }
    return std::move(print_stmt);
  }
  std::unique_ptr<If_statement> parse_if_stmt()
  {
    if (!read_token("if")) return nullptr;
    if (!read_token("(")) return nullptr;
    std::unique_ptr<Expression> expr = parse_expression();
    if (!read_token(")")) {
      error("\")\" is expected in IF statement", err_kind::character);
    }
    std::unique_ptr<Executable_construct> action_stmt = parse_action_stmt();
    return std::make_unique<If_statement>(std::move(expr), std::move(action_stmt));
  }
  std::unique_ptr<Executable_construct> parse_action_stmt()
  {
    skip_blank_lines();
    std::unique_ptr<Executable_construct> exec;
    if ((exec = parse_assignment_stmt())) return std::move(exec);
    if ((exec = parse_print_stmt())) return std::move(exec);
    // TODO: if文かif構文かこれだけではわからないはず
    if ((exec = parse_if_stmt())) return std::move(exec);
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
