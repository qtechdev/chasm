#include <fstream>
#include <optional>
#include <string>
#include <vector>

#include "xdg.hpp"

std::vector<xdg::path_t> split_dirs(std::string s) {
  std::vector<xdg::path_t> dirs;

  std::size_t pos = 0;
  std::string dir;

  while ((pos = s.find(":")) != std::string::npos) {
    dir = s.substr(0, pos);
    dirs.push_back(dir);
    s.erase(0, pos + 1);
  }

  dirs.push_back(s);

  return dirs;
}

xdg::base xdg::get_base_directories() {
  base base_dirs;
  base_dirs.home = std::getenv("HOME");

  char *xdg_data_home = std::getenv("XDG_DATA_HOME");
  if (xdg_data_home == nullptr) {
    base_dirs.xdg_data_home = base_dirs.home / ".local" / "share";
  } else {
    base_dirs.xdg_data_home = xdg_data_home;
  }

  char *xdg_config_home = std::getenv("XDG_CONFIG_HOME");
  if (xdg_config_home == nullptr) {
    base_dirs.xdg_config_home = base_dirs.home / ".config";
  } else {
    base_dirs.xdg_config_home = xdg_config_home;
  }

  char *xdg_data_dirs = std::getenv("XDG_DATA_DIRS");
  if (xdg_data_dirs == nullptr) {
    base_dirs.xdg_data_dirs = {"/usr/local/share", "/usr/share"};
  } else {
    base_dirs.xdg_data_dirs = split_dirs(xdg_data_dirs);
  }

  char *xdg_config_dirs = std::getenv("XDG_CONFIG_DIRS");
  if (xdg_config_dirs == nullptr) {
    base_dirs.xdg_config_dirs = {"/etc/xdg"};
  } else {
    base_dirs.xdg_config_dirs = split_dirs(xdg_config_dirs);
  }

  char *xdg_cache_home = std::getenv("XDG_CACHE_HOME");
  if (xdg_cache_home == nullptr) {
    base_dirs.xdg_cache_home = base_dirs.home / ".cache";
  } else {
    base_dirs.xdg_cache_home = xdg_cache_home;
  }

  char *xdg_runtime_dir = std::getenv("XDG_RUNTIME_DIR");
  if (xdg_runtime_dir == nullptr) {
    // create runtime dir or throw
  } else {
    // make sure directory is valid
    base_dirs.xdg_runtime_dir = xdg_runtime_dir;
  }

  return base_dirs;
}

std::optional<xdg::path_t> xdg::get_data_path(
  const base &b, const std::string &name, const path_t &p, const bool create
) {
  path_t home_path = b.xdg_data_home / name / p;
  if (fs::is_regular_file(home_path)) {
    return home_path;
  }

  if (create) {
    if (!fs::exists(home_path)) {
      fs::create_directories(home_path.parent_path());
      std::ofstream(home_path);
    }

    return home_path;
  }

  for (const auto &dir : b.xdg_data_dirs) {
    path_t system_path = dir / name / p;
    if (fs::is_regular_file(system_path)) {
      return system_path;
    }
  }

  path_t cwd_path = path_t("./data") / p;
  if (fs::is_regular_file(cwd_path)) {
    return cwd_path;
  }

  return {};
}
