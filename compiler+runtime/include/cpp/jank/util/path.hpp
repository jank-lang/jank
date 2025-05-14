#pragma once

#include <filesystem>

#include <jtl/immutable_string.hpp>

namespace jank::util
{
  std::filesystem::path relative_path(std::filesystem::path const &path);
  std::filesystem::path relative_path(jtl::immutable_string const &path);

  std::string compact_path(std::filesystem::path const &path, usize max_size);
}
