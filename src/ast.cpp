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
  }
  void Binary_op::print() const
  {
    this->lhs->print();
    std::cout << binary_op_to_string(this->exp_operator);
    this->rhs->print();
  }
  void Int32_constant::print() const
  {
    std::cout << this->value;
  }
  void FP32_constant::print() const
  {
    std::cout << this->value;
  }
  void Logical_constant::print() const
  {
    std::cout << this->value;
  }
  void Variable_reference::print() const
  {
    std::cout << this->var->get_name();
  }
  void Array_element_reference::print() const
  {
    std::cout << this->var->get_name() << "(";
    this->indices[0]->print();
    for (int i=1; i<indices.size(); i++) {
      std::cout << ",";
      this->indices[i]->print();
    }
    std::cout << ")";
  }
  void Assignment_statement::print(std::string indent) const
  {
    std::cout << indent;
    this->lhs->print();
    std::cout << " = ";
    this->rhs->print();
    std::cout << std::endl;
  }
  void Output_statement::print(std::string indent) const
  {
    std::cout << indent << "Output statement: ";
    this->elements[0]->print();
    for (int i=1; i<this->elements.size(); i++) {
      std::cout << ", ";
      this->elements[i]->print();
    }
    std::cout << std::endl;
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
    std::cout << indent + "  " << "initial_expr:  ";
    this->initial_expr->print("");
    if (this->increment_expr) {
      std::cout << indent + "  " << "increment_expr: ";
      this->increment_expr->print("");
    }
    if (this->condition_expr) {
      std::cout << indent + "  " << "condition_expr: ";
      this->condition_expr->print();
      std::cout << std::endl;
    }
    std::cout << indent + "  " << "block:" << std::endl;
    this->block->print(indent + "  ");
  }
  void Program_unit::print(std::string indent) const
  {
    std::cout << indent << "ProgramUnit:" << this->name << std::endl;
    std::cout << indent << "  " << "variables:" << std::endl;
    for (auto var : *this->variables) {
      var.second->print(indent + "    ");
      std::cout << std::endl;
    }
    
    std::cout << indent << "  " << "statements:" << std::endl;;
    for (auto &stmt : this->statements) {
      stmt->print(indent + "    ");
    }

    if (this->internal_programs.size() >= 1) {
      std::cout << indent << "  " << "internal programs:" << std::endl;
      for (auto &internal_program : this->internal_programs) {
        internal_program->print(indent + "    ");
      }
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
    std::cout << this->lower_bounds[0] << ":" << this->upper_bounds[0];
    for (int i=1; i<this->lower_bounds.size(); i++) {
      std::cout << " " << this->lower_bounds[i] << ":" << this->upper_bounds[i];
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
  bool Binary_op::is_constant_int() const
  {
    if (this->get_type() != Type_kind::i32) return false;
    return lhs->is_constant_int() && rhs->is_constant_int();
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

  // これを常に呼ぶというインターフェースはあまり良くない気がするが...
  // Array_element_definitionのindicesの持ち方を変更して、コンストラクタでやる？
  void Array_element_reference::calc_offset_expr()
  {
    // DEBUG
    // this->offset_expr = std::make_unique<Int32_constant>(1);
    //   //std::move(this->indices[0]);
    // return;
    // END DEBUG
    assert(!this->offset_expr);
    std::unique_ptr<Expression> expr;
    // integer :: a(10,3,2)
    // a(i,j,k)のoffsetは(k*3+j)*10+i
    expr = this->indices[indices.size()-1]->get_copy();
    for (int i=indices.size()-2; i>=0; i--) {
      // mult
      std::unique_ptr<Expression> lower = this->var->get_shape().get_lower_bound_expr(i).get_copy();
      expr = std::make_unique<Binary_op>(binary_op_kind::mul, std::move(expr), std::move(lower));
      // add
      std::unique_ptr<Expression> subscript = this->indices[i]->get_copy();
      expr = std::make_unique<Binary_op>(binary_op_kind::add, std::move(expr), std::move(subscript));
    }
    this->offset_expr = std::move(expr);
  }
}
