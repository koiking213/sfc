#pragma once
#include <string>
#include <vector>
#include <memory>
#include "ast.hpp"

namespace cst {

  
  // TODO: operatorとexpressionを分離
  class Expression {
  public:
    virtual void print() const = 0;
    virtual std::unique_ptr<ast::Expression> ASTgen() const = 0;
  };

  class Operator : public Expression {
  public:
    virtual void print() const;
    void add_operand(std::unique_ptr<Expression> operand) {operands.push_back(std::move(operand));}
    void add_operator(std::string str) {operators.push_back(str);}
    int get_operator_count() const {return operators.size();}
    virtual std::unique_ptr<ast::Expression> ASTgen() const;
  private:
    std::vector<std::unique_ptr<Expression>> operands;
    std::vector<std::string> operators;
  };
  
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
    virtual void print(std::string indent) const = 0 ;
    virtual void ASTgen(std::shared_ptr<ast::Program_unit> program) const = 0;
    virtual ~Specification() {};
  };

  class Array_spec {
  public:
    virtual void print() const = 0;
    virtual std::unique_ptr<ast::Shape> ASTgen() const = 0;
    virtual ~Array_spec() {};
  };

  class Explicit_shape_spec : public Array_spec {
  public:
    void add_spec(std::unique_ptr<Expression> lower, std::unique_ptr<Expression> upper) {
      this->lower_bounds.push_back(std::move(lower));
      this->upper_bounds.push_back(std::move(upper));
    }
    std::unique_ptr<ast::Shape> ASTgen() const;
    void print() const;
  private:
    std::vector<std::unique_ptr<Expression>> lower_bounds;
    std::vector<std::unique_ptr<Expression>> upper_bounds;
  };

  class Dimension_spec {
  public:
    Dimension_spec(std::string array_name, std::unique_ptr<Array_spec> array_spec) : array_name(array_name) {
      this->array_spec = std::move(array_spec);
    }
    void print(std::string indent) const;
    std::string get_array_name() {return array_name;}
    std::unique_ptr<ast::Shape> ASTgen() const {return array_spec->ASTgen();};
  private:
    std::string array_name;
    std::unique_ptr<Array_spec> array_spec;
  };

  class Dimension_statement : public Specification {
  public:
    void print(std::string indent) const;
    void ASTgen(std::shared_ptr<ast::Program_unit> program) const;
    void add_spec(std::unique_ptr<Dimension_spec> spec) {this->specs.push_back(std::move(spec));}
  private:
    std::vector<std::unique_ptr<Dimension_spec>> specs;
  };

  class Type_specification : public Specification {
  public:
    void print(std::string indent) const;
    void ASTgen(std::shared_ptr<ast::Program_unit> program) const;
    Type_specification(enum Type_kind kind, std::string name, std::unique_ptr<Expression> len=nullptr) : type_kind(kind), type_name(name), len(std::move(len)) {};
    void add_variable(std::string var) {variables.push_back(var);}
  private:
    std::unique_ptr<Expression> len;
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
    virtual void print(std::string indent) const = 0 ;
    virtual std::unique_ptr<ast::Statement> ASTgen() const = 0;
    virtual ~Executable_construct() {};
  };


  class Print_statement : public Executable_construct {
  public:
    void print(std::string indent) const;
    std::unique_ptr<ast::Statement> ASTgen() const;
    void add_element(std::unique_ptr<Expression> elm) {elements.push_back(std::move(elm));}
  private:
    std::vector<std::unique_ptr<Expression>> elements;
  };

  class If_statement : public Executable_construct {
  public:
    If_statement(std::unique_ptr<Expression> expr, std::unique_ptr<Executable_construct> stmt) {
      logical_expr = std::move(expr);
      action_stmt = std::move(stmt);
    }
    void print(std::string indent) const;
    std::unique_ptr<ast::Statement> ASTgen() const;
  private:
    std::unique_ptr<Executable_construct> action_stmt;
    std::unique_ptr<Expression> logical_expr;
  };

  class Block {
  public:
    void print(std::string indent) const;
    void add_construct(std::unique_ptr<Executable_construct> construct) {
      execution_part_constructs.push_back(std::move(construct));
    }
    std::unique_ptr<ast::Block> ASTgen() const;
  private:
    std::vector<std::unique_ptr<Executable_construct>> execution_part_constructs;
  };


  class Variable : public Expression {
  public:
    Variable(std::string name) : name(name) {};
    virtual void print() const;
    virtual std::unique_ptr<ast::Expression> ASTgen() const;
    virtual std::unique_ptr<ast::Variable_definition> ASTgen_definition() const;
  protected:
    std::string name;
  };

  class Array_element : public Variable {
  public:
    Array_element(std::string name, std::vector<std::unique_ptr<Expression>> subscripts)
      : Variable(name), subscripts(std::move(subscripts)) {}
    void print() const;
    std::unique_ptr<ast::Expression> ASTgen() const;
    std::unique_ptr<ast::Variable_definition> ASTgen_definition() const;
  private:
    std::vector<std::unique_ptr<Expression>> subscripts;
  };

  class Constant : public Expression {
  public:
    void print() const;
    Constant(enum Type_kind kind, std::string name, std::string value) : type_kind(kind), type_name(name), value(value) {};
    std::unique_ptr<ast::Expression> ASTgen() const;
  private:
    enum Type_kind type_kind;
    std::string type_name;
    std::string value;
  };

  
  class Do_construct : public Executable_construct {
  public:
    Do_construct(std::string name) : construct_name(name) {}
    virtual void print(std::string indent) const = 0;
    void set_block(std::unique_ptr<Block> block) {this->block = std::move(block);}
    std::string get_construct_name() {return construct_name;}
  protected:
    std::string construct_name;
    std::unique_ptr<Block> block;
  };

  class Do_with_do_variable : public Do_construct {
  public:
    Do_with_do_variable(std::string construct_name,
                        std::unique_ptr<Variable> do_variable,
                        std::unique_ptr<Expression> start_expr,
                        std::unique_ptr<Expression> end_expr,
                        std::unique_ptr<Expression> stride_expr) : Do_construct(construct_name)
    {
      this->do_variable = std::move(do_variable);
      this->start_expr = std::move(start_expr);
      this->end_expr = std::move(end_expr);
      this->stride_expr = std::move(stride_expr);
    }
    void print(std::string indent) const;
    std::unique_ptr<ast::Statement> ASTgen() const;
  private:
    std::unique_ptr<Variable> do_variable;
    std::unique_ptr<Expression> start_expr;
    std::unique_ptr<Expression> end_expr;
    std::unique_ptr<Expression> stride_expr;
  };

  class Assignment_statement : public Executable_construct {
  public:
    Assignment_statement(std::unique_ptr<Variable> lhs,
                         std::unique_ptr<Expression> rhs)
      : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    void print(std::string indent) const;
    std::unique_ptr<ast::Statement> ASTgen() const;
  private:
    std::unique_ptr<Variable> lhs;
    std::unique_ptr<Expression> rhs;
  };

  class Subroutine {

  };

  class Program {
  public:
    Program(std::string str) { name = str; };
    void print() const;
    std::shared_ptr<ast::Program_unit> ASTgen() const;
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
