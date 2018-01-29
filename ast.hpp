#pragma once
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

  char const* to_string(operators op);

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

  std::string stringize(Expression const& expr);
}
