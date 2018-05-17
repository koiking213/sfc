#pragma once
#include "cst.hpp"

namespace parser {
  extern std::unique_ptr<cst::Program> parse(std::string str);
}
