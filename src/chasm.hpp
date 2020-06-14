#ifndef __CHASM_HPP__
#define __CHASM_HPP__

#include <cstdint>
#include <string>
#include <vector>
#ifdef DEBUG
#include <ostream>
#endif

#include "util/fsm.hpp"

namespace chasm {
  static constexpr uint16_t ARG_REG = 0b0001;
  static constexpr uint16_t ARG_VAL = 0b0010;
  static constexpr uint16_t ARG_ADR = 0b0100;

  static constexpr uint16_t ARG_0    = 0x0000;
  static constexpr uint16_t ARG_R    = 0x0001 | (ARG_REG << 4);
  static constexpr uint16_t ARG_RR   = 0x0002 | (ARG_REG << 4) | (ARG_REG << 8);
  static constexpr uint16_t ARG_RV   = 0x0002 | (ARG_REG << 4) | (ARG_VAL << 8);
  static constexpr uint16_t ARG_RRV  = 0x0003 | (ARG_REG << 4) | (ARG_REG << 8) | (ARG_VAL << 12);
  static constexpr uint16_t ARG_ADDR = 0x0001 | (ARG_ADR << 4);

  static const fsm::table_t opcode_table = fsm::make_table({
    "clear", "ret", "jmp", "call", "seq", "sne", "seqr", "mov", "add", "movr",
    "or", "and", "xor", "addr", "sub", "slr", "rsub", "sll", "sner", "movi",
    "jmpv", "rand", "draw", "keq", "kne", "std", "key", "ldd", "lds", "addi",
    "sprite", "bcd", "str", "ldr"
  });

  static const fsm::table_t register_table = fsm::make_table({
    "V0", "V1", "V2", "V3", "V4", "V5", "V6", "V7",
    "V8", "V9", "VA", "VB", "VC", "VD", "VE", "VF"
  });

  static const fsm::table_t hex_table = fsm::make_hex_table();

  enum class error_t {
    success = 0,
    invalid_token = 1,
    null_string = 2,
    bad_arg_count = 3,
    bad_arg_type = 4
  };

  enum class token_type {
    OPCODE, REGISTER, INT_LITERAL, UNKNOWN
  };

  enum class token_value {
    CLEAR, RET, JMP, CALL, SEQ, SNE, SEQR, MOV, ADD, MOVR, OR, AND, XOR, ADDR,
    SUB, SLR, RSUB, SLL, SNER, MOVI, JMPV, RAND, DRAW, KEQ, KNE, STD, KEY, LDD,
    LDS, ADDI, SPRITE, BCD, STR, LDR, NOP, HALT,
    V0, V1, V2, V3, V4, V5, V6, V7, V8, V9, VA, VB, VC, VD, VE, VF,
    INT_LITERAL, UNKNOWN
  };

  struct token_t {
    token_type type;
    token_value value;
    uint16_t ival = 0;
  };

  std::vector<std::string> scan(const std::string &s, error_t *error);
  std::vector<token_t> eval(const std::vector<std::string> &s, error_t *error);

  token_t str_to_token(const std::string &s);
  uint16_t tokens_to_binary(const std::vector<token_t> &t);
}

#ifdef DEBUG
std::ostream &operator<<(std::ostream &os, const chasm::token_t &t);
#endif

#endif // __CHASM_HPP__
