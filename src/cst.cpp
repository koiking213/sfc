#include "cst.hpp"
#include <iostream>
#include <typeinfo>

namespace cst {
  void Program::print()
  {
    std::cout << this->name << std::endl;
    for (int i=0; i<this->specifications.size(); i++) {
      this->specifications[i]->print("  ");
    }
    for (int i=0; i<this->executable_constructs.size(); i++) {
      this->executable_constructs[i]->print("  ");
    }
  }

  void Type_specification::print(std::string indent)
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

  void Assignment_statement::print(std::string indent)
  {
    std::cout << indent << "lhs: ";
    this->lhs->print();
    std::cout << std::endl;

    std::cout << indent << "rhs: ";
    this->rhs->print();
    std::cout << std::endl;
  }
  void Variable::print()
  {
    std::cout << this->name;
  }
  void Constant::print()
  {
    std::cout << "(" << this->type_name << ")" << this->value;
  }
  void Expression::print()
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
  void Print_statement::print(std::string indent)
  {
    std::cout << indent << "PRINT statement: ";
    for (auto &elm : this->elements) {
      elm->print();
    }
    std::cout << std::endl;
  
  }
  void If_statement::print(std::string indent)
  {
    std::cout << indent << "IF statement: " << "(";
    logical_expr->print();
    std::cout << ")" << std::endl;
    action_stmt->print(indent + "  ");
  }
  void Block::print(std::string indent)
  {
    for (auto& construct : this->execution_part_constructs) {
      construct->print(indent + "  ");
    }
  }
  void Do_with_do_variable::print(std::string indent)
  {
    std::cout << indent << "DO construct: " << this->construct_name << std::endl;
      
    std::cout << indent + "  ";
    this->do_variable->print();
    std::cout << "=";
    this->start_expr->print();
    std::cout << ",";
    this->end_expr->print();
    if (this->stride_expr) {
      std::cout << ",";
      this->stride_expr->print();
    }
    std::cout << std::endl;
    
    std::cout << indent << "statements:" << std::endl;
    this->block->print(indent + "  ");
  }
}
