#ifdef DEBUG
#include <iostream>
#include <iomanip>
#endif

#include <array>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <qch_vm/spec.hpp>

#include "qch_asm.hpp"
#include "util/fsm.hpp"

std::size_t get_num_args(const qch::args_config &ac) {
  return static_cast<int>(ac) & 0xf;
}

bool is_label(const std::string &s) {
  std::size_t token_size = qch::label_token.size();
  if (s.size() <= token_size) {
    return false;
  }

  return s.substr(0, token_size) == qch::label_token;
}

namespace qch_asm {
  static const fsm::fsm_table hex_table = fsm::make_hex_table();

  static const fsm::fsm_table label_table = []{
    fsm::fsm_table table = {
      {0, {{':', 1}}},
      {1, {{'_', 1}, {' ', -1}}}
    };

    for (char c = 'a'; c <= 'z'; c++) {
      table.at(1)[c] = 1;
    }
    for (char c = 'A'; c <= 'Z'; c++) {
      table.at(1)[c] = 1;
    }

    return table;
  }();

  enum class error_t {
    success = 0,
    invalid_token = 1,
    null_string = 2,
    bad_arg_count = 3,
    bad_arg_type = 4
  };

  /*
  token_t to qch::instruction

  qch::instruction  = { uint16_t, uint16_t, uint16_t, args_config,  std:string }
  OPCODE            = { value,    mask,     data,     args,         name }
  REGISTER          = { value,    0x000f,   0x0001,   Z,            R_STRING }
  INT_LITERAL       = { value,    0x0fff,   0x0002,   Z,            I_STRING }
  LABEL             = { address,  0x0fff,   0x0003,   Z,            label }
  DATA              = { 0x0000,   0x0000,   0x0004,   D,            D_STRING }
  UNKNOWN           = { 0xdead,   0xbeef,   0x0000,   Z,            "UNKNOWN" }
  */
}

void qch_asm::build_tables() {
  std::vector<std::string> names;
  for (const auto &x : qch::isa) {
    names.push_back(x.name);
  }

  opcode_table = fsm::make_table(names);

  std::vector<std::string> registers;
  for (int i = 0; i <= 0xf; ++i) {
    char buf[3];
    snprintf(buf, 3, "%s%x", qch::reg_token.data(), i);
    registers.push_back(buf);
  }

  register_table = fsm::make_table(registers);

  data_table = fsm::make_table({qch::data_token.data()});
}

qch_asm::assembler::assembler() {
  current_address = entry_point;
  labels = {};
}

std::vector<uint8_t> qch_asm::assembler::operator()(
  const std::vector<std::string> &lines
) {
  std::vector<uint8_t> program;
  int line_number = 0;
  int line_error = 0;

  // validate program file
  for (const std::string &line : lines) {
    if (line == "") {
      continue;
    }
    line_number++;

    error_t error = error_t::success;
    std::vector<std::string> split_line = scan(line, error);

    #ifdef DEBUG
    std::cout << "Line " << line_number << ": ";
    for (const std::string &s : split_line) { std::cout << s << " "; }
    std::cout << "\n";
    #endif

    if (error != error_t::success) {
      char buf[512];
      snprintf(
        buf, 512, "Invalid Program!\nError on line %d.\n>>%s\n",
        line_number, line.c_str()
      );
      throw std::invalid_argument(buf);
    }

    std::vector<qch::instruction> tokens = eval(split_line, error);

    #ifdef DEBUG
    std::cout << "-> ";
    for (auto &t : tokens) { std::cout << t << " "; }
    std::cout << "\n\n";
    #endif

    if (error != error_t::success) {
      char buf[512];
      snprintf(
        buf, 512, "Invalid Command!\nError %d on line %d.\n>>%s\n",
        static_cast<int>(error), line_number, line.c_str()
      );
      throw std::invalid_argument(buf);
    }

    instructions.push_back(tokens);
  }

  // build label lookup
  current_address = entry_point;
  for (const std::vector<qch::instruction> &tokens : instructions) {
    if (is_label(tokens[0].name)) {
      labels[tokens[0].name] = current_address;
    } else {
      current_address += 2;
    }
  }

  // assemble
  current_address = entry_point;
  for (std::vector<qch::instruction> &tokens : instructions) {
    if (is_label(tokens[0].name)) {
      continue;
    }

    for (auto &t : tokens) {
      if (is_label(t.name)) {
        t.value = labels.at(t.name);
      }
    }

    uint16_t binary = tokens_to_binary(tokens);
    program.push_back(binary >> 8);
    program.push_back(binary & 0x00ff);
    current_address += 2;
  }

  return program;
}

