#include <iostream>
#include <string>
#include <boost/spirit/include/support_utree.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include "fortran_parser.hpp"

namespace qi = boost::spirit::qi;
namespace spirit = boost::spirit;

int main()
{
  client::fortran_parser<std::string::const_iterator> fortran_parser;
  std::string str;
  std::cin.unsetf(std::ios::skipws);
  std::copy(std::istream_iterator<char>(std::cin), std::istream_iterator<char>(), std::back_inserter(str));
  {
    // remove_space(str);
    std::string::const_iterator it = str.begin(), end = str.end();
    spirit::utree ut;
    bool r = phrase_parse(it, end, fortran_parser, qi::ascii::space/*, qi::skip_flag::dont_postskip*/, ut);
    std::cout << str << std::endl;
    if (r && it == end) {
      std::cout << "succeeded:\t" << ut << std::endl;
    }
    else {
      std::cout << "failed:\t" << std::string(it, end) << std::endl;
    }
  }
  return 0;
}
