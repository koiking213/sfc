#include "parser.hpp"
#include "ast.hpp"
#include <set>

static std::unique_ptr<std::map<std::string, std::shared_ptr<ast::Variable>>> current_variable_table;
static std::unique_ptr<std::map<std::string, std::shared_ptr<ast::Type>>> current_type_table;

namespace cst {
  template<typename TO, typename FROM>
  std::unique_ptr<TO> static_unique_pointer_cast (std::unique_ptr<FROM>&& old){
    return std::unique_ptr<TO>{static_cast<TO*>(old.release())};
    //conversion: unique_ptr<FROM>->FROM*->TO*->unique_ptr<TO>
  }

  bool is_binary_operator(std::string op)
  {
    static std::set<std::string> binary_ops{"+", "-", "*", "/", "==", "/=", "<", "<=", ">", ">="};
    return binary_ops.find(op) != binary_ops.end();
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
    std::shared_ptr<ast::Variable> var = (*current_variable_table)[this->name];
    if (!var) {
      var = std::make_shared<ast::Variable>(this->name);
      (*current_variable_table)[this->name] = var;
    }
    std::unique_ptr<ast::Variable_reference> var_ref { new ast::Variable_reference(var) };
    return static_unique_pointer_cast<ast::Expression>(std::move(var_ref));
  }
  std::unique_ptr<ast::Expression> Constant::ASTgen()
  {
    if (this->type_kind == Type_kind::Intrinsic) {
      if (this->type_name == "integer") {
        std::unique_ptr<ast::Int32_constant> cnt { new ast::Int32_constant(std::stoi(this->value)) };
        return static_unique_pointer_cast<ast::Expression>(std::move(cnt));
      } else if (this->type_name == "real") {
        std::unique_ptr<ast::FP32_constant> cnt { new ast::FP32_constant(std::strtod(this->value.c_str(), nullptr)) };
        return static_unique_pointer_cast<ast::Expression>(std::move(cnt));
      } else if (this->type_name == "logical") {
        std::unique_ptr<ast::Logical_constant> cnt;
        if (this->value == ".true.") {
          cnt = std::make_unique<ast::Logical_constant>(true);
        } else if (this->value == ".false.") {
          cnt = std::make_unique<ast::Logical_constant>(false);
        } else {
          assert(0);
        }
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
    std::unique_ptr<ast::Expression> exp = nullptr;
    // for iでうまく行ったら以下で行く?
    // auto &op = this->operators.rbegin();
    // auto &operand = this->operand.rbegin();
    // for (; op != this->operators.rend(); op++, operand++)
    // そもそも再帰で書きたい
    
    if (is_binary_operator(this->operators[0])) {
      for (int i=0; i<this->operators.size(); i++) {
        ast::binary_op_kind op;
        if (this->operators[i] == "+") {
          op = ast::binary_op_kind::add;
        } else if (this->operators[i] == "-") {
          op = ast::binary_op_kind::sub;
        } else if (this->operators[i] == "*") {
          op = ast::binary_op_kind::mul;
        } else if (this->operators[i] == "/") {
          op = ast::binary_op_kind::div;
        } else if (this->operators[i] == "==") {
          op = ast::binary_op_kind::eq;
        } else if (this->operators[i] == "/=") {
          op = ast::binary_op_kind::ne;
        } else if (this->operators[i] == "<") {
          op = ast::binary_op_kind::lt;
        } else if (this->operators[i] == "<=") {
          op = ast::binary_op_kind::le;
        } else if (this->operators[i] == ">") {
          op = ast::binary_op_kind::gt;
        } else if (this->operators[i] == ">=") {
          op = ast::binary_op_kind::ge;
        }  else {
          std::cout << "internal error: unknown operator" << std::endl;
          assert(0);
        }
        // 三項演算子で書き直せる？
        std::unique_ptr<ast::Expression> lhs;
        if (exp) {
          lhs = std::move(exp);
        } else {
          lhs = this->operands[i]->ASTgen();
        }
        std::unique_ptr<ast::Expression> rhs = this->operands[i+1]->ASTgen();
        if (lhs->get_type() != rhs->get_type()) {
          // cast
          if (lhs->get_type() == ast::Type_kind::fp32 &&
              rhs->get_type() == ast::Type_kind::i32) {
            rhs = std::make_unique<ast::Unary_op>(ast::unary_op_kind::i32tofp32, std::move(rhs));
          } else if (lhs->get_type() == ast::Type_kind::i32 &&
                     rhs->get_type() == ast::Type_kind::fp32) {
            lhs = std::make_unique<ast::Unary_op>(ast::unary_op_kind::i32tofp32, std::move(lhs));
          } else {
            // error message should be output
            assert(0);
          }
        }
        exp = std::make_unique<ast::Binary_op>(op, std::move(lhs), std::move(rhs));
      }
    } else if (is_unary_operator(this->operators[0])) {
      assert(false);
    } else {
      assert(false);
    }
    return std::move(exp);
  }

  std::unique_ptr<ast::Statement> Do_with_do_variable::ASTgen()
  {
    // auto 使える？
    std::unique_ptr<ast::Assignment_statement> initial_expr { new ast::Assignment_statement() };
    {
      initial_expr->set_lhs(std::move(this->do_variable->ASTgen_definition()));
      initial_expr->set_rhs(std::move(this->start_expr->ASTgen()));
    }
    
    std::unique_ptr<ast::Assignment_statement> increment_expr { new ast::Assignment_statement() };
    {
      std::unique_ptr<ast::Expression> increment_val;
      if (this->stride_expr) {
        increment_val = this->stride_expr->ASTgen();
      } else {
        increment_val = std::make_unique<ast::Int32_constant>(1);
      }
      std::unique_ptr<ast::Expression> increment_expr_rhs { new ast::Binary_op
          (ast::binary_op_kind::add,
           this->do_variable->ASTgen(),
           std::move(increment_val)) };
      increment_expr->set_lhs(this->do_variable->ASTgen_definition());
      increment_expr->set_rhs(std::move(increment_expr_rhs));
    }

    std::unique_ptr<ast::Binary_op> condition_expr
      = std::make_unique<ast::Binary_op>(ast::binary_op_kind::le,
                                         this->do_variable->ASTgen(),
                                         this->end_expr->ASTgen());

    return std::make_unique<ast::Do_construct> (std::move(initial_expr),
                                                std::move(increment_expr),
                                                std::move(condition_expr),
                                                this->block->ASTgen());
  }

  std::unique_ptr<ast::Block> Block::ASTgen()
  {
    std::unique_ptr<ast::Block> ast_block { new ast::Block() };
    for (auto &construct : this->execution_part_constructs) {
      ast_block->add_statement(construct->ASTgen());
    }
    return std::move(ast_block);
  }
  
  std::unique_ptr<ast::Statement> Assignment_statement::ASTgen()
  {
    std::unique_ptr<ast::Assignment_statement> stmt { new ast::Assignment_statement() };
    stmt->set_lhs(this->lhs->ASTgen_definition());
    stmt->set_rhs(this->rhs->ASTgen());
    return std::move(stmt);
  }
  
  std::shared_ptr<ast::Program_unit> Program::ASTgen()
  {
    std::shared_ptr<ast::Program_unit> program_unit { new ast::Program_unit(this->name) };
    current_variable_table = std::make_unique<std::map<std::string, std::shared_ptr<ast::Variable>>>();
    current_type_table = std::make_unique<std::map<std::string, std::shared_ptr<ast::Type>>>();
    for (auto &spec : this->specifications) {
      spec->ASTgen(program_unit);
    }
    for (auto &exec : this->executable_constructs) {
      program_unit->add_statement(exec->ASTgen());
    }
    program_unit->set_variables(std::move(current_variable_table));
    program_unit->set_types(std::move(current_type_table));
    return program_unit;
  }

  std::shared_ptr<ast::Type> get_or_create_type(cst::Type_kind type_kind, std::string type_name)
  {
    if ((*current_type_table)[type_name]) {
      return (*current_type_table)[type_name];
    }
    
    assert(type_kind == cst::Type_kind::Intrinsic);
    std::shared_ptr<ast::Type> result;
    if (type_name == "integer") {
      result = std::make_shared<ast::Type>(ast::Type_kind::i32);
    } else if (type_name == "real") {
      result = std::make_shared<ast::Type>(ast::Type_kind::fp32);
    }
    (*current_type_table)[type_name] = result;
    return result;
  }
  
  void Type_specification::ASTgen(std::shared_ptr<ast::Program_unit> program)
  {
    std::shared_ptr<ast::Type> type = get_or_create_type(this->type_kind, this->type_name);
    for (std::string name : this->variables) {
      std::shared_ptr<ast::Variable> var = (*current_variable_table)[name];
      if (!var) {
        var = std::make_shared<ast::Variable>(name);
        (*current_variable_table)[name] = var;
      }
      var->set_type(type);
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

  // if文とif構文の違いはASTで吸収する
  std::unique_ptr<ast::Statement> If_statement::ASTgen()
  {
    std::unique_ptr<ast::If_construct> ast_if_construct {
      new ast::If_construct(this->logical_expr->ASTgen()) };
    ast_if_construct->add_then_stmt(this->action_stmt->ASTgen());
    return static_unique_pointer_cast<ast::Statement>(std::move(ast_if_construct));
  }
}