std::vector<std::string> qch_asm::assembler::scan(
  const std::string &s, error_t &error
) {
  error = error_t::success;
  std::vector<std::string> words;
  std::string word;
  std::istringstream iss(s);

  fsm::error_code status;
  while (iss >> word) {
    // check for opcode
    status = fsm::check(word, opcode_table);
    if (status == fsm::error_code::success) { words.push_back(word); continue; }

    // check for register
    status = fsm::check(word, register_table);
    if (status == fsm::error_code::success) { words.push_back(word); continue; }

    // check for hex numbers
    status = fsm::check(word, hex_table);
    if (status == fsm::error_code::success) { words.push_back(word); continue; }

    // check for labels
    status = fsm::check(word, label_table);
    if (status == fsm::error_code::success) { words.push_back(word); continue; }

    // check for data
    status = fsm::check(word, data_table);
    if (status == fsm::error_code::success) { words.push_back(word); continue; }

    words.push_back(">" + word + "<");
    error = error_t::invalid_token;
  }

  return words;
}
std::vector<qch::instruction> qch_asm::assembler::eval(
  const std::vector<std::string> &s, error_t &error
) {
  std::vector<qch::instruction> tokens;
  error = error_t::success;

  if (s.size() == 0) {
    error = error_t::null_string;
    return {};
  }

  // parse opcode
  qch::instruction inst = str_to_token(s.at(0));
  tokens.push_back(inst);

  // pop num_args from args_config
  uint16_t arg_cfg = static_cast<int>(inst.args);
  std::size_t num_args = arg_cfg & 0xf;
  arg_cfg >>= 4;

  if ((s.size() - 1) != num_args) {
    error = error_t::bad_arg_count;
    return {};
  }

  for (int i = 1; i <= num_args; ++i) {
    // pop first argument
    uint8_t arg_type = arg_cfg & 0xf;
    arg_cfg >>= 4;
    qch::instruction arg = str_to_token(s.at(i));

    bool arg_ok = true;
    if (arg_type == qch::arg_type::R && arg.name != R_STRING) {
      arg_ok = false;
    }
    if (arg_type == qch::arg_type::D && arg.name != I_STRING) {
      arg_ok = false;
    }
    bool is_int = (arg_type == qch::arg_type::B) ||
                  (arg_type == qch::arg_type::A) ||
                  (arg_type == qch::arg_type::N);

    if (is_int && (arg.name != I_STRING)) {
      arg_ok = false;
    }

    if (is_label(arg.name) && arg_type == qch::arg_type::A) {
      arg_ok = true;
    }

    if (!arg_ok) {
      error = error_t::bad_arg_type;
      return {};
    }

    tokens.push_back(arg);
  }

  return tokens;
}

qch::instruction qch_asm::str_to_token(const std::string &s) {
  for (const auto &inst : qch::isa) {
    if (s == inst.name) {
      return inst;
    }
  }

  fsm::error_code status = fsm::error_code::success;

  status = fsm::check(s, register_table);
  if (status == fsm::error_code::success) {
    std::string c = {s.at(1)};
    uint8_t n = std::stoi(c, nullptr, 16) & 0xff;
    return {n, 0x000f, 0x0001, qch::args_config::Z, R_STRING};
  }

  status = fsm::check(s, hex_table);
  if (status == fsm::error_code::success) {
    uint16_t n = std::stoi(s, nullptr, 16) & 0xffff;
    return {n, 0x0fff, 0x0002, qch::args_config::Z, I_STRING};
  }

  status = fsm::check(s, label_table);
  if (status == fsm::error_code::success) {
    return {0, 0x0fff, 0x0003, qch::args_config::Z, s};
  }

  status = fsm::check(s, data_table);
  if (status == fsm::error_code::success) {
    return {0, 0, 0x0004, qch::args_config::D, D_STRING};
  }

  return qch::unknown_instruction;
}

static constexpr uint16_t to_a(const uint16_t x) { return (x & 0x0fff); }
static constexpr uint16_t to_x(const uint16_t x) { return (x & 0x000f) << 8; }
static constexpr uint16_t to_y(const uint16_t x) { return (x & 0x000f) << 4; }
static constexpr uint16_t to_n(const uint16_t x) { return (x & 0x000f); }
static constexpr uint16_t to_b(const uint16_t x) { return (x & 0x00ff);}

uint16_t qch_asm::tokens_to_binary(const std::vector<qch::instruction> &t) {
  uint16_t b = 0;

  switch (t.at(0).args) {
    case qch::args_config::Z: b = t.at(0).value; break;
    case qch::args_config::R: b = t.at(0).value | to_x(t.at(1).value); break;
    case qch::args_config::RR: b = t.at(0).value | to_x(t.at(1).value) | to_y(t.at(2).value); break;
    case qch::args_config::RB: b = t.at(0).value | to_x(t.at(1).value) | to_b(t.at(2).value); break;
    case qch::args_config::RRN: b = t.at(0).value | to_x(t.at(1).value) | to_y(t.at(2).value) | to_n(t.at(3).value); break;
    case qch::args_config::A: b = t.at(0).value | to_a(t.at(1).value); break;
    case qch::args_config::D: b = t.at(1).value; break;
  }

  return b;
}

#ifdef DEBUG
std::ostream &operator<<(std::ostream &os, const qch::instruction &inst) {
  os << inst.name;
  if (inst.name == qch_asm::R_STRING) {
    os << "(" << qch::reg_token << inst.value << ")";
  } else if (inst.name == qch_asm::I_STRING) {
    os << "(" << inst.value << ")";
  }

  return os;
}
#endif
