#include <filesystem>

#include <ftxui/screen/screen.hpp>

#include <jtl/format/color.hpp>

#include <jank/environment/check_health.hpp>
#include <jank/util/process_location.hpp>
#include <jank/util/clang.hpp>
#include <jank/util/dir.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::environment
{
  /* Each status report must start with one of:
   *
   * - ✅ for expected behavior
   * - ⚠️ for warnings
   * - ❌ for errors
   *
   * Colors for these should be green, yellow, and red. All paths should be blue. */

  static jtl::immutable_string jank_version()
  {
    return util::format("{}─ ✅{} jank version: {}",
                        jtl::terminal_color::green,
                        jtl::terminal_color::reset,
                        JANK_VERSION);
  }

  static jtl::immutable_string jank_resource_dir()
  {
    std::filesystem::path dir{ JANK_RESOURCE_DIR };
    bool relative{};
    auto const jank_path{ util::process_location().unwrap().parent_path() };
    if(!dir.is_absolute())
    {
      dir = (jank_path / dir);
      relative = true;
    }
    auto const exists{ std::filesystem::exists(dir) };
    auto const dev_build{ jank_path.filename() == "build"
                          && jank_path.parent_path().filename() == "compiler+runtime" };
    auto const icon{ exists ? "✅"
                            /* NOLINTNEXTLINE(readability-avoid-nested-conditional-operator) */
                            : (dev_build ? "✅" : "❌") };
    auto const col{ exists ? jtl::terminal_color::green
                           /* NOLINTNEXTLINE(readability-avoid-nested-conditional-operator) */
                           : (dev_build ? jtl::terminal_color::yellow : jtl::terminal_color::red) };

    jtl::string_builder sb;
    util::format_to(sb,
                    "{}─ {}{} jank resource dir: {}{}{} {}{}{}",
                    col,
                    icon,
                    jtl::terminal_color::reset,
                    jtl::terminal_color::blue,
                    JANK_RESOURCE_DIR,
                    jtl::terminal_color::reset,
                    jtl::terminal_color::bright_black,
                    (exists ? " (found)" : " (not found)"),
                    jtl::terminal_color::reset);

    if(relative)
    {
      util::format_to(sb,
                      "\n{}─ {}{} jank resolved resource dir: {}{}{}",
                      col,
                      icon,
                      jtl::terminal_color::reset,
                      jtl::terminal_color::blue,
                      dir.c_str(),
                      jtl::terminal_color::reset,
                      jtl::terminal_color::bright_black,
                      dev_build ? " (ignored for dev build)" : " (not found)",
                      jtl::terminal_color::reset);
    }

    return sb.release();
  }

  static jtl::immutable_string jank_user_cache_dir()
  {
    auto const path{ util::user_cache_dir(util::binary_version()) };
    auto const configured_path_exists{ std::filesystem::exists(path.c_str()) };
    return util::format("{}─ ✅{} jank user cache dir: {}{}{} {}{}{}",
                        configured_path_exists ? jtl::terminal_color::green
                                               : jtl::terminal_color::yellow,
                        jtl::terminal_color::reset,
                        jtl::terminal_color::blue,
                        path,
                        jtl::terminal_color::reset,
                        jtl::terminal_color::bright_black,
                        configured_path_exists ? " (found)" : " (not found)",
                        jtl::terminal_color::reset);
  }

  static jtl::immutable_string clang_path()
  {
    auto const configured_path_exists{ std::filesystem::exists(JANK_CLANG_PATH) };
    jtl::string_builder sb;
    util::format_to(sb,
                    "{}─ {}{} configured clang path: {}{}{} {}{}{}",
                    configured_path_exists ? jtl::terminal_color::green
                                           : jtl::terminal_color::yellow,
                    configured_path_exists ? "✅" : "⚠️ ",
                    jtl::terminal_color::reset,
                    jtl::terminal_color::blue,
                    JANK_CLANG_PATH,
                    jtl::terminal_color::reset,
                    jtl::terminal_color::bright_black,
                    configured_path_exists ? "(found)" : "(not found)",
                    jtl::terminal_color::reset);

    auto const found_clang{ util::find_clang() };
    auto const found_path_exists{ found_clang.is_some()
                                    ? std::filesystem::exists(found_clang.unwrap().c_str())
                                    : false };
    if(found_path_exists && found_clang.unwrap() != JANK_CLANG_PATH)
    {
      util::format_to(sb,
                      "\n{}─ ✅{} runtime clang path: {}{}{} {}(found){}",
                      jtl::terminal_color::green,
                      jtl::terminal_color::reset,
                      jtl::terminal_color::blue,
                      found_clang.unwrap(),
                      jtl::terminal_color::reset,
                      jtl::terminal_color::bright_black,
                      jtl::terminal_color::reset);
    }
    else if(!found_path_exists)
    {
      util::format_to(sb,
                      "\n{}─ ❌ clang version {} not found in configured location or on PATH{}",
                      jtl::terminal_color::red,
                      JANK_CLANG_MAJOR_VERSION,
                      jtl::terminal_color::reset);
    }

    return sb.release();
  }

  static jtl::immutable_string clang_resource_root()
  {
    /* TODO: Runtime resource root. */
    auto const configured_path_exists{ std::filesystem::exists(JANK_CLANG_RESOURCE_DIR) };
    return util::format("{}─ {}{} configured clang resource dir: {}{}{} {}{}{}",
                        configured_path_exists ? jtl::terminal_color::green
                                               : jtl::terminal_color::yellow,
                        configured_path_exists ? "✅" : "⚠️ ",
                        jtl::terminal_color::reset,
                        jtl::terminal_color::blue,
                        JANK_CLANG_RESOURCE_DIR,
                        jtl::terminal_color::reset,
                        jtl::terminal_color::bright_black,
                        configured_path_exists ? " (found)" : " (not found)",
                        jtl::terminal_color::reset);
  }

  static jtl::immutable_string pch_location()
  {
    auto const pch_path{ util::find_pch(util::binary_version()) };
    if(pch_path.is_some())
    {
      return util::format("{}─ ✅{} jank pch path: {}{}{} {}(found){}",
                          jtl::terminal_color::green,
                          jtl::terminal_color::reset,
                          jtl::terminal_color::blue,
                          util::user_cache_dir(util::binary_version()),
                          jtl::terminal_color::reset,
                          jtl::terminal_color::bright_black,
                          jtl::terminal_color::reset);
    }

    return util::format(
      "{}─ ✅{} jank pch dir: {}{}{} {}(no pch found, should be built automatically){}",
      jtl::terminal_color::yellow,
      jtl::terminal_color::reset,
      jtl::terminal_color::blue,
      util::user_cache_dir(util::binary_version()),
      jtl::terminal_color::reset,
      jtl::terminal_color::bright_black,
      jtl::terminal_color::reset);
  }

  static jtl::immutable_string header(std::string const &title, usize const max_width)
  {
    auto const padding_count(max_width - 3 - title.size());
    std::string padding;
    for(usize i{}; i < padding_count; ++i)
    {
      padding.insert(padding.size(), "─");
    }
    return util::format("{}─ {}{} {}{}{}",
                        jtl::terminal_color::bright_black,
                        jtl::terminal_color::bright_blue,
                        title,
                        jtl::terminal_color::bright_black,
                        padding,
                        jtl::terminal_color::reset);
  }

  /* Runs through the various jank systems and outputs to stdout various status reports.
   * This is meant to be useful for debugging distribution/installation issues.
   *
   * Returns whether or not jank is healthy. */
  bool check_health()
  {
    auto const terminal_width{ ftxui::Terminal::Size().dimx };
    auto const max_width{ std::min(terminal_width, 100) };

    util::println("{}", header("jank install", max_width));
    util::println("{}", jank_version());
    util::println("{}", jank_resource_dir());
    util::println("{}", jank_user_cache_dir());
    util::println("{}", pch_location());
    util::println("");

    util::println("{}", header("clang install", max_width));
    util::println("{}", clang_path());
    util::println("{}", clang_resource_root());
    util::println("");

    //util::println("{}", header("jank runtime", max_width));

    return true;
  }
}
