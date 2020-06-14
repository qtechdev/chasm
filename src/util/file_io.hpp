#ifndef __FILE_IO_HPP__
#define __FILE_IO_HPP__

#include <cstdint>
#include <fstream>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace fio {
  std::optional<std::string> read(const std::filesystem::path &p);
  std::optional<std::vector<uint8_t>> readb(const std::filesystem::path &p);
  bool write(
    const std::filesystem::path &p, const std::string &data,
    const bool trunc=false
  );
  bool write(
    const std::filesystem::path &p, const std::vector<uint8_t> &data,
    const bool trunc=false
  );

  class log_stream_f {
  public:
    log_stream_f(const std::string &s, const bool no_buf=false);
    template <typename T>
    std::ofstream &operator<<(const T t);
  private:
    std::ofstream ofs;
  };
};

template<typename T>
std::ofstream &fio::log_stream_f::operator<<(const T t) {
  ofs << t;

  return ofs;
}

#endif // __FILE_IO_HPP__
