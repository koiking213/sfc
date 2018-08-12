#include "cst.hpp"
#include <iostream>
#include <typeinfo>

void cst::Program::print()
{
  std::cout << this->name << std::endl;
  for (int i=0; i<this->specifications.size(); i++) {
    this->specifications[i]->print("  ");
  }
  for (int i=0; i<this->executable_constructs.size(); i++) {
    this->executable_constructs[i]->print("  ");
  }
}

void cst::Type_specification::print(std::string indent)
{
  std::cout << indent;
  if (this->type_kind == Type_kind::Intrinsic) {
    std::cout << this->type_name << ": ";
  }
  for (std::string v:this->variables) {
    std::cout << v << " ";
  }
  std::cout << std::endl;
}

void cst::Assignment_statement::print(std::string indent)
{
  std::cout << indent << "lhs: ";
  this->lhs->print(indent);
  std::cout << std::endl;

  std::cout << indent << "rhs: ";
  this->rhs->print(indent);
  std::cout << std::endl;
}
void cst::Variable::print(std::string indent)
{
  std::cout << this->name;
}
void cst::Constant::print(std::string indent)
{
  std::cout << "(" << this->type_name << ")" << this->value;
}
void cst::Expression::print(std::string indent)
{
  std::cout << "(" << this->exp_operator;
  for (auto &operand : this->operands) {
    std::cout << " ";
    operand->print(indent + "  ");
  }
  std::cout << ")";
}
void cst::Print_statement::print(std::string indent)
{
  std::cout << indent << "PRINT statement: ";
  for (auto &elm : this->elements) {
    elm->print(indent);
  }
  std::cout << std::endl;
  
}
void cst::If_statement::print(std::string indent)
{
  std::cout << indent << "IF statement: " << "(";
  logical_expr->print(indent + "  ");
  std::cout << ")" << std::endl;
  action_stmt->print(indent + "  ");
}
