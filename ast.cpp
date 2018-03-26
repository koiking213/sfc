#include "ast.hpp"
namespace ast {
  struct stringizer : public boost::static_visitor<std::string> {
    std::string operator()(Constant const constant) const
    {
      if (constant.type() == typeid(Integer_constant)) {
	return std::to_string(boost::get<Integer_constant>(constant).value);
      } else {
	return "invalid_constant";
      }
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
  
  std::string stringize(Expression const& expr)
   {
     return boost::apply_visitor(stringizer(), expr);
   }
}