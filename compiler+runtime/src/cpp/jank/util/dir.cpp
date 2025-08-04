#include <clang/Basic/Version.h>
#include <llvm/TargetParser/Host.h>

#include <jtl/string_builder.hpp>

#include <jank/util/dir.hpp>
#include <jank/util/sha256.hpp>
#include <jank/util/cli.hpp>
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

  jtl::immutable_string const &user_cache_dir(jtl::immutable_string const &binary_version)
  {
    static jtl::immutable_string res;
    if(!res.empty())
    {
      return res;
    }

    auto const xdg_cache(getenv("XDG_CACHE_HOME"));
    if(xdg_cache)
    {
      res = util::format("{}/jank/{}", xdg_cache, binary_version);
      return res;
    }
    res = util::format("{}/.cache/jank/{}", user_home_dir(), binary_version);
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

  jtl::immutable_string const &binary_cache_dir(jtl::immutable_string const &binary_version)
  {
    static jtl::immutable_string res;
    if(!res.empty())
    {
      return res;
    }

    return res = util::format("target/{}", binary_version);
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
  jtl::immutable_string const &binary_version()
  {
    static jtl::immutable_string res;
    if(!res.empty())
    {
      return res;
    }

    jtl::string_builder sb;
    for(auto const &inc : util::cli::opts.include_dirs)
    {
      sb(inc);
    }

    sb(".");

    for(auto const &def : util::cli::opts.define_macros)
    {
      sb(def);
    }

    auto const input(util::format("{}.{}.{}.{}.{}.{}",
                                  JANK_VERSION,
                                  clang::getClangRevision(),
                                  JANK_JIT_FLAGS,
                                  util::cli::opts.optimization_level,
                                  static_cast<int>(util::cli::opts.codegen),
                                  sb.release()));
    res = util::format("{}-{}", util::default_target_triple(), util::sha256(input));

    //util::println("binary_version {}", res);

    return res;
  }
}
