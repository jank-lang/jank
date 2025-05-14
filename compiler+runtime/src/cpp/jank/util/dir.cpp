#include <clang/Basic/Version.h>
#include <llvm/TargetParser/Host.h>

#include <jank/util/dir.hpp>
#include <jank/util/sha256.hpp>
#include <jank/util/string_builder.hpp>
#include <jank/util/fmt.hpp>

namespace jank::util
{
  jtl::immutable_string const &user_home_dir()
  {
    static jtl::immutable_string res;
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

  jtl::immutable_string const &user_cache_dir()
  {
    static jtl::immutable_string res;
    if(!res.empty())
    {
      return res;
    }

    auto const home(getenv("XDG_CACHE_HOME"));
    if(home)
    {
      res = util::format("{}/jank", home);
      return res;
    }
    res = util::format("{}/.cache/jank", user_home_dir());
    return res;
  }

  jtl::immutable_string const &user_config_dir()
  {
    static jtl::immutable_string res;
    if(!res.empty())
    {
      return res;
    }

    auto const home(getenv("XDG_CONFIG_HOME"));
    if(home)
    {
      res = util::format("{}/jank", home);
      return res;
    }
    res = util::format("{}/.config/jank", user_home_dir());
    return res;
  }

  jtl::immutable_string const &
  binary_cache_dir(i64 const optimization_level,
                   native_vector<jtl::immutable_string> const &includes,
                   native_vector<jtl::immutable_string> const &defines)
  {
    static jtl::immutable_string res;
    if(!res.empty())
    {
      return res;
    }

    return res = util::format("target/{}", binary_version(optimization_level, includes, defines));
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
  jtl::immutable_string const &binary_version(i64 const optimization_level,
                                              native_vector<jtl::immutable_string> const &includes,
                                              native_vector<jtl::immutable_string> const &defines)
  {
    static jtl::immutable_string res;
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

    auto const input(util::format("{}.{}.{}.{}.{}",
                                  JANK_VERSION,
                                  clang::getClangRevision(),
                                  JANK_JIT_FLAGS,
                                  optimization_level,
                                  sb.release()));
    res = util::format("{}-{}", llvm::sys::getDefaultTargetTriple(), util::sha256(input));

    //util::println("binary_version {}", res);

    return res;
  }
}
