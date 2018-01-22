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

namespace ast {
  enum struct operators {
    add, sub, mul, div
  };

  template <operators Op>
  struct binary_op;

  char const* to_string(operators op)
  {
    switch(op) {
    case operators::add: return "+";
    case operators::sub: return "-";
    case operators::mul: return "*";
    case operators::div: return "/";
    default: return nullptr;
    }
  }

  using Expression = boost::variant<
    int,
    std::string,
    boost::recursive_wrapper< binary_op< operators::add > >,
    boost::recursive_wrapper< binary_op< operators::sub > >,
    boost::recursive_wrapper< binary_op< operators::mul > >,
    boost::recursive_wrapper< binary_op< operators::div > >
    >;

  template <operators Op>
  struct binary_op
  {
    Expression lhs;
    Expression rhs;
    
    binary_op(Expression const& lhs_, Expression const& rhs_) :
      lhs( lhs_ ), rhs( rhs_ )
    { }
  };

  struct stringizer : public boost::static_visitor<std::string> {
    std::string operator()(int const constant) const
    {
      return std::to_string(constant);
    }
    std::string operator()(std::string const var) const
    {
      return var;
    }
    template<operators Binary_op>
    std::string operator()(binary_op<Binary_op> const& op) const
    {
      return "("
    	 + boost::apply_visitor(stringizer(), op.lhs)
    	 + to_string(Binary_op)
    	 + boost::apply_visitor(stringizer(), op.rhs)
    	 + ")";
    }
  };
  
   std::string stringize(Expression const& expr)
   {
     return boost::apply_visitor(stringizer(), expr);
   }

  template<operators Op>
  auto make_binary_operator()
  {
    return boost::phoenix::bind(
				[](auto const& lhs, auto const& rhs){
				  return binary_op<Op>(lhs, rhs);
				}, boost::spirit::_val, boost::spirit::_1 );
  }
}


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

struct Assignment_statement {
  ast::Expression lhs;
  ast::Expression rhs;
};

using Executable_construct = boost::variant<
  Assignment_statement
  >;

struct Subroutine {

};

struct Program {
  std::string name;
  //  int program_kind; // enum
  std::vector<Specification> specifications;
  std::vector<Executable_construct> executable_constructs;
  Subroutine subroutines_head;
};

BOOST_FUSION_ADAPT_STRUCT (
			   Program,
			   (std::string, name)
			   (std::vector<Specification>, specifications)
			   (std::vector<Executable_construct>, executable_constructs)
			   //			   (Subroutine, subroutines_head)
			   )


BOOST_FUSION_ADAPT_STRUCT (
			   Type_specification,
			   (enum Type_kind, type_kind)
			   (std::string, type_name)
			   (std::vector<std::string>, variables)
			   )

BOOST_FUSION_ADAPT_STRUCT (
			   Parameter_statement,
			   (std::vector<Named_constant_definition>, named_constants)
			   )

BOOST_FUSION_ADAPT_STRUCT (
			   Assignment_statement,
			   (ast::Expression, lhs)
			   (ast::Expression, rhs)
			   )

template <typename Iterator>
struct test_parser : qi::grammar<Iterator, Program(), ascii::blank_type>
{
  test_parser() : test_parser::base_type(start)
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
    action_stmt = assignment_stmt.alias();

    // todo
    assignment_stmt =
      variable[at_c<0>(_val) = _1]
      >> lit("=")
      >> expr[at_c<1>(_val) = _1] >> eol;

    //assignment_stmt = variable >> lit("=") >> varia >> eol;
    
    // todo
    variable = name.alias();

