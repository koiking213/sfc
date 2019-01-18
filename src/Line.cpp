#include "Line.hpp"

static bool is_blank(char c)
{
  return (c == ' ' || c == '\t');
}

void Line::skip_blanks()
{
  while (is_blank(content[column])) {
    column++;
  }
}
std::string Line::read_name()
{
  skip_blanks();
  if (!isalpha(content[column])) {
    return "";
  }
  int begin = column;
  while (isalpha(content[column]) ||
	 isdigit(content[column]) ||
	 content[column] == '_') {
    column++;
  }
  return content.substr(begin, column-begin);
}
bool Line::read_one_blank()
{
  if (is_blank(content[column])) {
    column++;
    return true;
  }
  return false;
}
bool Line::is_end_of_line()
{
  int save_ofs = column;
  skip_blanks();
  if (content.size() == column) {
    return true;
  } else {
    column = save_ofs;
    return false;
  }
}
bool Line::read_token(const std::string str)
{
  int save_ofs = column;
  skip_blanks();
  if (std::equal(str.begin(), str.end(), content.begin()+column)) {
    column += str.size();
    return true;
  } else {
    column = save_ofs;
    return false;
  }
}
std::string Line::read_int_constant()
{
  int save_ofs = column;
  if (!isdigit(content[column])) {
    return "";
  }
  int begin = column;
  while (isdigit(content[column])) {
    column++;
  }
  return content.substr(begin, column-begin);
}
std::string Line::read_real_constant()
{
  int save_ofs = column;
  int begin = column;
  while (isdigit(content[column])) {
    column++;
  }
  if (content[column++] == '.' && isdigit(content[column])) {
    while (isdigit(content[column])) {
      column++;
    }
    return content.substr(begin, column-begin);
  } else {
    column = save_ofs;
    return "";
  }
}
std::string Line::read_logical_constant()
{
  if (this->read_token(".true.")) {
    return ".true.";
  }
  if (this->read_token(".false.")) {
    return ".false.";
  }
  return "";
}
std::string Line::read_character_constant()
{
  int save_ofs = column;
  if (this->read_token("\"")) {
    int pos = content.find("\"", column+1);
    if (pos != std::string::npos) {
      column = pos+1;
      return content.substr(save_ofs+1, pos-save_ofs-1);
    }
  }
  column = save_ofs;
  return "";
}
bool Line::read_operator(const std::string op)
{
  int save_ofs = column;
  skip_blanks();
  if (std::equal(op.begin(), op.end(), content.begin()+column)) {
    char c = content[column+op.size()];
    if (isalpha(c) || isdigit(c) || c=='(') {
      column += op.size();
      return true;
    }
  }
  column = save_ofs;
  return false;
}

