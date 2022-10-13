#pragma once

#include <cstdlib> // size_t
#include <string>

#include <jank/result.hpp>

namespace jank::util
{
  struct mapped_file
  {
    mapped_file() = default;
    mapped_file(mapped_file const&) = delete;
    mapped_file(mapped_file &&) noexcept;
    mapped_file(int const f, char const * const h, size_t const s);
    ~mapped_file();

    int fd{};
    char const *head{};
    size_t size{};
  };

  result<mapped_file, std::string> map_file(std::string_view const &path);
}
