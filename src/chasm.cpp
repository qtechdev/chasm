#include <iostream>

#include <array>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "chasm.hpp"
#include "util/fsm.hpp"


std::vector<std::string> chasm::scan(const std::string &s, error_t *error) {
  *error = error_t::success;
  std::vector<std::string> words;
  std::string word;
  std::istringstream iss(s);

  fsm::error_t status;
  while (iss >> word) {
    // check for opcode
    status = fsm::error_t::success;
    fsm::machine opcode_machine{word, opcode_table};
    while (!opcode_machine.end) {
      status = fsm::next_state(opcode_machine);
      if (status != fsm::error_t::success) {
        opcode_machine.end = true;
      }
    }

    if (status == fsm::error_t::success) {
      words.push_back(word);
      continue;
    }

    // check for register
    status = fsm::error_t::success;
    fsm::machine register_machine{word, register_table};
    while (!register_machine.end) {
      status = fsm::next_state(register_machine);
      if (status != fsm::error_t::success) {
        register_machine.end = true;
      }
    }

    if (status == fsm::error_t::success) {
      words.push_back(word);
      continue;
    }

    // check for hex numbers
    status = fsm::error_t::success;
    fsm::machine hex_machine{word, hex_table};
    while (!hex_machine.end) {
      status = fsm::next_state(hex_machine);
      if (status != fsm::error_t::success) {
        hex_machine.end = true;
      }
    }

    if (status == fsm::error_t::success) {
      words.push_back(word);
      continue;
    }

    words.push_back(">" + word + "<");
    *error = error_t::invalid_token;
  }

  return words;
}

std::vector<chasm::token_t> chasm::eval(
  const std::vector<std::string> &s, error_t *error
) {
  *error = error_t::success;
  std::vector<token_t> tokens;

  if (s.size() == 0) {
    *error = error_t::null_string;
    return {};
  }

  token_t token = str_to_token(s.at(0));
  tokens.push_back(token);

  uint16_t ival = token.ival;
  uint16_t num_args = ival & 0x0000f;
  ival >>= 4;

  if ((s.size() - 1) != num_args) {
    *error = error_t::bad_arg_count;
    return {};
  }

  for (int i = 1; i <= num_args; i++) {
    uint16_t arg_type = ival & 0xf;
    token_t arg = str_to_token(s.at(i));

    if (
      (arg.type == token_type::OPCODE) ||
      ((arg.type == token_type::REGISTER) && (arg_type != ARG_REG)) ||
      ((arg.type == token_type::INT_LITERAL) && ((arg_type != ARG_VAL) && (arg_type != ARG_ADR)))
    ) {
      *error = error_t::bad_arg_type;
      return {};
    }

    tokens.push_back(arg);

    ival >>= 4;
  }

  return tokens;
}

chasm::token_t chasm::str_to_token(const std::string &s) {
  if (s == "clear") { return {token_type::OPCODE, token_value::CLEAR}; };
  if (s == "ret") { return {token_type::OPCODE, token_value::RET}; };
  if (s == "jmp") { return {token_type::OPCODE, token_value::JMP, ARG_ADDR}; };
  if (s == "call") { return {token_type::OPCODE, token_value::CALL, ARG_ADDR}; };
  if (s == "seq") { return {token_type::OPCODE, token_value::SEQ, ARG_RV}; };
  if (s == "sne") { return {token_type::OPCODE, token_value::SNE, ARG_RV}; };
  if (s == "seqr") { return {token_type::OPCODE, token_value::SEQR, ARG_RR}; };
  if (s == "mov") { return {token_type::OPCODE, token_value::MOV, ARG_RV}; };
  if (s == "add") { return {token_type::OPCODE, token_value::ADD, ARG_RV}; };
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
  if (s == "movi") { return {token_type::OPCODE, token_value::MOVI, ARG_ADDR}; };
  if (s == "jmpv") { return {token_type::OPCODE, token_value::JMPV, ARG_ADDR}; };
  if (s == "rand") { return {token_type::OPCODE, token_value::RAND, ARG_RV}; };
  if (s == "draw") { return {token_type::OPCODE, token_value::DRAW, ARG_RRV}; };
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
  if (s == "V0") { return {token_type::REGISTER, token_value::V0, 0x0}; };
  if (s == "V1") { return {token_type::REGISTER, token_value::V1, 0x1}; };
  if (s == "V2") { return {token_type::REGISTER, token_value::V2, 0x2}; };
  if (s == "V3") { return {token_type::REGISTER, token_value::V3, 0x3}; };
  if (s == "V4") { return {token_type::REGISTER, token_value::V4, 0x4}; };
  if (s == "V5") { return {token_type::REGISTER, token_value::V5, 0x5}; };
  if (s == "V6") { return {token_type::REGISTER, token_value::V6, 0x6}; };
  if (s == "V7") { return {token_type::REGISTER, token_value::V7, 0x7}; };
  if (s == "V8") { return {token_type::REGISTER, token_value::V8, 0x8}; };
  if (s == "V9") { return {token_type::REGISTER, token_value::V9, 0x9}; };
  if (s == "VA") { return {token_type::REGISTER, token_value::VA, 0xA}; };
  if (s == "VB") { return {token_type::REGISTER, token_value::VB, 0xB}; };
  if (s == "VC") { return {token_type::REGISTER, token_value::VC, 0xC}; };
  if (s == "VD") { return {token_type::REGISTER, token_value::VD, 0xD}; };
  if (s == "VE") { return {token_type::REGISTER, token_value::VE, 0xE}; };
  if (s == "VF") { return {token_type::REGISTER, token_value::VF, 0xF}; };

  fsm::error_t status = fsm::error_t::success;
  fsm::machine hex_machine{s, hex_table};
  while (!hex_machine.end) {
    status = fsm::next_state(hex_machine);
    if (status != fsm::error_t::success) {
      hex_machine.end = true;
    }
  }

  if (status == fsm::error_t::success) {
    uint16_t n = std::stoi(s, nullptr, 16) & 0xffff;
    return {token_type::INT_LITERAL, token_value::INT_LITERAL, n};
  }


  return {token_type::UNKNOWN, token_value::UNKNOWN};
}

