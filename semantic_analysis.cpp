#include "parser.hpp"
#include "ast.hpp"

namespace cst {
  template<typename TO, typename FROM>
  std::unique_ptr<TO> static_unique_pointer_cast (std::unique_ptr<FROM>&& old){
    return std::unique_ptr<TO>{static_cast<TO*>(old.release())};
    //conversion: unique_ptr<FROM>->FROM*->TO*->unique_ptr<TO>
  }
  
  bool is_binary_operator(std::string op)
  {
    if (op == "+" || op == "-" || op == "*" || op == "/") {
      return true;
    }
    return false;
  }
  bool is_unary_operator(std:: string op)
  {
    return false;
  }
  std::unique_ptr<ast::Variable_definition> Variable::ASTgen_definition()
  {
    std::unique_ptr<ast::Variable_definition> var { new ast::Variable_definition(this->name) };
    return var;
  }
  std::unique_ptr<ast::Expression> Variable::ASTgen()
  {
    std::unique_ptr<ast::Variable_reference> var { new ast::Variable_reference(this->name) };
    return static_unique_pointer_cast<ast::Expression>(std::move(var));
  }
  std::unique_ptr<ast::Expression> Constant::ASTgen()
  {
    if (this->type_kind == Type_kind::Intrinsic) {
      if (this->type_name == "integer") {
	std::unique_ptr<ast::Int32_constant> cnt { new ast::Int32_constant(std::stoi(this->value)) };
	return static_unique_pointer_cast<ast::Expression>(std::move(cnt));
      } else {
	assert(false);
      }
    } else {
      assert(false);
    }
    return nullptr;
  }
  std::unique_ptr<ast::Expression> Expression::ASTgen()
  {
    std::unique_ptr<ast::Expression> exp { new ast::Expression() };
    if (this->exp_operator == "+") {
      exp->set_operator(ast::operators::add);
    } else if (this->exp_operator == "-") {
      exp->set_operator(ast::operators::sub);
    } else if (this->exp_operator == "*") {
      exp->set_operator(ast::operators::mul);
    } else if (this->exp_operator == "/") {
      exp->set_operator(ast::operators::div);
    }  else if (this->exp_operator == "") {
      exp->set_operator(ast::operators::leaf);
    }
    if (is_binary_operator(this->exp_operator)) {
      exp->set_lhs(this->operands[0]->ASTgen());
      exp->set_rhs(this->operands[1]->ASTgen());
    } else if (is_unary_operator(this->exp_operator)) {
      assert(false);
    } else {
      assert(false);
    }
    return std::move(exp);
  }
  
  std::unique_ptr<ast::Statement> Assignment_statement::ASTgen()
  {
    std::unique_ptr<ast::Assignment_statement> stmt { new ast::Assignment_statement() };
    stmt->set_lhs(this->lhs->ASTgen_definition());
    stmt->set_rhs(this->rhs->ASTgen());
    return static_unique_pointer_cast<ast::Statement>(std::move(stmt));
  }
  
  std::shared_ptr<ast::Program_unit> Program::ASTgen()
  {
    std::shared_ptr<ast::Program_unit> program_unit { new ast::Program_unit(this->name) };
    for (auto &spec : this->specifications) {
      spec->ASTgen(program_unit);
    }
    for (auto &exec : this->executable_constructs) {
      program_unit->add_statement(exec->ASTgen());
    }
    return program_unit;
  }

  void Type_specification::ASTgen(std::shared_ptr<ast::Program_unit> program)
  {
    assert(this->type_kind == cst::Type_kind::Intrinsic);
    ast::Type_kind ast_type_kind;
    if (this->type_name == "integer") {
      ast_type_kind = ast::Type_kind::i32;
    } else if (this->type_name == "real") {
      ast_type_kind = ast::Type_kind::fp32;
    }
    for (std::string name : this->variables) {
      std::unique_ptr<ast::Variable> var { new ast::Variable(name, ast_type_kind) };
      program->add_variable_decl(std::move(var));
    }
  }

  std::unique_ptr<ast::Statement> Print_statement::ASTgen()
  {
    std::unique_ptr<ast::Output_statement> ast_output_stmt { new ast::Output_statement() };
    for (auto &elm : this->elements) {
      ast_output_stmt->add_element(elm->ASTgen());
    }
    return static_unique_pointer_cast<ast::Statement>(std::move(ast_output_stmt));
  }

  // std::unique_ptr<ast::Variable_definition> Variable::ASTgen_definition()
  // {
  //   return std::move(new ast::Variable_definition(this->name));
  // }
}


