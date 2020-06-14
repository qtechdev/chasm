#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "chasm.hpp"
#include "util/file_io.hpp"
#include "util/fsm.hpp"
#include "util/xdg.hpp"

std::vector<uint8_t> program;

std::vector<std::string> read_lines(const std::string &path) {
  std::vector<std::string> lines;

  std::ifstream ifs(path);
  std::string line;
  while (std::getline(ifs, line)) {
    if (line[0] != '#') {
      lines.push_back(line);
    }
  }

  return lines;
}

int main(int argc, const char *argv[]) {
  xdg::base base_dirs = xdg::get_base_directories();
  auto program_path = xdg::get_data_path(base_dirs, "chasm", "ex.chasm");
  auto binary_path = xdg::get_data_path(base_dirs, "chasm", "out", true);
  std::cout << *binary_path << "\n";

  if (!program_path) {
    return 1;
  }

  std::vector<std::string> lines = read_lines(*program_path);

  int line_number = 0;
  int line_error = 0;
  for (const auto &line : lines) {
    chasm::error_t error = chasm::error_t::success;
    std::vector<std::string> split_line = chasm::scan(line, &error);
    line_number++;
    if (error != chasm::error_t::success) {
      line_error = line_number;
      std::cerr << "Invalid program!\nError on line " << line_error << "\n";
      std::cerr << ">> " << line << "\n";
      return -1;
    }

    std::vector<chasm::token_t> tokens = chasm::eval(split_line, &error);
    if (error != chasm::error_t::success) {
      line_error = line_number;
      std::cerr << "Invalid command!\nError on line " << line_error << "\n";
      std::cerr << ">> " << line << "\n";
      return -1;
    }

    for (const auto &s : split_line) {
      std::cout << s << " ";
    }
    std::cout << "\n";

    #ifdef DEBUG
    for (auto &t : tokens) {
      std::cout << t << " ";
    }
    std::cout << "\n";
    #endif

    uint16_t binary = tokens_to_binary(tokens);

    program.push_back(binary >> 8);
    program.push_back(binary & 0x00ff);

    #ifdef DEBUG
    char buf[5];
    snprintf(buf, 5, "%04x", binary);
    std::cout << buf << "\n\n";
    #endif
  }

  fio::write(*binary_path, program, true);

  #ifdef DEBUG
  char buf[3];
  for (const auto &b : program) {
    snprintf(buf, 3, "%02x", b);
    std::cout << buf << " ";
  }
  std::cout << "\n";
  #endif

  return 0;
}
