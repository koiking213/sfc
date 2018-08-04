#pragma once
#include <iostream>
#include <string>
#include "llvm/IR/IRBuilder.h"

namespace ast {
  enum class binary_op_kind {
    add, sub, mul, div,
    eq, ne, lt, le, gt, ge
  };

  enum class unary_op_kind {
    i32tofp32
  };

  enum class Type_kind : int {
    logical,
    i32,
    i64,
    fp32,
    fp64,
    pointer
  };
  
  class Type {
  public:
    Type(Type_kind type_kind) : type_kind(type_kind) {}
    void print() const;
    Type_kind get_type_kind() const {return type_kind;} ;
  private:
    enum Type_kind type_kind;
  };

  class Variable {
  public:
    Variable(std::string name) : name(name) {}
    void print(std::string indent) const;
    void codegen() const;
    std::string get_name() const {return name;}
    Type_kind get_type_kind() const {return type->get_type_kind();}
    void set_type(std::shared_ptr<Type> type){this->type = type;}
  private:
    std::string name;
    std::shared_ptr<Type> type;
    int64_t element_num=0;
  };
  
  class Expression {
  public:
    virtual void print() const = 0;
    virtual llvm::Value *codegen() const = 0;
    virtual enum Type_kind get_type() const = 0;
  };

  class Binary_op : public Expression {
  public:
    Binary_op(binary_op_kind op, std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs) : exp_operator(op) {
      this->lhs = std::move(lhs);
      this->rhs = std::move(rhs);
    }
    void set_operator(binary_op_kind op) {this->exp_operator = op;}
    void set_lhs(std::unique_ptr<Expression> lhs) {this->lhs = std::move(lhs);}
    void set_rhs(std::unique_ptr<Expression> rhs) {this->rhs = std::move(rhs);}
    void print() const;
    llvm::Value *codegen() const;
    enum Type_kind get_type() const;
  private:
    binary_op_kind exp_operator;
    std::unique_ptr<Expression> lhs;
    std::unique_ptr<Expression> rhs;
  };

  class Unary_op : public Expression {
  public:
    Unary_op(unary_op_kind op, std::unique_ptr<Expression> elm) : exp_operator(op) {
      this->operand = std::move(elm);
    }
    void set_operator(unary_op_kind op) {this->exp_operator = op;}
    void set_operand(std::unique_ptr<Expression> elm) {this->operand = std::move(elm);}
    void print() const;
    llvm::Value *codegen() const;
    enum Type_kind get_type() const;
  private:
    unary_op_kind exp_operator;
    std::unique_ptr<Expression> operand;
  };
  
  class Constant : public Expression {
  public:
    virtual void print() const = 0;
    virtual llvm::Value *codegen() const = 0;
    virtual Type_kind get_type() const = 0;
  private:
    enum Type_kind type_kind;
  };

  class Int32_constant : public Constant {
  public:
    Int32_constant(int32_t val) {this->value = val;}
    void print() const;
    llvm::Value *codegen() const;
    int32_t get_value() const {return value;}
    Type_kind get_type() const {return Type_kind::i32;};
  private:
    int32_t value;
  };

  class FP32_constant : public Constant {
  public:
    FP32_constant(float val) {this->value = val;}
    void print() const;
    llvm::Value *codegen() const;
    float get_value() const {return value;}
    Type_kind get_type() const {return Type_kind::fp32;};
  private:
    float value;
  };

  class Logical_constant : public Constant {
  public:
    Logical_constant(bool val) : value(val) {};
    void print() const;
    llvm::Value *codegen() const;
    bool get_value() const {return value;}
    Type_kind get_type() const {return Type_kind::logical;}
    int get_int_value() const {return 1 ? value : 0;}
  private:
    bool value;
  };

  class Variable_reference : public Expression {
  public:
    void print() const;
    llvm::Value *codegen() const;
    Variable_reference(std::shared_ptr<Variable> var) {this->var = var;}
    Type_kind get_type() const {return var->get_type_kind();}
  private:
    std::shared_ptr<Variable> var;
  };

  class Variable_definition {
  public:
    void print() const;
    llvm::Value *codegen() const;
    Variable_definition(std::string name) {this->name = name;}
  private:
    enum Type_kind type_kind;
    std::string name;
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
    void add_statement(std::unique_ptr<Statement> stmt) {this->statements.push_back(std::move(stmt));};
    void add_internal_program(std::unique_ptr<Program_unit>);
    void set_variables(std::unique_ptr<std::map<std::string, std::shared_ptr<Variable>>> table) {this->variables = std::move(table);}
    void set_types(std::unique_ptr<std::map<std::string, std::shared_ptr<Type>>> table) {this->types = std::move(table);}
  private:
    std::string name;
    std::vector<std::unique_ptr<Statement>> statements;
    std::vector<std::shared_ptr<Program_unit>> internal_programs;
    std::unique_ptr<std::map<std::string, std::shared_ptr<Variable>>> variables;
    std::unique_ptr<std::map<std::string, std::shared_ptr<Type>>> types;
  };
}
