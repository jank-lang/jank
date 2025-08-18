#include <filesystem>

#include <Interpreter/Compatibility.h>

#include <llvm/TargetParser/Host.h>

#include <ftxui/screen/screen.hpp>

#include <jtl/format/style.hpp>

#include <jank/environment/check_health.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/equal.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/util/clang.hpp>
#include <jank/util/dir.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/scope_exit.hpp>
#include <jank/util/try.hpp>

namespace jank::environment
{
  using jtl::terminal_style;

  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
  static bool fatal_error{};

  /* Each status report must start with one of:
   *
   * - ✅ for expected behavior
   * - ⚠️ for warnings
   * - ❌ for errors
   *
   * Colors for these should be green, yellow, and red. All paths should be blue. */

  static jtl::immutable_string system_os()
  {
    jtl::immutable_string os{ "unknown" };
    switch(jtl::current_platform)
    {
      case jtl::platform::linux_like:
        os = "linux";
        break;
      case jtl::platform::macos_like:
        os = "macos";
        break;
      case jtl::platform::windows_like:
        os = "windows";
        break;
      case jtl::platform::other_unix_like:
        os = "unix";
        break;
    }

    return util::format("{}─ ✅{} operating system: {}",
                        terminal_style::green,
                        terminal_style::reset,
                        os);
  }

  static jtl::immutable_string system_triple()
  {
    auto const &target_triple{ llvm::sys::getDefaultTargetTriple() };

    return util::format("{}─ ✅{} default triple: {}",
                        terminal_style::green,
                        terminal_style::reset,
                        target_triple);
  }

  static jtl::immutable_string jank_version()
  {
    return util::format("{}─ ✅{} jank version: {}",
                        terminal_style::green,
                        terminal_style::reset,
                        JANK_VERSION);
  }

  static jtl::immutable_string jank_asserts()
  {
#ifndef NDEBUG
    return util::format("{}─ ✅ jank assertions are enabled; performance will be impacted {}\n",
                        terminal_style::yellow,
                        terminal_style::reset);
#else
    return "";
#endif
  }

  static jtl::immutable_string jank_resource_dir()
  {
    std::filesystem::path dir{ JANK_RESOURCE_DIR };
    bool relative{};
    std::filesystem::path const jank_path{ util::process_dir().c_str() };
    if(!dir.is_absolute())
    {
      dir = (jank_path / dir);
      relative = true;
    }
    auto const exists{ std::filesystem::exists(dir) };
    auto const dev_build{ jank_path.filename() == "build"
                          && jank_path.parent_path().filename() == "compiler+runtime" };
    auto const error{ !exists && !dev_build };
    auto const icon{ error ? "❌" : "✅" };
    auto const col{ exists ? terminal_style::green
                           /* NOLINTNEXTLINE(readability-avoid-nested-conditional-operator) */
                           : (dev_build ? terminal_style::yellow : terminal_style::red) };

    fatal_error |= error;

    jtl::string_builder sb;
    util::format_to(sb,
                    "{}─ {}{} jank resource dir: {}{}{} {}{}{}",
                    col,
                    icon,
                    terminal_style::reset,
                    terminal_style::blue,
                    JANK_RESOURCE_DIR,
                    terminal_style::reset,
                    terminal_style::bright_black,
                    (exists ? " (found)" : " (not found)"),
                    terminal_style::reset);

    if(relative)
    {
      util::format_to(sb,
                      "\n{}─ {}{} jank resolved resource dir: {}{}{} {}{}{}",
                      col,
                      icon,
                      terminal_style::reset,
                      terminal_style::blue,
                      dir.c_str(),
                      terminal_style::reset,
                      terminal_style::bright_black,
                      /* TODO: Logic is not right here. */
                      dev_build ? " (ignored for dev build)" : " (not found)",
                      terminal_style::reset);
    }

    return sb.release();
  }

  static jtl::immutable_string jank_user_cache_dir()
  {
    auto const path{ util::user_cache_dir(util::binary_version()) };
    auto const configured_path_exists{ std::filesystem::exists(path.c_str()) };
    return util::format("{}─ ✅{} jank user cache dir: {}{}{} {}{}{}",
                        configured_path_exists ? terminal_style::green : terminal_style::yellow,
                        terminal_style::reset,
                        terminal_style::blue,
                        path,
                        terminal_style::reset,
                        terminal_style::bright_black,
                        configured_path_exists ? " (found)" : " (not found)",
                        terminal_style::reset);
  }

