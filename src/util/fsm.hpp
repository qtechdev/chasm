#ifndef __FSM_HPP__
#define __FSM_HPP__

#include <map>
#include <string>
#include <vector>

namespace fsm {
  using table_t = std::map<int, std::map<char, int>>;
  enum class error_t {
    success = 0,
    invalid_state = 1,
    invalid_string = 2,
    too_short = 3
  };

  struct machine {
    std::string s;
    const table_t table;
    int index=0;
    int state=0;
    bool end=false;
  };

  error_t next_state(machine &f);
  error_t check(const std::string &s, const table_t &t);

  // some basic table templates
  table_t make_int_table();
  table_t make_hex_table();
  table_t make_char_table();

  // make_table can handle substrings, e.g. "foo" and "foobar" but they must
  // be provided in order of shortest substring to longest.
  // todo: character classes and recursion
  table_t make_table(const std::vector<std::string> &strings);

  int get_max_index(const table_t &table);

  // adds transitions for any length decimal integer
  void add_digit_transition(table_t &table);

}

#endif // __FSM_HPP__
