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
  struct token_t;

  class assembler {
  public:
    assembler();
    std::vector<uint8_t> operator()(const std::vector<std::string> &lines);

    std::vector<std::string> scan(const std::string &s, error_t &error);
    std::vector<token_t> eval(
      const std::vector<std::string> &s, error_t &error
    );
  private:
    uint16_t current_address;
    std::map<std::string, int> labels;
  };

  token_t str_to_token(const std::string &s);
  uint16_t tokens_to_binary(const std::vector<token_t> &t);
}

#ifdef DEBUG
std::ostream &operator<<(std::ostream &os, const chasm::token_t &t);
#endif

#endif // __CHASM_HPP__