  static jtl::immutable_string clang_path()
  {
    auto const configured_path_exists{ std::filesystem::exists(JANK_CLANG_PATH) };
    jtl::string_builder sb;
    util::format_to(sb,
                    "{}─ {}{} configured clang path: {}{}{} {}{}{}",
                    configured_path_exists ? terminal_style::green : terminal_style::yellow,
                    configured_path_exists ? "✅" : "⚠️ ",
                    terminal_style::reset,
                    terminal_style::blue,
                    JANK_CLANG_PATH,
                    terminal_style::reset,
                    terminal_style::bright_black,
                    configured_path_exists ? "(found)" : "(not found)",
                    terminal_style::reset);

    auto const found_clang{ util::find_clang() };
    auto const found_path_exists{ found_clang.is_some()
                                    ? std::filesystem::exists(found_clang.unwrap().c_str())
                                    : false };
    if(found_path_exists && found_clang.unwrap() != JANK_CLANG_PATH)
    {
      util::format_to(sb,
                      "\n{}─ ✅{} runtime clang path: {}{}{} {}(found){}",
                      terminal_style::green,
                      terminal_style::reset,
                      terminal_style::blue,
                      found_clang.unwrap(),
                      terminal_style::reset,
                      terminal_style::bright_black,
                      terminal_style::reset);
    }
    else if(!found_path_exists)
    {
      fatal_error = true;
      util::format_to(sb,
                      "\n{}─ ❌ clang version {} not found in configured location or on PATH{}",
                      terminal_style::red,
                      JANK_CLANG_MAJOR_VERSION,
                      terminal_style::reset);
    }

    return sb.release();
  }

  static jtl::immutable_string clang_resource_root()
  {
    auto const configured_path_exists{ std::filesystem::exists(JANK_CLANG_RESOURCE_DIR) };
    jtl::string_builder sb;
    util::format_to(sb,
                    "{}─ {}{} configured clang resource dir: {}{}{} {}{}{}",
                    configured_path_exists ? terminal_style::green : terminal_style::yellow,
                    configured_path_exists ? "✅" : "⚠️ ",
                    terminal_style::reset,
                    terminal_style::blue,
                    JANK_CLANG_RESOURCE_DIR,
                    terminal_style::reset,
                    terminal_style::bright_black,
                    configured_path_exists ? "(found)" : "(not found)",
                    terminal_style::reset);

    auto const found_clang_resource_dir{ util::find_clang_resource_dir() };
    auto const found_path_exists{ found_clang_resource_dir.is_some()
                                    ? std::filesystem::exists(
                                        found_clang_resource_dir.unwrap().c_str())
                                    : false };
    if(found_path_exists && found_clang_resource_dir.unwrap() != JANK_CLANG_RESOURCE_DIR)
    {
      util::format_to(sb,
                      "\n{}─ ✅{} runtime clang resource dir: {}{}{} {}(found){}",
                      terminal_style::green,
                      terminal_style::reset,
                      terminal_style::blue,
                      found_clang_resource_dir.unwrap(),
                      terminal_style::reset,
                      terminal_style::bright_black,
                      terminal_style::reset);
    }
    else if(!found_path_exists)
    {
      fatal_error = true;
      util::format_to(sb,
                      "\n{}─ ❌ no viable clang version {} resource dir found{}",
                      terminal_style::red,
                      JANK_CLANG_MAJOR_VERSION,
                      terminal_style::reset);
    }

    return sb.release();
  }

  static jtl::immutable_string pch_location()
  {
    auto const pch_path{ util::find_pch(util::binary_version()) };
    if(pch_path.is_some())
    {
      return util::format("{}─ ✅{} jank pch path: {}{}{} {}(found){}",
                          terminal_style::green,
                          terminal_style::reset,
                          terminal_style::blue,
                          util::user_cache_dir(util::binary_version()),
                          terminal_style::reset,
                          terminal_style::bright_black,
                          terminal_style::reset);
    }

    return util::format(
      "{}─ ✅{} jank pch dir: {}{}{} {}(no pch found, should be built automatically){}",
      terminal_style::yellow,
      terminal_style::reset,
      terminal_style::blue,
      util::user_cache_dir(util::binary_version()),
      terminal_style::reset,
      terminal_style::bright_black,
      terminal_style::reset);
  }

