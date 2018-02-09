#include "ast.hpp"
#include "parser.hpp"
#include "parser_def.hpp"
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

  void print_assignment_statement(ast::Assignment_statement &st)
  {
    std::cout << "lhs:" << ast::stringize(st.lhs) << std::endl;
    std::cout << "rhs:" << ast::stringize(st.rhs) << std::endl;
  }

  void print_executable_construct(Executable_construct &exec)
  {
    if (exec.type() == typeid(ast::Assignment_statement)) {
      print_assignment_statement(boost::get<ast::Assignment_statement>(exec));
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


  void test()
  {
    test_parser<std::string::const_iterator> parser;
    std::string input="hoge";
    std::string result;
    //test_parser(input, parser.name, result);
    std::string::const_iterator it = input.begin(), end = input.end();
    parse(it, end, parser.name, result);
    std::cout << result << std::endl;
  }

  Program *do_parse(std::string str, Program &program)
  {
    test_parser<std::string::const_iterator> parser;
    std::string::const_iterator it = str.begin(), end = str.end();
    bool r = phrase_parse(it, end, parser, qi::ascii::blank, program);
    if (r && it == end) {
      std::cout << "succeeded:\t" << std::endl;
    }
    else {
      std::cout << "failed:\t" << std::string(it, end) << std::endl;
    }
    return 0;
  }
}

#ifdef PARSER_MAIN
int main()
{
  std::string str;
  std::cin.unsetf(std::ios::skipws);
  std::copy(std::istream_iterator<char>(std::cin),
	    std::istream_iterator<char>(),
	    std::back_inserter(str));
  parser::Program program;
  parser::do_parse(str, program);
  parser::print_program(program);
  return 0;
}
#endif
