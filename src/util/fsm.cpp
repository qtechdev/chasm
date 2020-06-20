#include <map>
#include <string>
#include <vector>

#include "fsm.hpp"

fsm::error_t fsm::next_state(machine &f) {
  if (f.index == f.s.size()) {
    auto state_index = f.table.find(f.state);
    if (state_index == f.table.end()) {
      return error_t::invalid_state;
    }

    auto char_index = f.table.at(f.state).find(' ');
    if (char_index == f.table.at(f.state).end()) {
      return error_t::too_short;
    }

    f.end = true;
    return error_t::success;
  }

  char c = f.s[f.index];
  auto char_index = f.table.at(f.state).find(c);
  if (char_index == f.table.at(f.state).end()) {
    return error_t::invalid_string;
  }

  f.state = f.table.at(f.state).at(c);
  f.index++;
  if (f.state == -1) {
    f.end = true;
  }

  return error_t::success;
};

fsm::error_t fsm::check(const std::string &s, const table_t &t) {
  error_t status = error_t::success;

  machine m{s, t};
  while (!m.end) {
    status = next_state(m);
    if (status != error_t::success) {
      m.end = true;
    }
  }

  return status;
}


fsm::table_t fsm::make_int_table() {
  table_t table = {
    {0, {}},
    {1, {}}
  };

  for (char c = '1'; c <= '9'; c++) {
    table.at(0)[c] = 1;
    table.at(1)[c] = 1;
  }
  table.at(1)['0'] = 1;
  table.at(1)[' '] = -1;

  return table;
}

fsm::table_t fsm::make_hex_table() {
  table_t table = {
    {0, {}},
    {1, {}},
    {2, {}}
  };
  table.at(0)['0'] = 1;
  table.at(1)['x'] = 2;

  for (char c = '0'; c <= '9'; c++) {
    table.at(2)[c] = 2;
  }
  for (char c = 'a'; c <= 'f'; c++) {
    table.at(2)[c] = 2;
  }
  for (char c = 'A'; c <= 'F'; c++) {
    table.at(2)[c] = 2;
  }

  table.at(2)[' '] = -1;

  return table;
}

fsm::table_t fsm::make_char_table() {
  table_t table = {
    {0, {}},
    {1, {}}
  };

  for (char c = 'a'; c <= 'z'; c++) {
    table.at(0)[c] = 1;
    table.at(1)[c] = 1;
  }
  for (char c = 'A'; c <= 'Z'; c++) {
    table.at(0)[c] = 1;
    table.at(1)[c] = 1;
  }
  table.at(1)[' '] = -1;

  return table;
}

fsm::table_t fsm::make_table(const std::vector<std::string> &strings) {
  table_t table;
  int next_unused_state = 1;

  for (const auto &s : strings) {
    int current_state = 0;
    for (int i = 0; i < s.size(); i++) {
      const char c = s[i];
      // get current state from table
      auto cs = table.find(current_state);
      // create if not found
      if (cs == table.end()) {
        table[current_state] = {};
      }

      // check current state for current character
      auto cc = table[current_state].find(c);
      // insert if not found
      if (cc == table[current_state].end()) {
        // if char is not end of string
        table[current_state][c] = next_unused_state;
        current_state = next_unused_state;
        next_unused_state++;
        if (i == s.size() - 1) {
          // state -1 signifies end state
          table[current_state][' '] = -1;
        }
      } else {
        current_state = table[current_state][c];
      }
    }
  }

  return table;
}

int fsm::get_max_index(const table_t &table) {
  int max_index = 0;
  for (const auto &[k, v] : table) {
    if (k > max_index) {
      max_index = k;
    }
  }

  return max_index;
}

void fsm::add_digit_transition(table_t &table) {
  if (table.find(0) == table.end()) {
    table[0] = {};
  }

  int index = get_max_index(table) + 1;

  table[index] = {};
  for (char c = '1'; c <= '9'; c++) {
    table.at(0)[c] = index;
    table.at(index)[c] = index;
  }
  table.at(index)['0'] = index;
  table.at(index)[' '] = -1;
}
