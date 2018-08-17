#pragma once
#include <iostream>
#include <string>
#include "llvm/IR/IRBuilder.h"

namespace ast {

  class Expression;
  
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

  class Bound {
  public:
    Bound(std::unique_ptr<Expression> lower, std::unique_ptr<Expression> upper)
      : lower(std::move(lower)), upper(std::move(upper)) {}
    const Expression &get_lower() const {return *lower;}
    const Expression &get_upper() const {return *upper;}
    void print() const;
  private:
    std::unique_ptr<Expression> lower;
    std::unique_ptr<Expression> upper;
  };

  class Shape {
  public:
    int get_size(int i) const;
    int get_size() const;
    Shape(std::vector<std::unique_ptr<Bound>> bounds) : bounds(std::move(bounds)) {}
    void print() const;
    const Expression &get_lower_bound(int index) const {return bounds[index]->get_lower();}
    const Expression &get_upper_bound(int index) const {return bounds[index]->get_upper();}
    int get_rank() const {return bounds.size();}
    
  private:
    std::vector<std::unique_ptr<Bound>> bounds;
  };

  class Variable {
  public:
    Variable(std::string name) : name(name) {}
    void print(std::string indent) const;
    void codegen() const;
    std::string get_name() const {return name;}
    Type_kind get_type_kind() const {return type->get_type_kind();}
    void set_type(std::shared_ptr<Type> type){this->type = type;}
    void set_shape(std::unique_ptr<Shape> shape) {this->shape = std::move(shape);}
    const Shape& get_shape() const {return *shape;}
  private:
    std::string name;
    std::shared_ptr<Type> type;
    std::unique_ptr<Shape> shape;
  };

  class Expression {
  public:
    virtual int eval_constant_value() const = 0;
    virtual void print() const = 0;
    virtual llvm::Value *codegen() const = 0;
    virtual enum Type_kind get_type() const = 0;
    virtual bool is_constant_int() const = 0;
    virtual std::unique_ptr<Expression> get_copy() const = 0;
    virtual ~Expression() {};
  };

