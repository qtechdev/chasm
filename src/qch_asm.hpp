#ifndef __QCH_ASM_HPP__
#define __QCH_ASM_HPP__

#include <cstdint>
#include <string>
#include <vector>
#ifdef DEBUG
#include <ostream>
#endif

#include "util/fsm.hpp"

namespace qch_asm {
  enum class error_t;

  enum class token_type;
  enum class token_value;

  struct token_t {
    token_type type;
    token_value value;
    uint16_t ival = 0;
    std::string sval = "";
  };

  class assembler {
  public:
    assembler();
    std::vector<uint8_t> operator()(const std::vector<std::string> &lines);

    std::vector<std::string> scan(const std::string &s, error_t &error);
    std::vector<token_t> eval(
      const std::vector<std::string> &s, error_t &error
    );
  private:
    static constexpr uint16_t entry_point = 0x200;
    uint16_t current_address;
    std::map<std::string, int> labels;
    std::vector<std::vector<qch_asm::token_t>> tokenised;
  };

  token_t str_to_token(const std::string &s);
  uint16_t tokens_to_binary(const std::vector<token_t> &t);

  void build_tables();
  static fsm::fsm_table opcode_table;
  static fsm::fsm_table register_table;
  static fsm::fsm_table data_table;
}


#ifdef DEBUG
std::ostream &operator<<(std::ostream &os, const qch_asm::token_t &t);
#endif

#endif // __QCH_ASM_HPP__
