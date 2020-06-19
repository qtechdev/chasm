#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <vector>

#include <qxdg/qxdg.hpp>

#include "chasm.hpp"
#include "util/file_io.hpp"
#include "util/fsm.hpp"

std::vector<uint8_t> program;
static const std::regex program_re(R"re((.*)(\.chasm)$)re");

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
  auto program_files = xdg::search_data_dirs(base_dirs, "qchip", program_re);

  int index = 0;
  while ((index < 1) || (index > program_files.size())) {
    std::cout << "Choose program!\n";
    for (int i = 0; i < program_files.size(); i++) {
      std::cout << (i + 1) << ") " << program_files[i] << "\n";
    }

    std::cin >> index;
    if (std::cin.fail()) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
  }

  std::string program_path = program_files[index - 1];
  std::string binary_path = std::regex_replace(
    program_path, program_re, "$1.ch8"
  );
  std::vector<std::string> lines = read_lines(program_path);

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

  fio::write(binary_path, program, true);

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