  static jtl::immutable_string check_cpp_jit()
  {
    bool error{};
    auto def_err{ runtime::__rt_ctx->jit_prc.interpreter->ParseAndExecute(
      "std::string jank_cpp_health_check(){ return \"healthy\"; }") };
    if(def_err)
    {
      error = true;
    }
    else
    {
      clang::Value v;
      auto call_err{
        runtime::__rt_ctx->jit_prc.interpreter->ParseAndExecute("jank_cpp_health_check()", &v)
      };
      if(call_err)
      {
        error = true;
      }
      else
      {
        auto const s{ v.convertTo<std::string *>() };
        error = (s == nullptr || *s != "healthy");
      }
    }

    fatal_error |= error;

    if(error)
    {
      return util::format("{}─ ✅{} jank cannot jit compile c++",
                          terminal_style::red,
                          terminal_style::reset);
    }
    return util::format("{}─ ✅{} jank can jit compile c++",
                        terminal_style::green,
                        terminal_style::reset);
  }

  static jtl::immutable_string check_ir_jit()
  {
    bool error{};

    /* Force IR gen, regardless of CLI flags. We make sure to reset the old value, though. */
    auto const saved_codegen{ util::cli::opts.codegen };
    util::cli::opts.codegen = util::cli::codegen_type::llvm_ir;
    util::scope_exit const finally{ [=] { util::cli::opts.codegen = saved_codegen; } };

    JANK_TRY
    {
      auto const res{ runtime::__rt_ctx->eval_string("((fn* [] \"healthy\"))") };
      if(!runtime::equal(res, runtime::make_box("healthy")))
      {
        error = true;
      }
    }
    JANK_CATCH([&](auto const &e) {
      jank::util::print_exception(e);
      error = true;
    })


    fatal_error |= error;

    if(error)
    {
      return util::format("{}─ ✅{} jank cannot jit compile llvm ir",
                          terminal_style::red,
                          terminal_style::reset);
    }
    return util::format("{}─ ✅{} jank can jit compile llvm ir",
                        terminal_style::green,
                        terminal_style::reset);
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
                        terminal_style::bright_black,
                        terminal_style::bright_blue,
                        title,
                        terminal_style::bright_black,
                        padding,
                        terminal_style::reset);
  }

  /* Runs through the various jank systems and outputs to stdout various status reports.
   * This is meant to be useful for debugging distribution/installation issues.
   *
   * Returns whether or not jank is healthy. */
  bool check_health()
  {
    auto const terminal_width{ ftxui::Terminal::Size().dimx };
    auto const max_width{ std::min(terminal_width, 100) };

    util::println("{}", header("system", max_width));
    util::println("{}", system_os());
    util::println("{}", system_triple());
    util::println("");

    util::println("{}", header("jank install", max_width));
    util::println("{}", jank_version());
    util::print("{}", jank_asserts());
    util::println("{}", jank_resource_dir());
    util::println("{}", jank_user_cache_dir());
    util::println("{}", pch_location());
    util::println("");

    util::println("{}", header("clang install", max_width));
    util::println("{}", clang_path());
    util::println("{}", clang_resource_root());
    util::println("");

    /* If there's a fatal error with the install, don't even bother to check the runtime. */
    if(!fatal_error)
    {
      util::println("{}", header("jank runtime", max_width));
      runtime::__rt_ctx = new(GC) runtime::context{};
      util::println("{}─ ✅{} jank runtime initialized",
                    terminal_style::green,
                    terminal_style::reset);
      util::println("{}", check_cpp_jit());
      util::println("{}", check_ir_jit());
      /* TODO: Check AOT */
      util::println("");
    }


    util::println("{}", header("support", max_width));
    util::println(
      "If you're having issues with jank, please either "
      "ask the jank community on the Clojurians Slack or report the issue on Github.\n");
    util::println("─ Slack: {}https://clojurians.slack.com/archives/C03SRH97FDK{}",
                  terminal_style::underline,
                  terminal_style::no_underline);
    util::println("─ Github: {}https://github.com/jank-lang/jank{}",
                  terminal_style::underline,
                  terminal_style::no_underline);

    return !fatal_error;
  }
}
