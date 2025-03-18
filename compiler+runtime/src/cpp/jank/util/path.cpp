#include <jank/util/path.hpp>

namespace jank::util
{
  /* Returns a path relative to the current working directory, if the path
   * is under it. Otherwise, returns the input path unchanged. */
  std::filesystem::path relative_path(std::filesystem::path const &path)
  {
    auto const &relative{ std::filesystem::relative(path) };
    auto const is_subpath{ !relative.empty() && relative.native()[0] != '.' };
    if(is_subpath)
    {
      return relative;
    }
    return path;
  }

  std::filesystem::path relative_path(native_persistent_string const &path)
  {
    return relative_path(std::filesystem::path{ path.c_str() });
  }
}
