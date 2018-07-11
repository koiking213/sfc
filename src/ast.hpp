#pragma once
#include <iostream>
#include <string>
#include "llvm/IR/IRBuilder.h"

namespace ast {
  enum class operators {
    add, sub, mul, div, leaf
  };

  class Expression {
  public:
    void set_operator(enum operators op) {this->exp_operator = op;}
    void set_lhs(std::unique_ptr<Expression> lhs) {this->lhs = std::move(lhs);}
    void set_rhs(std::unique_ptr<Expression> rhs) {this->rhs = std::move(rhs);}
    virtual void print() const;
    virtual llvm::Value *codegen() const;
  private:
    enum operators exp_operator;
    std::unique_ptr<Expression> lhs;
    std::unique_ptr<Expression> rhs;
  };
  
  enum class Type_kind : int {
    logical,
    i32,
    i64,
    fp32,
    fp64,
    pointer
  };

  class Constant : public Expression {
  public:
    virtual void print() const = 0;
    virtual llvm::Value *codegen() const = 0;
  private:
    enum Type_kind type_kind;
  };

  class Int32_constant : public Constant {
  public:
    Int32_constant(int32_t val) {this->value = val;}
    void print() const;
    llvm::Value *codegen() const;
    int32_t get_value() {return value;}
  private:
    int32_t value;
  };

  class Variable_reference : public Expression {
  public:
    // TODO: この時点で変数のmapを持っておいてそれを指すほうが良さそう?
    void print() const;
    llvm::Value *codegen() const;
    Variable_reference(std::string name) {this->name = name;}
  private:
    std::string name;
  };

  class Variable_definition {
  public:
    void print() const;
    llvm::Value *codegen() const;
    Variable_definition(std::string name) {this->name = name;}
  private:
    std::string name;
  };
  
  class Type {
  public:
    Type(Type_kind type_kind) : type_kind(type_kind) {}
    void print() const;
  private:
    enum Type_kind type_kind;
  };

  class Variable {
  public:
    Variable(std::string name, Type_kind type_kind) : name(name), type(type_kind) {}
    void print(std::string indent) const;
    void codegen() const;
  private:
    std::string name;
    Type type;
    int64_t element_num=0;
  };

  class Statement {
  public:
    virtual void print(std::string indent) const = 0;
    virtual void codegen() const = 0;
    virtual ~Statement() {};
  };

  class Assignment_statement : public Statement {
  public:
    void print(std::string indent) const;
    void codegen() const;
    void set_lhs(std::unique_ptr<Variable_definition> lhs) {this->lhs = std::move(lhs);}
    void set_rhs(std::unique_ptr<Expression> rhs) {this->rhs = std::move(rhs);}
  private:
    std::unique_ptr<Variable_definition> lhs;
    std::unique_ptr<Expression> rhs;
  };

  class Output_statement : public Statement {
  public:
    void print(std::string indent) const;
    void codegen() const;
    void add_element(std::unique_ptr<Expression> elm) {this->elements.push_back(std::move(elm));};
  private:
    std::vector<std::unique_ptr<Expression>> elements;
  };

  class Program_unit {
  public:
    void print(std::string indent) const;
    void codegen() const;
    Program_unit(std::string name) {this->name = name; }
    void add_variable_decl(std::unique_ptr<Variable> var) {this->variable_declarations.push_back(std::move(var));};
    void add_statement(std::unique_ptr<Statement> stmt) {this->statements.push_back(std::move(stmt));};
    void add_internal_program(std::unique_ptr<Program_unit>);
    //std::map<std::string, llvm::Value *>
  private:
    std::string name;
    std::vector<std::unique_ptr<Statement>> statements;
    std::vector<std::shared_ptr<Program_unit>> internal_programs;
    std::vector<std::unique_ptr<Variable>> variable_declarations; // vector以外にいいのがあるかも
  };
}
