#include <iostream>
#include <string>
#define BOOST_SPIRIT_DEBUG
#include <boost/spirit/include/support_utree.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace client {
  namespace qi = boost::spirit::qi;
  namespace spirit = boost::spirit;
  template <typename Iterator>
  struct fortran_parser : qi::grammar<Iterator, spirit::utree::list_type(), qi::locals<std::string> > {
    qi::rule<Iterator, spirit::utree::list_type(), qi::locals<std::string> > top,
      name,
      program_unit,
      main_program,
      program_stmt,
      end_program_stmt,
      specification_part,
      use_stmt,
      rename_list,
      rename,
      import_stmt,
      module,
      module_name,
      module_nature,
      only_part,
      only_list,
      one_only,
      generic_spec,
      comment;
    qi::rule<Iterator, std::string()> letter, digit, underscore, blank;
    qi::rule<Iterator, std::string()> alphanumeric_character;
    qi::rule<Iterator, std::string()> Arrow, Doublesemi;
    qi::rule<Iterator, spirit::utree::list_type()> PROGRAM, END, USE, INTRINSIC, NON_INTRINSIC, ONLY;
  public:
    fortran_parser(bool fixed=false) : fortran_parser::base_type(top)
    {
      PROGRAM = qi::as<std::string>()["program"];
      END = qi::as<std::string>()["end"];
      USE = qi::as<std::string>()["use"];
      INTRINSIC = qi::as<std::string>()["intrinsic"];
      NON_INTRINSIC = qi::as<std::string>()["non_intrinsic"];
      ONLY = qi::as<std::string>()["only"];
      Arrow = qi::as<std::string>()["=>"];
      Doublesemi = qi::as<std::string>()["::"];

      if (fixed) {
	blank = "";
      } else {
	blank = +qi::ascii::blank;
      }

      top = program_unit % qi::eol;

      // todo
      program_unit = main_program | module;

      // todo
      main_program =
	-(program_stmt >> qi::omit[+qi::eol]) >>
	-(specification_part >> qi::omit[+qi::eol]) >>
	end_program_stmt;

      program_stmt = qi::skip(qi::ascii::blank)[PROGRAM] >> qi::omit[blank] >> name;

      end_program_stmt =
	qi::skip(qi::ascii::blank)[END] >>
	-(qi::omit[blank] >> -PROGRAM >>
	  -(qi::omit[blank] >> -name));

      // todo
      specification_part = *use_stmt >> *import_stmt;

      // doing
      use_stmt = qi::skip(qi::ascii::blank)[USE] >>
	-( -(',' >> qi::skip(qi::ascii::blank)[module_nature])
	   >> qi::skip(qi::ascii::blank)[Doublesemi]) >>
	qi::omit[blank] >> name >> -(qi::skip(qi::ascii::blank)[','] >> qi::omit[*qi::ascii::blank]
				     >> (rename_list|only_part));

      module_nature = INTRINSIC | NON_INTRINSIC;

      only_part = ONLY >> qi::skip(qi::ascii::blank)[':'] >> only_list;
      only_list = (generic_spec | rename | name) % qi::char_(',');
      
      // todo
      rename_list = rename % qi::char_(',');
      rename = qi::skip(qi::ascii::blank)[name] >> qi::skip(qi::ascii::blank)[Arrow] >> qi::skip(qi::ascii::blank)[name];

      // todo
      generic_spec = "this is generic-spec" >> name;
      
      // todo
      import_stmt = "this is import statement" >> name >> name;

      // todo
      module = "this is module" >> name >> qi::omit[blank] >> name;
      
      name = letter >> *(alphanumeric_character);
      alphanumeric_character = letter | digit | underscore;
      letter = qi::ascii::alpha;
      digit = qi::digit;
      underscore = qi::char_('_');

      BOOST_SPIRIT_DEBUG_NODE(top);
    }
  };
}
