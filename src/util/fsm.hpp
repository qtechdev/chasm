#ifndef __FSM_HPP__
#define __FSM_HPP__

#include <map>
#include <string>
#include <vector>

namespace fsm {
  using fsm_table = std::map<int, std::map<char, int>>;
  enum class error_code {
    success = 0,
    invalid_state = 1,
    invalid_string = 2,
    too_short = 3
  };

  struct machine {
    std::string s;
    const fsm_table table;
    int index=0;
    int state=0;
    bool end=false;
  };

  error_code next_state(machine &f);
  error_code check(const std::string &s, const fsm_table &t);

  // some basic table templates
  fsm_table make_int_table();
  fsm_table make_hex_table();
  fsm_table make_char_table();

  // make_table can handle substrings, e.g. "foo" and "foobar" but they must
  // be provided in order of shortest substring to longest.
  // todo: character classes and recursion
  fsm_table make_table(const std::vector<std::string> &strings);

  int get_max_index(const fsm_table &table);

  // adds transitions for any length decimal integer
  void add_digit_transition(fsm_table &table);

}

#endif // __FSM_HPP__
