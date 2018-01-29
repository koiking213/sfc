#include "ast.hpp"
namespace ast {
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
