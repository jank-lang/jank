#include <clang/Basic/Version.h>
#include <llvm/TargetParser/Host.h>

#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/util/dir.hpp>
#include <jank/util/sha256.hpp>
#include <jank/util/string_builder.hpp>

namespace jank::util
{
  native_persistent_string const &user_home_dir()
  {
    static native_persistent_string res;
    if(!res.empty())
    {
      return res;
    }

    auto const home(getenv("HOME"));
    if(home)
    {
      res = home;
    }
    return res;
  }

  native_persistent_string const &user_cache_dir()
  {
    static native_persistent_string res;
    if(!res.empty())
    {
      return res;
    }

    auto const home(getenv("XDG_CACHE_HOME"));
    if(home)
    {
      res = fmt::format("{}/jank", home);
      return res;
    }
    res = fmt::format("{}/.cache/jank", user_home_dir());
    return res;
  }

  native_persistent_string const &user_config_dir()
  {
    static native_persistent_string res;
    if(!res.empty())
    {
      return res;
    }

    auto const home(getenv("XDG_CONFIG_HOME"));
    if(home)
    {
      res = fmt::format("{}/jank", home);
      return res;
    }
    res = fmt::format("{}/.config/jank", user_home_dir());
    return res;
  }

  native_persistent_string const &
  binary_cache_dir(native_integer const optimization_level,
                   native_vector<native_persistent_string> const &includes,
                   native_vector<native_persistent_string> const &defines)
  {
    static native_persistent_string res;
    if(!res.empty())
    {
      return res;
    }

    return res
      = fmt::format("{}/{}", "target", binary_version(optimization_level, includes, defines));
  }

  /* The binary version is composed of two things:
   *
   * 1. The LLVM target triplet
   * 2. A SHA256 hash of unique inputs
   *
   * The intention of the hash is to ensure that changes made to the compiler will
   * result in needing to recompile previous binary artifacts. The way that it's
   * currently implemented, any new jank version will require a clean compile of
   * every module. I think this is much safer than trying to reconcile ABI
   * changes more granularly.
   */
  native_persistent_string const &
  binary_version(native_integer const optimization_level,
                 native_vector<native_persistent_string> const &includes,
                 native_vector<native_persistent_string> const &defines)
  {
    static native_persistent_string res;
    if(!res.empty())
    {
      return res;
    }

    string_builder sb;
    for(auto const &inc : includes)
    {
      sb(inc);
    }

    sb(".");

    for(auto const &def : defines)
    {
      sb(def);
    }

    auto const input(fmt::format("{}.{}.{}.{}.{}",
                                 JANK_VERSION,
                                 clang::getClangRevision(),
                                 JANK_JIT_FLAGS,
                                 optimization_level,
                                 sb.release()));
    res = fmt::format("{}-{}", llvm::sys::getDefaultTargetTriple(), util::sha256(input));

    //fmt::println("binary_version {}", res);

    return res;
  }
}
