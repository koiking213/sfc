#pragma once
#include "ast.hpp"
#include <iostream>
#include <string>
#include <boost/spirit/include/support_utree.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>

namespace parser {
  namespace qi = boost::spirit::qi;
  namespace spirit = boost::spirit;
  namespace ascii = boost::spirit::qi::ascii;

  enum class Type_kind : int {
    Intrinsic,
    derived_type,
    polymorphic,
    unlimited_polymorphic
  };

  enum class Specification_kind : int {
    Use_statement,
    Import_statement,
    Implicit_statement,
    Parameter_statement,
    Format_statement,
    Entry_statement,
    Derived_type_definition,
    Enum_definition,
    Interface_block,
    Procedure_declaration_statement,
    Type_declaration_statement,
    Statement_function_statement,
    // OTHER
  };

  struct Type_specification {
    enum Type_kind type_kind;
    std::string type_name;
    std::vector<std::string> variables;
    std::vector<ast::Variable *> ASTgen();
  };

  struct Named_constant_definition {
    std::string named_constant;
    //  Expression exp;
  };

  struct Parameter_statement {
    std::vector<Named_constant_definition> named_constants;
  };

  using Specification = boost::variant<
    Type_specification,
    Parameter_statement
    >;

  struct Print_statement {
    std::vector<std::string> elements;
    ast::Output_statement *ASTgen();
  };

  using Executable_construct = boost::variant<
    ast::Assignment_statement,
    Print_statement
    >;

  struct Subroutine {

  };

  struct Program {
    std::string name;
    //  int program_ki nd; // enum
    std::vector<Specification> specifications;
    std::vector<Executable_construct> executable_constructs;
    Subroutine subroutines_head;
    ast::ProgramUnit *ASTgen();
  };


  Program *do_parse(std::string str, Program &program);
  void print_program(Program &p);

  template <typename Iterator>
  struct test_parser : qi::grammar<Iterator, Program(), ascii::blank_type>
  {
    test_parser();

    qi::rule<Iterator, std::string()> name, variable;
    qi::rule<Iterator, ast::Constant()> constant;
    qi::rule<Iterator, ast::Integer_constant()> int_literal_constant;
    qi::rule<Iterator, std::string()> program_stmt, module_stmt;
    qi::rule<Iterator, std::vector<Specification>(), ascii::blank_type> specification_part;
    qi::rule<Iterator, Specification(), ascii::blank_type> use_stmt, declaration_construct;
    qi::rule<Iterator, Type_specification(), ascii::blank_type> type_declaration_stmt, declaration_type_spec;
    qi::rule<Iterator, ast::Expression(), ascii::blank_type> expr, level_2_expr, add_operand, mult_operand, level_1_expr, primary;
    qi::rule<Iterator, Parameter_statement(), ascii::blank_type> parameter_stmt;
    qi::rule<Iterator, std::vector<Executable_construct>(), ascii::blank_type> execution_part;
    qi::rule<Iterator, Executable_construct(), ascii::blank_type> executable_construct, action_stmt;
    qi::rule<Iterator, ast::Assignment_statement(), ascii::blank_type> assignment_stmt;
    qi::rule<Iterator, Print_statement(), ascii::blank_type> print_stmt;
    qi::rule<Iterator, Subroutine(), ascii::blank_type> internal_subprogram_part;
    qi::rule<Iterator, std::string(), ascii::blank_type> end_program_stmt, end_module_stmt;
    qi::rule<Iterator> blank;
    qi::rule<Iterator, Program(), ascii::blank_type> start;
    qi::rule<Iterator, Program(), ascii::blank_type> main_program, module;
  };

}
 

BOOST_FUSION_ADAPT_STRUCT (
			   parser::Program,
			   (std::string, name)
			   (std::vector<parser::Specification>, specifications)
			   (std::vector<parser::Executable_construct>, executable_constructs)
			   //			   (Subroutine, subroutines_head)
			   )


BOOST_FUSION_ADAPT_STRUCT (
			   parser::Type_specification,
			   (enum parser::Type_kind, type_kind)
			   (std::string, type_name)
			   (std::vector<std::string>, variables)
			   )

BOOST_FUSION_ADAPT_STRUCT (
			   parser::Parameter_statement,
			   (std::vector<parser::Named_constant_definition>, named_constants)
			   )

BOOST_FUSION_ADAPT_STRUCT (
			   ast::Assignment_statement,
			   (ast::Expression, lhs)
			   (ast::Expression, rhs)
			   )

BOOST_FUSION_ADAPT_STRUCT (
			   ast::Integer_constant,
			   (int, value)
			   )

BOOST_FUSION_ADAPT_STRUCT (
			   parser::Print_statement,
			   (std::vector<std::string>, elements)
			   )