uint16_t to_a(const uint16_t x) {
  return (x & 0x0fff);
}

uint16_t to_x(const uint16_t x) {
  return (x & 0x000f) << 8;
}

uint16_t to_y(const uint16_t x) {
  return (x & 0x000f) << 4;
}
uint16_t to_n(const uint16_t x) {
  return (x & 0x000f);
}

uint16_t to_v(const uint16_t x) {
  return (x & 0x00ff);
}


uint16_t chasm::tokens_to_binary(const std::vector<token_t> &t) {
  uint16_t b = 0;

  switch (t.at(0).value) {
    case token_value::CLEAR:  b = 0x00e0; break;
    case token_value::RET:    b = 0x00ee; break;
    case token_value::JMP:    b = 0x1000 | to_a(t.at(1).ival); break;
    case token_value::CALL:   b = 0x2000 | to_a(t.at(1).ival); break;
    case token_value::SEQ:    b = 0x3000 | to_x(t.at(1).ival) | to_v(t.at(2).ival); break;
    case token_value::SNE:    b = 0x4000 | to_x(t.at(1).ival) | to_v(t.at(2).ival); break;
    case token_value::SEQR:   b = 0x5000 | to_x(t.at(1).ival) | to_y(t.at(2).ival); break;
    case token_value::MOV:    b = 0x6000 | to_x(t.at(1).ival) | to_v(t.at(2).ival); break;
    case token_value::ADD:    b = 0x7000 | to_x(t.at(1).ival) | to_v(t.at(2).ival); break;
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
    case token_value::RAND:   b = 0xc000 | to_x(t.at(1).ival) | to_v(t.at(2).ival); break;
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
    case token_value::NOP:
    default: b = 0x0000;
  }

  return b;
}


#ifdef DEBUG
std::ostream &operator<<(std::ostream &os, const chasm::token_t &t) {
  switch (t.value) {
    case chasm::token_value::CLEAR: os << "CLEAR"; break;
    case chasm::token_value::RET: os << "RET"; break;
    case chasm::token_value::JMP: os << "JMP"; break;
    case chasm::token_value::CALL: os << "CALL"; break;
    case chasm::token_value::SEQ: os << "SEQ"; break;
    case chasm::token_value::SNE: os << "SNE"; break;
    case chasm::token_value::SEQR: os << "SEQR"; break;
    case chasm::token_value::MOV: os << "MOV"; break;
    case chasm::token_value::ADD: os << "ADD"; break;
    case chasm::token_value::MOVR: os << "MOVR"; break;
    case chasm::token_value::OR: os << "OR"; break;
    case chasm::token_value::AND: os << "AND"; break;
    case chasm::token_value::XOR: os << "XOR"; break;
    case chasm::token_value::ADDR: os << "ADDR"; break;
    case chasm::token_value::SUB: os << "SUB"; break;
    case chasm::token_value::SLR: os << "SLR"; break;
    case chasm::token_value::RSUB: os << "RSUB"; break;
    case chasm::token_value::SLL: os << "SLL"; break;
    case chasm::token_value::SNER: os << "SNER"; break;
    case chasm::token_value::MOVI: os << "MOVI"; break;
    case chasm::token_value::JMPV: os << "JMPV"; break;
    case chasm::token_value::RAND: os << "RAND"; break;
    case chasm::token_value::DRAW: os << "DRAW"; break;
    case chasm::token_value::KEQ: os << "KEQ"; break;
    case chasm::token_value::KNE: os << "KNE"; break;
    case chasm::token_value::STD: os << "STD"; break;
    case chasm::token_value::KEY: os << "KEY"; break;
    case chasm::token_value::LDD: os << "LDD"; break;
    case chasm::token_value::LDS: os << "LDS"; break;
    case chasm::token_value::ADDI: os << "ADDI"; break;
    case chasm::token_value::SPRITE: os << "SPRITE"; break;
    case chasm::token_value::BCD: os << "BCD"; break;
    case chasm::token_value::STR: os << "STR"; break;
    case chasm::token_value::LDR: os << "LDR"; break;
    case chasm::token_value::NOP: os << "NOP"; break;
    case chasm::token_value::HALT: os << "HALT"; break;
    case chasm::token_value::V0: os << "V0"; break;
    case chasm::token_value::V1: os << "V1"; break;
    case chasm::token_value::V2: os << "V2"; break;
    case chasm::token_value::V3: os << "V3"; break;
    case chasm::token_value::V4: os << "V4"; break;
    case chasm::token_value::V5: os << "V5"; break;
    case chasm::token_value::V6: os << "V6"; break;
    case chasm::token_value::V7: os << "V7"; break;
    case chasm::token_value::V8: os << "V8"; break;
    case chasm::token_value::V9: os << "V9"; break;
    case chasm::token_value::VA: os << "VA"; break;
    case chasm::token_value::VB: os << "VB"; break;
    case chasm::token_value::VC: os << "VC"; break;
    case chasm::token_value::VD: os << "VD"; break;
    case chasm::token_value::VE: os << "VE"; break;
    case chasm::token_value::VF: os << "VF"; break;
    case chasm::token_value::INT_LITERAL: os << "INT_LITERAL(" << t.ival << ")"; break;
    default: os << "UNKNOWN"; break;
  }

  return os;
}
#endif
