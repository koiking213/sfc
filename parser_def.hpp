#pragma once
#include "parser.hpp"
#include "ast.hpp"

namespace parser {
  template <typename Iterator>
  test_parser<Iterator>::test_parser() : test_parser::base_type(start)
  {
      using qi::int_;
      using qi::lit;
      using qi::double_;
      using qi::lexeme;
      using qi::eol;
      using ascii::char_;
      using boost::spirit::qi::_1;
      using boost::spirit::qi::_2;
      using boost::spirit::qi::_val;
      using boost::phoenix::at_c;
      using boost::phoenix::push_back;
      namespace phx = boost::phoenix;

      bool fixed=false;
      if (fixed) {
	blank = "";
      } else {
	blank = +qi::ascii::blank;
      }
    
      name = qi::ascii::alpha >> *(qi::ascii::alpha | qi::digit | qi::char_('_'));

      start %=
	main_program | module;

      main_program %=
	-program_stmt
	>> specification_part
	>> execution_part
	//      >> internal_subprogram_part
	>> end_program_stmt
	;
    
      program_stmt = lit("program") >> blank >> name [_val = _1] >> eol;

      end_program_stmt =
	(lit("end") >> lit("program") >> name >> eol) [_val = _1] |
	(lit("end") >> -(lit("program")) >> eol)      [_val = ""];

      end_module_stmt =
	lit("end") >>
	-(blank >> lit("module") >> -(blank >> name[_val = _1]));

      // todo
      specification_part =
	/* *use_stmt                 [push_back(_val, _1)]
	   >>*/ *declaration_construct [push_back(_val, _1)];

      // todo
      execution_part = +executable_construct;
    
      // todo
      executable_construct = action_stmt.alias();

      // todo
      action_stmt =
	assignment_stmt |
	print_stmt;

      // todo
      assignment_stmt =
	variable[at_c<0>(_val) = _1]
	>> lit("=")
	>> expr[at_c<1>(_val) = _1] >> eol;

      // todo
      print_stmt =
	lit("print") >> lit("*") >> lit(",") >>
	variable[push_back(at_c<0>(_val), _1)] % qi::char_(',') >> eol;
    
      // todo
      variable = name.alias();

      // todo
      expr = add_operand [_val = _1]
	>> *(
	     ('+' >> add_operand [_val = phx::construct<ast::binary_op<ast::operators::add> >(_val, _1)])
	     | ('-' >> add_operand [_val = phx::construct<ast::binary_op<ast::operators::add> >(_val, _1)])
	     );
      add_operand = mult_operand [_val = _1]
	>> *(
	     ('*' >> mult_operand [_val = phx::construct<ast::binary_op<ast::operators::mul> >(_val, _1)])
	     | ('/' >> mult_operand [_val = phx::construct<ast::binary_op<ast::operators::div> >(_val, _1)])
	     );
      mult_operand = level_1_expr.alias();
      level_1_expr = primary.alias();
      primary =
	name
	| constant
	| (lit("(") >> expr >> lit(")"));

      // todo
      constant = int_literal_constant.alias();
      int_literal_constant %= qi::int_;
  
      // todo
      //use_stmt = lit("use statement") [at_c<0>(_val) = Specification_kind::Use_statement] >> eol;

      // todo
      //declaration_construct = lit("declaration construct") [at_c<0>(_val) = Specification_kind::Implicit_statement];
      declaration_construct = type_declaration_stmt.alias(); /*| parameter_stmt*/
      type_declaration_stmt =
	declaration_type_spec [_val = _1]
	>> name [push_back(at_c<2>(_val), _1)] % qi::char_(',')>> eol;
  
      // todo
      declaration_type_spec = lit("integer") [at_c<0>(_val) = Type_kind::Intrinsic,
					      at_c<1>(_val) = "integer"];

      // todo
      //parameter_stmt = lit("parameter") [at_c<0>(_val) = Specification_kind::Parameter_statement];
  
      // todo
      module %=
	module_stmt
	>> specification_part
	>> execution_part
	>> internal_subprogram_part
	>> end_module_stmt
	;

      module_stmt %=
	lit("module")
	>> name
	;
    }
}
