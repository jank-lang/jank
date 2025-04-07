#pragma once

#include <filesystem>

#include <jank/native_persistent_string.hpp>

namespace jank::util
{
  std::filesystem::path relative_path(std::filesystem::path const &path);
  std::filesystem::path relative_path(native_persistent_string const &path);

  std::string compact_path(std::filesystem::path const &path, size_t max_size);
}
