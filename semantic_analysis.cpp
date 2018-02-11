#include "parser.hpp"
#include "ast.hpp"

namespace parser {
  ast::ProgramUnit *Program::ASTgen()
  {
    auto program_unit = new ast::ProgramUnit();
    program_unit->name = this->name;
    for (auto spec : this->specifications) {
      if (spec.type() == typeid(Type_specification)) {
	auto variables = boost::get<Type_specification>(spec).ASTgen();
	std::copy(variables.begin(), variables.end(),
		  std::back_inserter(program_unit->variable_declarations));
      }
    }
    for (auto exec : this->executable_constructs) {
      if (exec.type() == typeid(ast::Assignment_statement)) {
	auto stmt = boost::get<ast::Assignment_statement>(exec);
	program_unit->statements.push_back(stmt);
      }
    }
    return program_unit;
  }

  std::vector<ast::Variable *> Type_specification::ASTgen()
  {
    assert(this->type_kind == parser::Type_kind::Intrinsic);
    ast::Type_kind ast_type_kind;
    if (this->type_name == "integer") {
      ast_type_kind = ast::Type_kind::i32;
    } else if (this->type_name == "real") {
      ast_type_kind = ast::Type_kind::fp32;
    }
    std::vector<ast::Variable *> ast_variables;
    for (auto name : this->variables) {
      auto var = new ast::Variable(name, ast_type_kind);
      ast_variables.push_back(var);
    }
    return ast_variables;
  }
}


