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

  std::filesystem::path relative_path(jtl::immutable_string const &path)
  {
    return relative_path(std::filesystem::path{ path.c_str() });
  }

  /* Truncates long paths so that `/foo/bar/spam/meow.jank` will become `…/spam/meow.jank`,
   * with specific support for directories so we don't end up with `r/spam/meow.jank`. */
  std::string compact_path(std::filesystem::path const &path, usize const max_size)
  {
    if(path.native().size() <= max_size)
    {
      return path.native();
    }

    auto str{ path.native() };
    while(str.size() + 1 > max_size)
    {
      auto const slash{ str.find('/') };
      if(slash == decltype(str)::npos)
      {
        auto const diff{ str.size() - max_size + 1 };
        return "…" + str.erase(0, diff);
      }

      str.erase(0, slash + 1);
    }
    return "…" + str;
  }
}
