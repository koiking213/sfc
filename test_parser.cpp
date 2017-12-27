#include <iostream>
#include <string>
#include <boost/spirit/include/support_utree.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

namespace qi = boost::spirit::qi;
namespace spirit = boost::spirit;
namespace ascii = boost::spirit::qi::ascii;

struct AST_specification {

};

struct AST_executable_statement {

};

struct AST_subroutine {

};

struct AST_program {
  std::string name;
  //  int program_kind; // enum
  AST_specification variables_head;
  AST_executable_statement executable_statements_head;
  AST_subroutine subroutines_head;
};

BOOST_FUSION_ADAPT_STRUCT (
			   AST_program,
			   (std::string, name)
			   //			   (AST_specification, variables_head)
			   //			   (AST_executable_statement, executable_statements_head)
			   //			   (AST_subroutine, subroutines_head)
			   )

template <typename Iterator>
struct test_parser : qi::grammar<Iterator, AST_program(), ascii::space_type>
{
  test_parser() : test_parser::base_type(start)
  {
    using qi::int_;
    using qi::lit;
    using qi::double_;
    using qi::lexeme;
    using ascii::char_;
    using boost::spirit::qi::_1;
    using boost::spirit::qi::_val;

    name = qi::ascii::alpha >> *(qi::ascii::alpha | qi::digit | qi::char_('_'));

    start %=
      main_program | module;

    main_program %=
      program_stmt
      //      >> specification_part
      //      >> execution_part
      //      >> internal_subprogram_part
      >> end_program_stmt
      ;
    
    program_stmt = lit("program") >> +qi::ascii::blank >> name [_val = _1];

    end_program_stmt =
      (lit("end") >> lit("program") >> name) [_val = _1] |
      (lit("end") >> -(lit("program")))      [_val = ""];

    end_module_stmt =
      lit("end") >>
      -(+qi::ascii::blank >> lit("module") >> -(+qi::ascii::blank >> name[_val = _1]));


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
  qi::rule<Iterator, AST_specification(), ascii::space_type> specification_part;
  qi::rule<Iterator, AST_executable_statement(), ascii::space_type> execution_part;
  qi::rule<Iterator, AST_subroutine(), ascii::space_type> internal_subprogram_part;
  qi::rule<Iterator, std::string(), ascii::space_type> end_program_stmt, end_module_stmt;
  qi::rule<Iterator> blank;
  qi::rule<Iterator, AST_program(), ascii::space_type> start;
  qi::rule<Iterator, AST_program(), ascii::space_type> main_program, module;
};

void print_program(AST_program p)
{
  std::cout << "program name:" << p.name << std::endl;
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
    AST_program program;
    bool r = phrase_parse(it, end, parser, qi::ascii::space, program);
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
