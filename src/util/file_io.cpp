#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "file_io.hpp"

std::optional<std::string> fio::read(const std::filesystem::path &p) {
  std::ifstream ifs(p);

  if (ifs) {
    ifs.seekg(0, std::ios::end);
    std::size_t filesize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::string data;
    data.resize(filesize);
    ifs.read(&data[0], filesize);

    return data;
  }

  return {};
}

std::optional<std::vector<uint8_t>> fio::readb(const std::filesystem::path &p) {
  std::ifstream ifs(p, std::ios::binary);

  if (ifs) {
    ifs.seekg(0, std::ios::end);
    std::size_t filesize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(
      (std::istreambuf_iterator<char>(ifs)),
      std::istreambuf_iterator<char>()
    );

    return data;
  }

  return {};
}

bool fio::write(
  const std::filesystem::path &p, const std::string &data, const bool trunc
) {
  std::ofstream ofs;

  if (trunc) {
    ofs.open(p, std::ios::out | std::ios::trunc);
  } else {
    ofs.open(p, std::ios::out | std::ios::app);
  }

  if (ofs) {
    ofs.write(data.c_str(), data.size());

    return true;
  }

  return false;
}

bool fio::write(
  const std::filesystem::path &p, const std::vector<uint8_t> &data,
  const bool trunc
) {
  std::ofstream ofs;

  if (trunc) {
    ofs.open(p, std::ios::out | std::ios::trunc);
  } else {
    ofs.open(p, std::ios::out | std::ios::app);
  }

  if (ofs) {
    ofs.write(reinterpret_cast<const char *>(data.data()), data.size());

    return true;
  }

  return false;
}

fio::log_stream_f::log_stream_f(const std::string &s, const bool no_buf) {
  if (no_buf) { ofs.rdbuf()->pubsetbuf(0, 0); }
  ofs.open(s, std::ios::out | std::ios::app);

  auto now = std::chrono::system_clock::now();
  std::time_t time = std::chrono::system_clock::to_time_t(now);
  ofs << std::string(79, '=') << "\n";
  ofs << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "\n";
}
