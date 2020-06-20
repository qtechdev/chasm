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
#include <qfio/qfio.hpp>

#include "chasm.hpp"
#include "util/fsm.hpp"

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

  chasm::assembler ch;
  std::vector<uint8_t> program;
  try {
    program = ch(lines);
  } catch (const std::invalid_argument &e) {
    std::cerr << e.what() << "\n";
    return -1;
  };

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
