#ifndef __XDG_HPP__
#define __XDG_HPP__
// https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace xdg {
  namespace fs = std::filesystem;
  using path_t = std::filesystem::path;

  struct base {
    path_t home;
    path_t xdg_data_home; // $HOME/.local/share
    path_t xdg_config_home; // $HOME/.config
    std::vector<path_t> xdg_data_dirs; // /usr/local/share/:/usr/share/
    std::vector<path_t> xdg_config_dirs; // /etc/xdg
    path_t xdg_cache_home; // $HOME/.cache
    path_t xdg_runtime_dir;
  };

  base get_base_directories();

  std::optional<path_t> get_data_path(
    const base &b, const std::string &name, const path_t &p,
    const bool create=false
  );
};

#endif // __XDG_HPP__
