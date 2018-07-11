#pragma once
#include <string>
#include <vector>
#include <memory>
#include "ast.hpp"

namespace cst {
    enum class Type_kind : int {
    Intrinsic,
    derived_type,
    polymorphic,
    unlimited_polymorphic
  };

  enum class Specification_kind : int {
    Use_statement,
    Import_statement,
    Implicit_statement,
    Parameter_statement,
    Format_statement,
    Entry_statement,
    Derived_type_definition,
    Enum_definition,
    Interface_block,
    Procedure_declaration_statement,
    Type_declaration_statement,
    Statement_function_statement,
    // OTHER
  };

  class Specification {
  public:
    virtual void print(std::string indent) = 0;
    virtual void ASTgen(std::shared_ptr<ast::Program_unit> program) = 0;
    virtual ~Specification() {};
  };

  class Type_specification : public Specification {
  public:
    void print(std::string indent);
    void ASTgen(std::shared_ptr<ast::Program_unit> program);
    Type_specification(enum Type_kind kind, std::string name) : type_kind(kind), type_name(name) {};
    void add_variable(std::string var) {variables.push_back(var);}
  private:
    enum Type_kind type_kind;
    std::string type_name;
    std::vector<std::string> variables;
  };

  class Named_constant_definition {
  private:
    std::string named_constant;
    //  Expression exp;
  };

  class Parameter_statement : public Specification {
  private:
    std::vector<Named_constant_definition> named_constants;
  };

  class Executable_construct {
  public:
    virtual void print(std::string indent) = 0;
    virtual std::unique_ptr<ast::Statement> ASTgen() = 0;
    virtual ~Executable_construct() {};
  };

  class Expression {
  public:
    virtual void print(std::string indent);
    void add_operand(std::unique_ptr<Expression> operand) {operands.push_back(std::move(operand));}
    void set_operator(std::string str) {exp_operator = str;}
    Expression(std::string op) : exp_operator(op) {};
    Expression() : exp_operator("") {};
    virtual std::unique_ptr<ast::Expression> ASTgen();
  private:
    std::vector<std::unique_ptr<Expression>> operands;
    std::string exp_operator;
  };



  class Print_statement : public Executable_construct {
  public:
    void print(std::string indent);
    std::unique_ptr<ast::Statement> ASTgen();
    void add_element(std::unique_ptr<Expression> elm) {elements.push_back(std::move(elm));}
  private:
    std::vector<std::unique_ptr<Expression>> elements;
  };

  class Variable : public Expression {
  public:
    void print(std::string indent);
    Variable(std::string name) : name(name) {};
    std::unique_ptr<ast::Expression> ASTgen();
    std::unique_ptr<ast::Variable_definition> ASTgen_definition();
  private:
    std::string name;
  };

  class Constant : public Expression {
  public:
    void print(std::string indent);
    Constant(enum Type_kind kind, std::string name, std::string value) : type_kind(kind), type_name(name), value(value) {};
    std::unique_ptr<ast::Expression> ASTgen();
  private:
    enum Type_kind type_kind;
    std::string type_name;
    std::string value;
  };

  class Assignment_statement : public Executable_construct {
  public:
    void set_lhs(std::unique_ptr<Variable> lhs) {this->lhs = std::move(lhs);}
    void set_rhs(std::unique_ptr<Expression> rhs) {this->rhs = std::move(rhs);}
    void print(std::string indent);
    std::unique_ptr<ast::Statement> ASTgen();
  private:
    std::unique_ptr<Variable> lhs;
    std::unique_ptr<Expression> rhs;
  };

  class Subroutine {

  };

  class Program {
  public:
    Program(std::string str) { name = str; };
    void print();
    std::shared_ptr<ast::Program_unit> ASTgen();
    void add_specification(std::unique_ptr<Specification> spec) {specifications.push_back(std::move(spec));};
    void add_executable_construct(std::unique_ptr<Executable_construct> exec) {executable_constructs.push_back(std::move(exec));};
    std::string get_name() { return name; }
  private:
    std::string name;
    //  int program_kind; // enum
    std::vector<std::unique_ptr<Specification>> specifications;
    std::vector<std::unique_ptr<Executable_construct>> executable_constructs;
    Subroutine subroutines_head;
  };
}
