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
    std::vector<std::vector<chasm::token_t>> tokenised;
  };

  token_t str_to_token(const std::string &s);
  uint16_t tokens_to_binary(const std::vector<token_t> &t);
}

#ifdef DEBUG
std::ostream &operator<<(std::ostream &os, const chasm::token_t &t);
#endif

#endif // __CHASM_HPP__
