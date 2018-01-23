#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include "parser.hpp"
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

struct Parser_fixture {
  parser::test_parser<std::string::const_iterator> parser;
};

bool test_name(parser::test_parser<std::string::const_iterator> &parser,
	       std::string input,
	       std::string expect)
{
  std::string::const_iterator it = input.begin(), end = input.end();
  std::string result;
  bool succeeded = parse(it, end, parser.name, result);
  if (expect == "") {
    BOOST_CHECK(succeeded == false);
  } else {
    BOOST_CHECK(succeeded == true);
    BOOST_CHECK(result == expect);
  }
}


BOOST_FIXTURE_TEST_SUITE(parser, Parser_fixture)

BOOST_AUTO_TEST_CASE(test1)
{
  test_name(parser, "hoge", "hoge");
}

BOOST_AUTO_TEST_CASE(test2)
{
  test_name(parser, "ho10", "ho10");
}

BOOST_AUTO_TEST_SUITE_END();
