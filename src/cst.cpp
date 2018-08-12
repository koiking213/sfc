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
  this->lhs->print();
  std::cout << std::endl;

  std::cout << indent << "rhs: ";
  this->rhs->print();
  std::cout << std::endl;
}
void cst::Variable::print()
{
  std::cout << this->name;
}
void cst::Constant::print()
{
  std::cout << "(" << this->type_name << ")" << this->value;
}
void cst::Expression::print()
{
  // もっといい感じに書けないか
  std::cout << "(";
  for (int i=0; i<this->operators.size(); i++) {
    this->operands[i]->print();
    std::cout << this->operators[i];
  }
  (*(this->operands.end()))->print();
  std::cout << ")";
}
void cst::Print_statement::print(std::string indent)
{
  std::cout << indent << "PRINT statement: ";
  for (auto &elm : this->elements) {
    elm->print();
  }
  std::cout << std::endl;
  
}
void cst::If_statement::print(std::string indent)
{
  std::cout << indent << "IF statement: " << "(";
  logical_expr->print();
  std::cout << ")" << std::endl;
  action_stmt->print(indent + "  ");
}
