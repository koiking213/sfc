#include "fortran_parser.hpp"
#include <fstream>
#define BOOST_SPIRIT_DEBUG


bool endswith(std::string s, std::string t)
{
  if (s.size() > t.size() &&
      s.find(t, s.size() - t.size()) != std::string::npos) {
    return true;
  }
  return false;
}

void remove_blank(std::string s)
{
  for (int i=0; i<s.size(); i++) {
    s.erase(i--,1);
  }
}

int main(int argc, char* argv[])
{
  namespace qi = boost::spirit::qi;
  namespace spirit = boost::spirit;

  // read file
  std::fstream fs;
  std::string filename = argv[1];
  std::string fixed_suffix = ".f";
  fs.open(argv[1], std::fstream::in);
  bool is_fixed_file = false;
  if (endswith(filename, fixed_suffix)) {
    is_fixed_file = true;
  }
  std::string buf, input="";
  while (getline(fs, buf)) {
    input += buf + "\n";
  }
  std::cout << input << std::endl;

  // parse
  client::fortran_parser<std::string::const_iterator> fortran_parser(is_fixed_file);
  if (is_fixed_file) remove_blank(input);
  std::string::const_iterator it = input.begin(), end = input.end();
  spirit::utree ut;
  bool r = phrase_parse(it, end, fortran_parser, qi::ascii::space, ut);

  // check result
  if (r && it == end) {
    std::cout << "ok" << std::endl;
  } else {
    std::cout << "ng" << std::endl;
  }
  return 0;
}