  class Binary_op : public Expression {
  public:
    Binary_op(binary_op_kind op, std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
      : exp_operator(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    void print() const;
    llvm::Value *codegen() const;
    enum Type_kind get_type() const;
    int eval_constant_value() const;
    bool is_constant_int() const;
    std::unique_ptr<Expression> get_copy() const {
      return std::make_unique<Binary_op>(exp_operator,
                                         lhs->get_copy(),
                                         rhs->get_copy());
    };
  private:
    binary_op_kind exp_operator;
    std::unique_ptr<Expression> lhs;
    std::unique_ptr<Expression> rhs;
  };

  class Unary_op : public Expression {
  public:
    Unary_op(unary_op_kind op, std::unique_ptr<Expression> elm)
      : exp_operator(op), operand(std::move(elm)) {}
    void print() const;
    llvm::Value *codegen() const;
    enum Type_kind get_type() const;
    int eval_constant_value() const {assert(0);};
    bool is_constant_int() const {return operand->is_constant_int();};
    std::unique_ptr<Expression> get_copy() const {
      return std::make_unique<Unary_op>(exp_operator, operand->get_copy());
    }
  private:
    unary_op_kind exp_operator;
    std::unique_ptr<Expression> operand;
  };
  
  class Constant : public Expression {
  public:
    virtual void print() const = 0;
    virtual llvm::Value *codegen() const = 0;
    virtual Type_kind get_type() const = 0;
    virtual int eval_constant_value() const = 0;
    virtual std::unique_ptr<Expression> get_copy() const = 0;
    virtual ~Constant() {};
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
    int eval_constant_value() const {return value;};
    bool is_constant_int() const {return true;};
    std::unique_ptr<Expression> get_copy() const {return std::make_unique<Int32_constant>(value);}
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
    int eval_constant_value() const {return (int)value;};
    bool is_constant_int() const {return false;};
    std::unique_ptr<Expression> get_copy() const {return std::make_unique<FP32_constant>(value);}
  private:
    float value;
  };

  class Logical_constant : public Constant { // 別のclassにするよりtemplateを検討する
  public:
    Logical_constant(bool val) : value(val) {};
    void print() const;
    llvm::Value *codegen() const;
    bool get_value() const {return value;}
    Type_kind get_type() const {return Type_kind::logical;}
    int get_int_value() const {return 1 ? value : 0;}
    int eval_constant_value() const {return (int)value;};
    bool is_constant_int() const {return false;};
    std::unique_ptr<Expression> get_copy() const {return std::make_unique<Logical_constant>(value);}
  private:
    bool value;
  };

  class Variable_reference : public Expression {
  public:
    virtual void print() const;
    virtual llvm::Value *codegen() const;
    Variable_reference(std::shared_ptr<Variable> var) : var(var) {};
    Type_kind get_type() const {return var->get_type_kind();}
    int eval_constant_value() const {assert(0);};
    bool is_constant_int() const {return false;};
    std::string get_var_name() const {return var->get_name();}
    virtual std::unique_ptr<Expression> get_copy() const {return std::make_unique<Variable_reference>(var);}
  protected:
    std::shared_ptr<Variable> var;
    Variable_reference() {};
  };

  class Array_element_reference : virtual public Variable_reference {
  public:
    void print() const;
    virtual llvm::Value *codegen() const;
    Array_element_reference(std::shared_ptr<Variable> var,
                            std::vector<std::unique_ptr<Expression>> indices)
      : Variable_reference(var), indices(std::move(indices)) {
      calc_offset_expr();
    }
    std::unique_ptr<Expression> get_copy() const {
      std::vector<std::unique_ptr<Expression>> new_indices;
      for (auto &index : this->indices) {
        new_indices.push_back(index->get_copy());
      }
      return std::make_unique<Array_element_reference>(var, std::move(new_indices));
    }
  protected:
    std::vector<std::unique_ptr<Expression>> indices;
    std::unique_ptr<Expression> offset_expr;
    void calc_offset_expr();
    Array_element_reference() {};
  };

  class Variable_definition : virtual public Variable_reference {
  public:
    virtual llvm::Value *codegen() const;
    Variable_definition(std::shared_ptr<Variable> var) : Variable_reference(var) {}
  };

  class Array_element_definition : public Variable_definition,
                                   public Array_element_reference {
  public:
    llvm::Value *codegen() const;
    Array_element_definition(std::shared_ptr<Variable> var,
                             std::vector<std::unique_ptr<Expression>> indices)
      : Variable_reference(var), Variable_definition(var), Array_element_reference(var, std::move(indices)) {}
  };
  
  class Statement {
  public:
    virtual void print(std::string indent) const = 0;
    virtual void codegen() const = 0;
    virtual ~Statement() {};
  };

  class Assignment_statement : public Statement {
  public:
    Assignment_statement(std::unique_ptr<Variable_definition> lhs,
                         std::unique_ptr<Expression> rhs)
      : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    void print(std::string indent) const;
    void codegen() const;
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

  class Construct : public Statement {
    // is there some feature all construct have?
  public:
    virtual void print(std::string indent) const = 0;
  };

  class Block {
  public:
    void add_statement(std::unique_ptr<Statement> stmt) {statements.push_back(std::move(stmt));}
    void print (std::string indent) const;
    void codegen() const;
  private:
    std::vector<std::unique_ptr<Statement>> statements;

  };

  class Do_construct : public Construct {
  public:
    void print(std::string indent) const;
    void codegen() const;
    void add_statement(std::unique_ptr<Statement> stmt) {block->add_statement(std::move(stmt));}
    Do_construct(std::unique_ptr<Assignment_statement> initial_expr,
                 std::unique_ptr<Assignment_statement> increment_expr,
                 std::unique_ptr<Expression> condition_expr,
                 std::unique_ptr<Block> block) {
      this->initial_expr = std::move(initial_expr);
      this->increment_expr = std::move(increment_expr);
      this->condition_expr = std::move(condition_expr);
      this->block = std::move(block);
    }
  private:
    std::unique_ptr<Block> block;
    std::unique_ptr<Assignment_statement> initial_expr;
    std::unique_ptr<Assignment_statement> increment_expr;
    std::unique_ptr<Expression> condition_expr;
  };

  class If_construct : public Construct {
  public:
    If_construct(std::unique_ptr<Expression> expr) {
      condition_expression = std::move(expr);
    }
    void print(std::string indent) const;
    void codegen() const;
    void add_then_stmt(std::unique_ptr<Statement> stmt) {then_block.add_statement(std::move(stmt));};
    void add_else_stmt(std::unique_ptr<Statement> stmt) {else_block.add_statement(std::move(stmt));};
  private:
    std::unique_ptr<Expression> condition_expression;
    Block then_block; // TODO: std::unique_ptrにする
    Block else_block; // TODO: std::unique_ptrにする
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
