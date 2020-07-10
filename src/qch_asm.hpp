#ifndef __QCH_ASM_HPP__
#define __QCH_ASM_HPP__

#include <cstdint>
#include <string>
#include <vector>
#ifdef DEBUG
#include <ostream>
#endif

#include <qch_vm/spec.hpp>

#include "util/fsm.hpp"

namespace qch_asm {
  enum class error_t;

  static const std::string R_STRING = "REGISTER";
  static const std::string I_STRING = "INT_LITERAL";
  static const std::string D_STRING = "DATA";

  class assembler {
  public:
    assembler();
    std::vector<uint8_t> operator()(const std::vector<std::string> &lines);

    std::vector<std::string> scan(const std::string &s, error_t &error);

    std::vector<qch::instruction> eval(
      const std::vector<std::string> &s, error_t &error
    );
  private:
    static constexpr uint16_t entry_point = 0x200;
    uint16_t current_address;
    std::map<std::string, int> labels;
    std::vector<std::vector<qch::instruction>> instructions;
  };

  qch::instruction str_to_token(const std::string &s);
  uint16_t tokens_to_binary(const std::vector<qch::instruction> &t);

  void build_tables();
  static fsm::fsm_table opcode_table;
  static fsm::fsm_table register_table;
  static fsm::fsm_table data_table;
  static fsm::fsm_table comment_table;
}


#ifdef DEBUG
std::ostream &operator<<(std::ostream &os, const qch::instruction &t);
#endif

#endif // __QCH_ASM_HPP__
