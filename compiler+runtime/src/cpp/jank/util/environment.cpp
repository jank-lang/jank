#if defined(__APPLE__)
  #include <mach-o/dyld.h>
  #include <string>
#endif

#include <filesystem>

#include <jank/util/environment.hpp>
#include <jank/util/fmt.hpp>
#include <jank/error/system.hpp>

#ifdef JANK_WINDOWS_LIKE
  // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
  #define WIN32_LEAN_AND_MEAN 1
  #include <windows.h>
#endif

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
#ifdef JANK_WINDOWS_LIKE
    else if(char const *userprofile = getenv("USERPROFILE"))
    {
      res = userprofile;
    }
    else if(char const *drive = getenv("HOMEDRIVE"))
    {
      if(char const *path = getenv("HOMEPATH"))
      {
        res = std::string(drive) + path;
      }
    }
#endif
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
#elif defined(JANK_WINDOWS_LIKE)
    char path[MAX_PATH]{};
    const DWORD size{ GetModuleFileNameA(nullptr, path, MAX_PATH) };

    if(size == 0)
    {
      return {};
    }
    return { path, size };
#else
    static_assert(false, "Unsupported platform");
#endif
  }

  jtl::immutable_string process_dir()
  {
    return std::filesystem::path{ process_path().c_str() }.parent_path().string();
  }
}
