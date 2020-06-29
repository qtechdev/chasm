#ifdef DEBUG
#include <iostream>
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

namespace qch_asm {
  static constexpr uint16_t ARG_REGISTER = 0b0001;
  static constexpr uint16_t ARG_BYTE = 0b0010;
  static constexpr uint16_t ARG_ADDR = 0b0100;
  static constexpr uint16_t ARG_DATA = 0b1000;

  static constexpr uint16_t ARG_0 = 0x0000;
  static constexpr uint16_t ARG_R = 0x0001 | (ARG_REGISTER << 4);
  static constexpr uint16_t ARG_RR = 0x0002 | (ARG_REGISTER << 4) | (ARG_REGISTER << 8);
  static constexpr uint16_t ARG_RB = 0x0002 | (ARG_REGISTER << 4) | (ARG_BYTE << 8);
  static constexpr uint16_t ARG_RRB = 0x0003 | (ARG_REGISTER << 4) | (ARG_REGISTER << 8) | (ARG_BYTE << 12);
  static constexpr uint16_t ARG_A = 0x0001 | (ARG_ADDR << 4);
  static constexpr uint16_t ARG_D = 0x0001 | (ARG_DATA << 4);

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

  enum class token_type {
    OPCODE, REGISTER, INT_LITERAL, LABEL, DATA, UNKNOWN
  };

  enum class token_value {
    CLEAR, RET, JMP, CALL, SEQ, SNE, SEQR, MOV, ADD, MOVR, OR, AND, XOR, ADDR,
    SUB, SLR, RSUB, SLL, SNER, MOVI, JMPV, RAND, DRAW, KEQ, KNE, STD, KEY, LDD,
    LDS, ADDI, SPRITE, BCD, STR, LDR, NOP, HALT,
    REGISTER, INT_LITERAL, LABEL, DATA, UNKNOWN
  };
}