    // todo
    expr = add_operand [_val = _1]
      >> *(
	   ('+' >> add_operand [_val = phx::construct<ast::binary_op<ast::operators::add> >(_val, _1)])
	    //[_val = ast::make_binary_operator<ast::operators::add>()])
	   | ('-' >> add_operand [_val = phx::construct<ast::binary_op<ast::operators::add> >(_val, _1)])
	   //[_val = ast::make_binary_operator<ast::operators::sub>()])
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
      | (lit("(") >> expr >> lit(")"));
    
  
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

  qi::rule<Iterator, std::string()> name, variable;
  qi::rule<Iterator, std::string()> program_stmt, module_stmt;
  qi::rule<Iterator, std::vector<Specification>(), ascii::blank_type> specification_part;
  qi::rule<Iterator, Specification(), ascii::blank_type> use_stmt, declaration_construct;
  qi::rule<Iterator, Type_specification(), ascii::blank_type> type_declaration_stmt, declaration_type_spec;
  qi::rule<Iterator, ast::Expression(), ascii::blank_type> expr, level_2_expr, add_operand, mult_operand, level_1_expr, primary;
  qi::rule<Iterator, Parameter_statement(), ascii::blank_type> parameter_stmt;
  qi::rule<Iterator, std::vector<Executable_construct>(), ascii::blank_type> execution_part;
  qi::rule<Iterator, Executable_construct(), ascii::blank_type> executable_construct, action_stmt;
  qi::rule<Iterator, Assignment_statement(), ascii::blank_type> assignment_stmt;
  qi::rule<Iterator, Subroutine(), ascii::blank_type> internal_subprogram_part;
  qi::rule<Iterator, std::string(), ascii::blank_type> end_program_stmt, end_module_stmt;
  qi::rule<Iterator> blank;
  qi::rule<Iterator, Program(), ascii::blank_type> start;
  qi::rule<Iterator, Program(), ascii::blank_type> main_program, module;
};

void print_type_declaration_statement(Type_specification &s)
{
  std::cout << "type declaration statement, typename=";
  if (s.type_kind == Type_kind::Intrinsic) {
    std::cout << s.type_name << std::endl;
  } else {
    std::cout << "not supported yet" << std::endl;
  }
  std::cout << "variables:" ;
  for (auto v : s.variables) {
    std::cout << v << ","; // last comma should be removed
  }
  std::cout << std::endl;
}

void print_specification(Specification &spec)
{
  if (spec.type() == typeid(Type_specification)) {
    print_type_declaration_statement(boost::get<Type_specification>(spec));
  } else if (spec.type() == typeid(Parameter_statement)) {
    //print_parameter_statement(boost::get<Type_specification>(spec));
  } else {
    std::cout << "unknown specification" << std::endl;
  }
}

void print_assignment_statement(Assignment_statement &st)
{
  std::cout << "lhs:" << ast::stringize(st.lhs) << std::endl;
  std::cout << "rhs:" << ast::stringize(st.rhs) << std::endl;
}

void print_executable_construct(Executable_construct &exec)
{
  if (exec.type() == typeid(Assignment_statement)) {
    print_assignment_statement(boost::get<Assignment_statement>(exec));
  } else {
    std::cout << "unknown executable construct" << std::endl;
  }
}

void print_program(Program &p)
{
  std::cout << "program name: " << p.name << std::endl;
  std::cout << "specifications:" << std::endl;
  for(auto spec : p.specifications) {
    print_specification(spec);
  }
  for(auto exec : p.executable_constructs) {
    print_executable_construct(exec);
  }
}

int main()
{
  test_parser<std::string::const_iterator> parser;
  std::string str;
  std::cin.unsetf(std::ios::skipws);
  std::copy(std::istream_iterator<char>(std::cin), std::istream_iterator<char>(), std::back_inserter(str));
  {
    // remove_space(str);
    std::string::const_iterator it = str.begin(), end = str.end();
    Program program;
    bool r = phrase_parse(it, end, parser, qi::ascii::blank, program);
    if (r && it == end) {
      std::cout << "succeeded:\t" << std::endl;
    }
    else {
      std::cout << "failed:\t" << std::string(it, end) << std::endl;
    }
    print_program(program);
  }
  return 0;
}
