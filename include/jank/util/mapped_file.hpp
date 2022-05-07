#pragma once

#include <optional>

namespace jank::util
{
  struct mapped_file
  {
    mapped_file() = default;
    ~mapped_file();

    int fd{};
    char const *head{};
    size_t size{};
  };

  std::optional<mapped_file> map_file(char const * const file);
}