void qch_asm::build_tables() {
  std::vector<std::string> names;
  for (const auto &x : qch::isa) {
    names.push_back(x.name.data());
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

    error_t error = error_t::success;
    std::vector<std::string> split_line = scan(line, error);
    #ifdef DEBUG
    for (const std::string &s : split_line) {
      std::cout << s << " ";
    }
    std::cout << "\n";
    #endif
    line_number++;
    if (error != error_t::success) {
      char buf[512];
      snprintf(
        buf, 512, "Invalid Program!\nError on line %d.\n>>%s\n",
        line_number, line.c_str()
      );
      throw std::invalid_argument(buf);
    }

    std::vector<token_t> tokens = eval(split_line, error);
    #ifdef DEBUG
    for (auto &t : tokens) {
      std::cout << t << " ";
    }
    std::cout << "\n\n";
    #endif
    if (error != error_t::success) {
      char buf[512];
      snprintf(
        buf, 512, "Invalid Command!\nError on line %d.\n>>%s\n",
        line_number, line.c_str()
      );
      throw std::invalid_argument(buf);
    }

    tokenised.push_back(tokens);
  }

  // build label lookup
  current_address = entry_point;
  for (const std::vector<token_t> &tokens : tokenised) {
    if (tokens[0].type == token_type::LABEL) {
      labels[tokens[0].sval] = current_address;
    } else {
      current_address += 2;
    }
  }

  // assemble
  current_address = entry_point;
  for (std::vector<token_t> &tokens : tokenised) {
    if (tokens[0].type == token_type::LABEL) {
      continue;
    }

    uint16_t binary = 0;

    if (tokens[1].type == token_type::LABEL) {
      tokens[1].ival = labels.at(tokens[1].sval);
    }

    binary = tokens_to_binary(tokens);
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

std::vector<qch_asm::token_t> qch_asm::assembler::eval(
  const std::vector<std::string> &s, error_t &error
) {
  error = error_t::success;
  std::vector<token_t> tokens;

  if (s.size() == 0) {
    error = error_t::null_string;
    return {};
  }

  token_t token = str_to_token(s.at(0));

  tokens.push_back(token);

  uint16_t ival = token.ival;
  uint16_t num_args = ival & 0x0000f;
  ival >>= 4;

  if ((s.size() - 1) != num_args) {
    error = error_t::bad_arg_count;
    return {};
  }

  for (int i = 1; i <= num_args; i++) {
    uint16_t arg_type = ival & 0xf;
    token_t arg = str_to_token(s.at(i));

    static const bool is_opcode = arg.type == token_type::OPCODE;
    static const bool bad_register =
      (arg.type == token_type::REGISTER) &&
      (arg_type != ARG_REGISTER);
    static const bool bad_int =
      (arg.type == token_type::INT_LITERAL) &&
      !(
        (arg_type == ARG_BYTE) || (arg_type == ARG_ADDR) || (arg_type == ARG_DATA)
      );

    if (is_opcode || bad_register || bad_int) {
      error = error_t::bad_arg_type;
      return {};
    }

    tokens.push_back(arg);

    ival >>= 4;
  }

  return tokens;
}

qch_asm::token_t qch_asm::str_to_token(const std::string &s) {
  if (s == "clear") { return {token_type::OPCODE, token_value::CLEAR}; };
  if (s == "ret") { return {token_type::OPCODE, token_value::RET}; };
  if (s == "jmp") { return {token_type::OPCODE, token_value::JMP, ARG_A}; };
  if (s == "call") { return {token_type::OPCODE, token_value::CALL, ARG_A}; };
  if (s == "seq") { return {token_type::OPCODE, token_value::SEQ, ARG_RB}; };
  if (s == "sne") { return {token_type::OPCODE, token_value::SNE, ARG_RB}; };
  if (s == "seqr") { return {token_type::OPCODE, token_value::SEQR, ARG_RR}; };
  if (s == "mov") { return {token_type::OPCODE, token_value::MOV, ARG_RB}; };
  if (s == "add") { return {token_type::OPCODE, token_value::ADD, ARG_RB}; };
  if (s == "movr") { return {token_type::OPCODE, token_value::MOVR, ARG_RR}; };
  if (s == "or") { return {token_type::OPCODE, token_value::OR, ARG_RR}; };
  if (s == "and") { return {token_type::OPCODE, token_value::AND, ARG_RR}; };
  if (s == "xor") { return {token_type::OPCODE, token_value::XOR, ARG_RR}; };
  if (s == "addr") { return {token_type::OPCODE, token_value::ADDR, ARG_RR}; };
  if (s == "sub") { return {token_type::OPCODE, token_value::SUB, ARG_RR}; };
  if (s == "slr") { return {token_type::OPCODE, token_value::SLR, ARG_RR}; };
  if (s == "rsub") { return {token_type::OPCODE, token_value::RSUB, ARG_RR}; };
  if (s == "sll") { return {token_type::OPCODE, token_value::SLL, ARG_RR}; };
  if (s == "sner") { return {token_type::OPCODE, token_value::SNER, ARG_RR}; };
  if (s == "movi") { return {token_type::OPCODE, token_value::MOVI, ARG_A}; };
  if (s == "jmpv") { return {token_type::OPCODE, token_value::JMPV, ARG_A}; };
  if (s == "rand") { return {token_type::OPCODE, token_value::RAND, ARG_RB}; };
  if (s == "draw") { return {token_type::OPCODE, token_value::DRAW, ARG_RRB}; };
  if (s == "keq") { return {token_type::OPCODE, token_value::KEQ, ARG_R}; };
  if (s == "kne") { return {token_type::OPCODE, token_value::KNE, ARG_R}; };
  if (s == "std") { return {token_type::OPCODE, token_value::STD, ARG_R}; };
  if (s == "key") { return {token_type::OPCODE, token_value::KEY, ARG_R}; };
  if (s == "ldd") { return {token_type::OPCODE, token_value::LDD, ARG_R}; };
  if (s == "lds") { return {token_type::OPCODE, token_value::LDS, ARG_R}; };
  if (s == "addi") { return {token_type::OPCODE, token_value::ADDI, ARG_R}; };
  if (s == "sprite") { return {token_type::OPCODE, token_value::SPRITE, ARG_R}; };
  if (s == "bcd") { return {token_type::OPCODE, token_value::BCD, ARG_R}; };
  if (s == "str") { return {token_type::OPCODE, token_value::STR, ARG_R}; };
  if (s == "ldr") { return {token_type::OPCODE, token_value::LDR, ARG_R}; };
  if (s == "nop") { return {token_type::OPCODE, token_value::NOP}; };
  if (s == "halt") { return {token_type::OPCODE, token_value::HALT}; };

  fsm::error_code status = fsm::error_code::success;

  status = fsm::check(s, register_table);
  if (status == fsm::error_code::success) {
    std::string c = {s.at(1)};
    uint8_t n = std::stoi(c, nullptr, 16) & 0xff;
    return {token_type::REGISTER, token_value::REGISTER, n};
  }

  status = fsm::check(s, hex_table);
  if (status == fsm::error_code::success) {
    uint16_t n = std::stoi(s, nullptr, 16) & 0xffff;
    return {token_type::INT_LITERAL, token_value::INT_LITERAL, n};
  }

  status = fsm::check(s, label_table);
  if (status == fsm::error_code::success) {
    return {token_type::LABEL, token_value::LABEL, 0, s};
  }

  status = fsm::check(s, data_table);
  if (status == fsm::error_code::success) {
    return {token_type::DATA, token_value::DATA, ARG_D};
  }

  return {token_type::UNKNOWN, token_value::UNKNOWN};
}

static constexpr uint16_t to_a(const uint16_t x) { return (x & 0x0fff); }
static constexpr uint16_t to_x(const uint16_t x) { return (x & 0x000f) << 8; }
static constexpr uint16_t to_y(const uint16_t x) { return (x & 0x000f) << 4; }
static constexpr uint16_t to_n(const uint16_t x) { return (x & 0x000f); }
static constexpr uint16_t to_b(const uint16_t x) { return (x & 0x00ff);}

uint16_t qch_asm::tokens_to_binary(const std::vector<token_t> &t) {
  uint16_t b = 0;

  switch (t.at(0).value) {
    case token_value::CLEAR:  b = 0x00e0; break;
    case token_value::RET:    b = 0x00ee; break;
    case token_value::JMP:    b = 0x1000 | to_a(t.at(1).ival); break;
    case token_value::CALL:   b = 0x2000 | to_a(t.at(1).ival); break;
    case token_value::SEQ:    b = 0x3000 | to_x(t.at(1).ival) | to_b(t.at(2).ival); break;
    case token_value::SNE:    b = 0x4000 | to_x(t.at(1).ival) | to_b(t.at(2).ival); break;
    case token_value::SEQR:   b = 0x5000 | to_x(t.at(1).ival) | to_y(t.at(2).ival); break;
    case token_value::MOV:    b = 0x6000 | to_x(t.at(1).ival) | to_b(t.at(2).ival); break;
    case token_value::ADD:    b = 0x7000 | to_x(t.at(1).ival) | to_b(t.at(2).ival); break;
    case token_value::MOVR:   b = 0x8000 | to_x(t.at(1).ival) | to_y(t.at(2).ival); break;
    case token_value::OR:     b = 0x8001 | to_x(t.at(1).ival) | to_y(t.at(2).ival); break;
    case token_value::AND:    b = 0x8002 | to_x(t.at(1).ival) | to_y(t.at(2).ival); break;
    case token_value::XOR:    b = 0x8003 | to_x(t.at(1).ival) | to_y(t.at(2).ival); break;
    case token_value::ADDR:   b = 0x8004 | to_x(t.at(1).ival) | to_y(t.at(2).ival); break;
    case token_value::SUB:    b = 0x8005 | to_x(t.at(1).ival) | to_y(t.at(2).ival); break;
    case token_value::SLR:    b = 0x8006 | to_x(t.at(1).ival) | to_y(t.at(2).ival); break;
    case token_value::RSUB:   b = 0x8007 | to_x(t.at(1).ival) | to_y(t.at(2).ival); break;
    case token_value::SLL:    b = 0x8008 | to_x(t.at(1).ival) | to_y(t.at(2).ival); break;
    case token_value::SNER:   b = 0x9000 | to_x(t.at(1).ival) | to_y(t.at(2).ival); break;
    case token_value::MOVI:   b = 0xa000 | to_a(t.at(1).ival); break;
    case token_value::JMPV:   b = 0xb000 | to_a(t.at(1).ival); break;
    case token_value::RAND:   b = 0xc000 | to_x(t.at(1).ival) | to_b(t.at(2).ival); break;
    case token_value::DRAW:   b = 0xd000 | to_x(t.at(1).ival) | to_y(t.at(2).ival) | to_n(t.at(3).ival); break;
    case token_value::KEQ:    b = 0xe09e | to_x(t.at(1).ival); break;
    case token_value::KNE:    b = 0xe0a1 | to_x(t.at(1).ival); break;
    case token_value::STD:    b = 0xf007 | to_x(t.at(1).ival); break;
    case token_value::KEY:    b = 0xf00a | to_x(t.at(1).ival); break;
    case token_value::LDD:    b = 0xf015 | to_x(t.at(1).ival); break;
    case token_value::LDS:    b = 0xf018 | to_x(t.at(1).ival); break;
    case token_value::ADDI:   b = 0xf01e | to_x(t.at(1).ival); break;
    case token_value::SPRITE: b = 0xf029 | to_x(t.at(1).ival); break;
    case token_value::BCD:    b = 0xf033 | to_x(t.at(1).ival); break;
    case token_value::STR:    b = 0xf055 | to_x(t.at(1).ival); break;
    case token_value::LDR:    b = 0xf065 | to_x(t.at(1).ival); break;
    case token_value::HALT:   b = 0xffff; break;
    case token_value::DATA:   b = t.at(1).ival; break;
    case token_value::NOP:
    default: b = 0x0000;
  }

  return b;
}


#ifdef DEBUG
std::ostream &operator<<(std::ostream &os, const qch_asm::token_t &t) {
  switch (t.value) {
    case qch_asm::token_value::CLEAR: os << "CLEAR"; break;
    case qch_asm::token_value::RET: os << "RET"; break;
    case qch_asm::token_value::JMP: os << "JMP"; break;
    case qch_asm::token_value::CALL: os << "CALL"; break;
    case qch_asm::token_value::SEQ: os << "SEQ"; break;
    case qch_asm::token_value::SNE: os << "SNE"; break;
    case qch_asm::token_value::SEQR: os << "SEQR"; break;
    case qch_asm::token_value::MOV: os << "MOV"; break;
    case qch_asm::token_value::ADD: os << "ADD"; break;
    case qch_asm::token_value::MOVR: os << "MOVR"; break;
    case qch_asm::token_value::OR: os << "OR"; break;
    case qch_asm::token_value::AND: os << "AND"; break;
    case qch_asm::token_value::XOR: os << "XOR"; break;
    case qch_asm::token_value::ADDR: os << "ADDR"; break;
    case qch_asm::token_value::SUB: os << "SUB"; break;
    case qch_asm::token_value::SLR: os << "SLR"; break;
    case qch_asm::token_value::RSUB: os << "RSUB"; break;
    case qch_asm::token_value::SLL: os << "SLL"; break;
    case qch_asm::token_value::SNER: os << "SNER"; break;
    case qch_asm::token_value::MOVI: os << "MOVI"; break;
    case qch_asm::token_value::JMPV: os << "JMPV"; break;
    case qch_asm::token_value::RAND: os << "RAND"; break;
    case qch_asm::token_value::DRAW: os << "DRAW"; break;
    case qch_asm::token_value::KEQ: os << "KEQ"; break;
    case qch_asm::token_value::KNE: os << "KNE"; break;
    case qch_asm::token_value::STD: os << "STD"; break;
    case qch_asm::token_value::KEY: os << "KEY"; break;
    case qch_asm::token_value::LDD: os << "LDD"; break;
    case qch_asm::token_value::LDS: os << "LDS"; break;
    case qch_asm::token_value::ADDI: os << "ADDI"; break;
    case qch_asm::token_value::SPRITE: os << "SPRITE"; break;
    case qch_asm::token_value::BCD: os << "BCD"; break;
    case qch_asm::token_value::STR: os << "STR"; break;
    case qch_asm::token_value::LDR: os << "LDR"; break;
    case qch_asm::token_value::NOP: os << "NOP"; break;
    case qch_asm::token_value::HALT: os << "HALT"; break;
    case qch_asm::token_value::REGISTER: os << "REGISTER(" << t.ival << ")"; break;
    case qch_asm::token_value::INT_LITERAL: os << "INT_LITERAL(" << t.ival << ")"; break;
    case qch_asm::token_value::LABEL: os << "LABEL(" << t.sval << ")"; break;
    case qch_asm::token_value::DATA: os << "DATA" << t.sval << ""; break;
    default: os << "UNKNOWN"; break;
  }

  return os;
}
#endif
