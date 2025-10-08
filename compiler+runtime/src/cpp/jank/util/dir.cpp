#if defined(__APPLE__)
  #include <mach-o/dyld.h>
  #include <string>
#endif

#include <filesystem>

#include <clang/Basic/Version.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Support/Program.h>

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
    /* TODO: Actual target triple. */
    res = util::format("{}-{}", llvm::sys::getDefaultTargetTriple(), util::sha256(input));

    //util::println("binary_version {}", res);

    return res;
  }

  jtl::immutable_string process_path()
  {
#if defined(__APPLE__)
    u32 path_length{};
    if(_NSGetExecutablePath(nullptr, &path_length) != -1 || path_length <= 1)
    {
      return "";
    }

    std::string path(path_length, std::string::value_type{});
    if(_NSGetExecutablePath(path.data(), &path_length) != 0)
    {
      return "";
    }
    return std::filesystem::canonical(path).string();
#elif defined(__linux__)
    return std::filesystem::canonical("/proc/self/exe").string();
#else
    static_assert(false, "Unsupported platform");
#endif
  }

  jtl::immutable_string process_dir()
  {
    return std::filesystem::path{ process_path().c_str() }.parent_path().c_str();
  }

  jtl::immutable_string resource_dir()
  {
    std::filesystem::path const dir{ JANK_RESOURCE_DIR };
    if(dir.is_absolute())
    {
      return dir.c_str();
    }
    else
    {
      std::filesystem::path const jank_path{ util::process_dir().c_str() };

      /* For a dev build, we don't worry about the configured resource dir not existing, since
       * we're running in-source. */
      auto const dev_build{ jank_path.filename() == "build"
                            && jank_path.parent_path().filename() == "compiler+runtime" };

      auto const configured_path{ (jank_path / dir) };
      if(std::filesystem::exists(configured_path) || dev_build)
      {
        return configured_path.c_str();
      }

      /* However, if the configured path doesn't exist, and we're not in a dev build, chances
       * are we're running an AOT compiled jank program. For that case, we want to find where
       * jank is and get its resource dir. */
      auto const installed_jank_path{ llvm::sys::findProgramByName("jank") };
      if(installed_jank_path)
      {
        return (*installed_jank_path / dir).c_str();
      }

      /* Otherwise, just return what we can and we'll raise an error down the road when we
       * fail to find things. */
      return configured_path.c_str();
    }
  }
}
