#include "parser.hpp"
#include "ast.hpp"
#include <set>

static std::unique_ptr<std::map<std::string, std::shared_ptr<ast::Variable>>> current_variable_table;
static std::unique_ptr<std::map<std::string, std::shared_ptr<ast::Type>>> current_type_table;
static std::shared_ptr<ast::Program_unit> current_program_unit;

namespace cst {
  template<typename TO, typename FROM>
  std::unique_ptr<TO> static_unique_pointer_cast (std::unique_ptr<FROM>&& old){
    return std::unique_ptr<TO>{static_cast<TO*>(old.release())};
    //conversion: unique_ptr<FROM>->FROM*->TO*->unique_ptr<TO>
  }
  template<typename TO, typename FROM>
  std::shared_ptr<TO> static_shared_pointer_cast (std::shared_ptr<FROM>&& old){
    return std::shared_ptr<TO>{static_cast<TO*>(old.release())};
    //conversion: shared_ptr<FROM>->FROM*->TO*->shared_ptr<TO>
  }

  std::shared_ptr<ast::Variable> get_or_create_var(std::string name) {
    std::shared_ptr<ast::Variable> var = (*current_variable_table)[name];
    if (!var) {
      var = std::make_shared<ast::Variable>(name);
      (*current_variable_table)[name] = var;
    }
    return var;
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
  std::unique_ptr<ast::Variable_definition> Variable::ASTgen_definition() const
  {
    return std::make_unique<ast::Variable_definition>(get_or_create_var(this->name));
  }
  std::unique_ptr<ast::Variable_definition> Array_element::ASTgen_definition() const
  {
    std::shared_ptr<ast::Variable> var = get_or_create_var(this->name);
    std::vector<std::unique_ptr<ast::Expression>> indices;
    const ast::Shape &shape = var->get_shape();
    for (int i=0; i<shape.get_rank(); i++) {
      auto lower = std::make_unique<ast::Int32_constant>(shape.get_lower_bound(i).eval_constant_value());
      auto index = std::make_unique<ast::Binary_op>(ast::binary_op_kind::sub,
                                                    this->subscripts[i]->ASTgen(),
                                                    std::move(lower));
      indices.push_back(std::move(index));
    }
    auto elm_def = std::make_unique<ast::Array_element_definition>(var, std::move(indices));
    return static_unique_pointer_cast<ast::Variable_definition>(std::move(elm_def));
  }
  std::unique_ptr<ast::Expression> Variable::ASTgen() const
  {
    std::shared_ptr<ast::Variable> var = get_or_create_var(this->name);
    std::unique_ptr<ast::Variable_reference> var_ref { new ast::Variable_reference(var) };
    return static_unique_pointer_cast<ast::Expression>(std::move(var_ref));
  }
  std::unique_ptr<ast::Expression> Array_element::ASTgen() const
  {
    std::shared_ptr<ast::Variable> var = get_or_create_var(this->name);
    std::vector<std::unique_ptr<ast::Expression>> indices;
    const ast::Shape &shape = var->get_shape();
    for (int i=0; i<shape.get_rank(); i++) {
      auto lower = std::make_unique<ast::Int32_constant> (shape.get_lower_bound(i).eval_constant_value());
      auto index = std::make_unique<ast::Binary_op>(ast::binary_op_kind::sub,
                                                    this->subscripts[i]->ASTgen(),
                                                    std::move(lower));
      indices.push_back(std::move(index));
    }
    auto elm_ref = std::make_unique<ast::Array_element_reference>(var, std::move(indices));
    return static_unique_pointer_cast<ast::Expression>(std::move(elm_ref));
  }
  std::unique_ptr<ast::Expression> Constant::ASTgen() const
  {
    if (this->type_kind == Type_kind::Intrinsic) {
      if (this->type_name == "integer") {
        auto cnt = std::make_unique<ast::Int32_constant>(std::stoi(this->value));
        return static_unique_pointer_cast<ast::Expression>(std::move(cnt));
      } else if (this->type_name == "real") {
        auto cnt = std::make_unique<ast::FP32_constant>(std::strtod(this->value.c_str(), nullptr));
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
      } else if (this->type_name == "character") {
        current_program_unit->add_global_string(this->value);
        return static_unique_pointer_cast<ast::Expression>(std::make_unique<ast::Character_constant>(this->value));
      } else {
        assert(false);
      }
    } else {
      assert(false);
    }
    return nullptr;
  }
  std::unique_ptr<ast::Expression> Operator::ASTgen() const
  {
    std::unique_ptr<ast::Expression> exp = nullptr;
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
        std::unique_ptr<ast::Expression> lhs = exp ? std::move(exp) : this->operands[i]->ASTgen();
        std::unique_ptr<ast::Expression> rhs = this->operands[i+1]->ASTgen();
        if (lhs->get_type_kind() != rhs->get_type_kind()) {
          // cast
          if (lhs->get_type_kind() == ast::Type_kind::fp32 &&
              rhs->get_type_kind() == ast::Type_kind::i32) {
            rhs = std::make_unique<ast::Unary_op>(ast::unary_op_kind::i32tofp32, std::move(rhs));
          } else if (lhs->get_type_kind() == ast::Type_kind::i32 &&
                     rhs->get_type_kind() == ast::Type_kind::fp32) {
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

  std::unique_ptr<ast::Statement> Do_with_do_variable::ASTgen() const
  {
    auto initial_expr = std::make_unique<ast::Assignment_statement>(this->do_variable->ASTgen_definition(),
                                                                    this->start_expr->ASTgen());

    std::unique_ptr<ast::Assignment_statement> increment_expr;
    {
      std::unique_ptr<ast::Expression> increment_val;
      if (this->stride_expr) {
        increment_val = this->stride_expr->ASTgen();
      } else {
        increment_val = std::make_unique<ast::Int32_constant>(1);
      }
      auto increment_expr_rhs = std::make_unique<ast::Binary_op>(ast::binary_op_kind::add,
                                                                 this->do_variable->ASTgen(),
                                                                 std::move(increment_val));
      increment_expr = std::make_unique<ast::Assignment_statement>(this->do_variable->ASTgen_definition(),
                                                                   std::move(increment_expr_rhs));
    }

    auto condition_expr = std::make_unique<ast::Binary_op>(ast::binary_op_kind::le,
                                                           this->do_variable->ASTgen(),
                                                           this->end_expr->ASTgen());
    return std::make_unique<ast::Do_construct> (std::move(initial_expr),
                                                std::move(increment_expr),
                                                std::move(condition_expr),
                                                this->block->ASTgen());
  }

  std::unique_ptr<ast::Block> Block::ASTgen() const
  {
    auto ast_block = std::make_unique<ast::Block>();
    for (auto &construct : this->execution_part_constructs) {
      ast_block->add_statement(construct->ASTgen());
    }
    return ast_block;
  }
  
  std::unique_ptr<ast::Statement> Assignment_statement::ASTgen() const
  {
    return std::make_unique<ast::Assignment_statement>(this->lhs->ASTgen_definition(),
                                                       this->rhs->ASTgen());
  }
  
  std::shared_ptr<ast::Program_unit> Program::ASTgen() const
  {
    current_program_unit = std::make_shared<ast::Program_unit>(this->name);
    current_variable_table = std::make_unique<std::map<std::string, std::shared_ptr<ast::Variable>>>();
    current_type_table = std::make_unique<std::map<std::string, std::shared_ptr<ast::Type>>>();
    for (auto &spec : this->specifications) {
      spec->ASTgen(current_program_unit);
    }
    for (auto &exec : this->executable_constructs) {
      current_program_unit->add_statement(exec->ASTgen());
    }
    current_program_unit->set_variables(std::move(current_variable_table));
    current_program_unit->set_types(std::move(current_type_table));
    
    return current_program_unit;
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
    } else if (type_name == "logical") {
      result = std::make_shared<ast::Type>(ast::Type_kind::logical);
    } else if (type_name == "character") {
      result = std::make_shared<ast::Type>(ast::Type_kind::character);
    }
    (*current_type_table)[type_name] = result;
    return result;
  }

  std::unique_ptr<ast::Shape> Explicit_shape_spec::ASTgen() const
  {
    std::vector<std::unique_ptr<ast::Bound>> bounds;
    for (int i=0; i<this->lower_bounds.size(); i++) {
      bounds.push_back(std::make_unique<ast::Bound>(lower_bounds[i]->ASTgen(),
                                                    upper_bounds[i]->ASTgen()));
    }
    return std::make_unique<ast::Shape>(std::move(bounds));
  }

  void Dimension_statement::ASTgen(std::shared_ptr<ast::Program_unit> program) const
  {
    for (auto& spec : this->specs) {
      std::shared_ptr<ast::Variable> var = get_or_create_var(spec->get_array_name());
      var->set_shape(std::move(spec->ASTgen()));
      var->set_array_attr();
    }
  }
  
  void Type_specification::ASTgen(std::shared_ptr<ast::Program_unit> program) const
  {
    std::shared_ptr<ast::Type> type = get_or_create_type(this->type_kind, this->type_name);
    for (std::string name : this->variables) {
      std::shared_ptr<ast::Variable> var = get_or_create_var(name);
      var->set_type(type);
      if (this->type_kind == Type_kind::Intrinsic && this->type_name == "character") {
        if (this->len) {
          var->set_len(this->len->ASTgen());
        } else {
          var->set_len(std::make_unique<ast::Int32_constant>(1));
        }
      }
    }
  }

  std::unique_ptr<ast::Statement> Print_statement::ASTgen() const
  {
    std::unique_ptr<ast::Output_statement> ast_output_stmt { new ast::Output_statement() };
    for (auto &elm : this->elements) {
      ast_output_stmt->add_element(elm->ASTgen());
    }
    return static_unique_pointer_cast<ast::Statement>(std::move(ast_output_stmt));
  }

  // if文とif構文の違いはASTで吸収する予定
  std::unique_ptr<ast::Statement> If_statement::ASTgen() const
  {
    std::vector<std::unique_ptr<ast::Statement>> statements;
    statements.push_back(this->action_stmt->ASTgen());
    std::unique_ptr<ast::Block> block { new ast::Block(std::move(statements)) };
    auto ast_if_construct = std::make_unique<ast::If_construct>(this->logical_expr->ASTgen(),
                                                                std::move(block));
    return static_unique_pointer_cast<ast::Statement>(std::move(ast_if_construct));
  }
}


