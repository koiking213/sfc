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

namespace ast {
  struct add;
  struct sub;
  struct mul;
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

class Type_specification {
public:
  enum Type_kind type_kind;
  std::string type_name;
  std::vector<std::string> variables;
};

class Named_constant_definition {
public:
  std::string named_constant;
  //  Expression exp;
};

class Parameter_statement {
public:
  std::vector<Named_constant_definition> named_constants;
};

using Specification = boost::variant<
  Type_specification,
  Parameter_statement
  >;


struct Executable_statement {

};

struct Subroutine {

};

struct Program {
  std::string name;
  //  int program_kind; // enum
  std::vector<Specification> specifications;
  Executable_statement executable_statements_head;
  Subroutine subroutines_head;
};

BOOST_FUSION_ADAPT_STRUCT (
			   Program,
			   (std::string, name)
			   (std::vector<Specification>, specifications)
			   //			   (Executable_statement, executable_statements_head)
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
    using boost::spirit::qi::_val;
    using boost::phoenix::at_c;
    using boost::phoenix::push_back;

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
      //      >> execution_part
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

  qi::rule<Iterator, std::string()> name;
  qi::rule<Iterator, std::string()> program_stmt, module_stmt;
  qi::rule<Iterator, std::vector<Specification>(), ascii::blank_type> specification_part;
  qi::rule<Iterator, Specification(), ascii::blank_type> use_stmt, declaration_construct;
  qi::rule<Iterator, Type_specification(), ascii::blank_type> type_declaration_stmt, declaration_type_spec;
  qi::rule<Iterator, Parameter_statement(), ascii::blank_type> parameter_stmt;
  qi::rule<Iterator, Executable_statement(), ascii::blank_type> execution_part;
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

void print_specification(Specification &s)
{
  std::string kind;
  if (s.type() == typeid(Type_specification)) {
    print_type_declaration_statement(boost::get<Type_specification>(s));
  } else if (s.type() == typeid(Parameter_statement)) {
    //print_parameter_statement(boost::get<Type_specification>(s));
  } else {
    std::cout << "unknown specification" << std::endl;
  }
}

void print_program(Program &p)
{
  std::cout << "program name: " << p.name << std::endl;
  std::cout << "specifications:" << std::endl;
  for(auto s : p.specifications) {
    print_specification(s);
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
