#pragma once

#include <string>
#include <algorithm>

class Line {
public:
  Line(int num, std::string str) {
    this->line_num = num;
    this->content_orig = str;
    this->content = str;
    std::transform(this->content.begin(), this->content.end(), this->content.begin(), tolower);
    this->column = 0;
  }
  bool is_eof();
  void skip_blanks();
  std::string read_name();
  bool read_one_blank();
  bool is_end_of_line();
  bool read_token(const std::string str);
  bool read_operator(const std::string op);
  std::string read_int_constant();
  std::string read_real_constant();
  std::string read_logical_constant();
  std::string read_character_constant();
  int get_line_num() {return line_num;};
  int get_column() {return column;};
  void set_column(int n) {column = n;};
  std::string get_content() {return content;};
  std::string get_content_orig() {return content_orig;};
private:
  int line_num;
  int column;
  std::string content;
  std::string content_orig; // for error message
};
