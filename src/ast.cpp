#include "ast.hpp"
namespace ast {
  std::string unary_op_to_string(const unary_op_kind op)
  {
    switch (op) {
    case unary_op_kind::i32tofp32:
      return "(fp32)";
    }
  }
  std::string binary_op_to_string(const binary_op_kind op)
  {
    switch (op) {
    case binary_op_kind::add:
      return "+";
    case binary_op_kind::sub:
      return "-";
    case binary_op_kind::mul:
      return "*";
    case binary_op_kind::div:
      return "/";
    case binary_op_kind::eq:
      return "==";
    case binary_op_kind::ne:
      return "!=";
    case binary_op_kind::lt:
      return "<";
    case binary_op_kind::le:
      return "<=";
    case binary_op_kind::gt:
      return ">";
    case binary_op_kind::ge:
      return ">=";
    }
  }
  std::string type_to_string(const enum Type_kind kind)
  {
    switch (kind) {
    case Type_kind::logical:
      return "logical";
    case Type_kind::i32:
      return "i32";
    case Type_kind::i64:
      return "i64";
    case Type_kind::fp32:
      return "fp32";
    case Type_kind::fp64:
      return "fp64";
    case Type_kind::pointer:
      return "pointer";
    }
  }
  void Unary_op::print() const
  {
    std::cout << unary_op_to_string(this->exp_operator);
    this->operand->print();
    std::cout << std::endl;
  }
  void Binary_op::print() const
  {
    this->lhs->print();
    std::cout << binary_op_to_string(this->exp_operator);
    this->rhs->print();
    std::cout << std::endl;
  }
  void Int32_constant::print() const
  {
    std::cout << "(int32)" << this->value;
  }
  void FP32_constant::print() const
  {
    std::cout << "(real32)" << this->value;
  }
  void Logical_constant::print() const
  {
    std::cout << "(logical)" << this->value;
  }
  void Variable_reference::print() const
  {
    std::cout << this->var->get_name();
  }
  void Variable_definition::print() const
  {
    std::cout << this->get_var_name();
  }
  void Array_element_reference::print() const
  {
    std::cout << this->var->get_name() << "(";
    this->index->print();
    std::cout << ")";
  }
  void Array_element_definition::print() const
  {
    std::cout << this->var->get_name() << "(";
    this->index->print();
    std::cout << ")";
  }
  void Assignment_statement::print(std::string indent) const
  {
    std::cout << indent << "Assignment statement:";
    std::cout << indent << "  ";
    this->lhs->print();
    std::cout << " = ";
    this->rhs->print();
    std::cout << std::endl;
  }
  void Output_statement::print(std::string indent) const
  {
    std::cout << indent << "Output statement:";
    for (auto &elm : this->elements) {
      elm->print();
      std::cout << ", ";
    }
  }
  void If_construct::print(std::string indent) const
  {
    std::cout << indent << "If construct:" << std::endl;
    std::cout << indent + "  " << "condition: ";
    this->condition_expression->print();
    std::cout << std::endl;
    std::cout << indent + "  " << "statements in then block:" << std::endl;
    this->then_block.print(indent + "  ");
    std::cout << indent + "  " << "statements in else block:" << std::endl;
    this->else_block.print(indent + "  ");
  }
  void Do_construct::print(std::string indent) const
  {
    std::cout << indent << "Do construct:" << std::endl;
    std::cout << indent + "  " << "initial_expr:";
    this->initial_expr->print(indent);
    std::cout << std::endl;
    if (this->increment_expr) {
      std::cout << indent + "  " << "increment_expr:";
      this->increment_expr->print(indent);
      std::cout << std::endl;
    }
    if (this->condition_expr) {
      std::cout << indent + "  " << "condition_expr:";
      this->condition_expr->print();
      std::cout << std::endl;
    }
    std::cout << indent + "  " << "block:" << std::endl;
    this->block->print(indent);
  }
  void Program_unit::print(std::string indent) const
  {
    std::cout << indent << "ProgramUnit:" << this->name << std::endl;
    // TODO: リストを出力する処理は共通化したい
    std::cout << indent << "  " << "variables:" << std::endl;
    for (auto var : *this->variables) {
      var.second->print(indent + "  ");
    }
    std::cout << std::endl;
    
    std::cout << indent << "  " << "statements:" << std::endl;;
    for (auto &stmt : this->statements) {
      stmt->print(indent + "  ");
    }
    std::cout << std::endl;

    std::cout << indent << "  " << "internal programs:" << std::endl;
    for (auto &internal_program : this->internal_programs) {
      internal_program->print(indent + "  ");
    }
    std::cout << std::endl;
  }
  void Variable::print(std::string indent) const
  {
    std::cout << indent << "name: " << this->name;
    std::cout << ", type: ";
    this->type->print(); // TODO: <<演算子の実装
    if (this->shape) {
      std::cout << ", shape: ";
      this->shape->print();
    }
  }
  void Shape::print() const
  {
    for (int i=0; i<this->lower_bounds.size(); i++) {
      std::cout << this->lower_bounds[i] << ":" << this->upper_bounds[i] << " ";
    }
  }
  void Type::print() const
  {
    std::cout << type_to_string(this->type_kind);
  }
  void Block::print(std::string indent) const
  {
    for (auto &stmt : this->statements) {
      stmt->print(indent + "  ");
    }
  }

  enum Type_kind Unary_op::get_type() const
  {
    if (this->exp_operator == unary_op_kind::i32tofp32) {
      assert(this->operand->get_type() == Type_kind::i32);
      return Type_kind::fp32;
    }
    return operand->get_type();
  }
  enum Type_kind Binary_op::get_type() const
  {
    if (this->exp_operator == binary_op_kind::eq ||
        this->exp_operator == binary_op_kind::ne ||
        this->exp_operator == binary_op_kind::lt ||
        this->exp_operator == binary_op_kind::le ||
        this->exp_operator == binary_op_kind::gt ||
        this->exp_operator == binary_op_kind::ge)
      {
        return Type_kind::logical;
      }
    enum Type_kind l = lhs->get_type();
    enum Type_kind r = rhs->get_type();
    if (l==r) return l;
    assert(0);
  }
  int Binary_op::eval_constant_value() const
  {
    int lval = this->lhs->eval_constant_value();
    int rval = this->rhs->eval_constant_value();
    switch (this->exp_operator) {
    case binary_op_kind::add:
      return lval + rval;
    case binary_op_kind::sub:
      return lval - rval;
    case binary_op_kind::mul:
      return lval * rval;
    case binary_op_kind::div:
      return lval / rval;
    default:
      assert(0);
    }
  }
  int Shape::get_size() const
  {
    int sum=1;
    for (int i=0; i<this->lower_bounds.size(); i++) {
      sum = sum * this->get_size(i);
    }
    return sum;
  }
}
