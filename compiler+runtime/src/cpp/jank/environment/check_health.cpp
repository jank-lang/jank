#include <filesystem>
#include <fstream>

#include <llvm/TargetParser/Host.h>
#include <llvm/Support/Program.h>

#include <CppInterOp/Compatibility.h>

#include <jtl/terminal.hpp>

#include <jank/environment/check_health.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/equal.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/aot/processor.hpp>
#include <jank/util/clang.hpp>
#include <jank/util/environment.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/scope_exit.hpp>
#include <jank/util/try.hpp>
#include <jank/c_api.h>

#include <clojure/core_native.hpp>

#ifdef JANK_PHASE_2
extern "C" void jank_load_clojure_core();
#endif

namespace jank::environment
{
  using jtl::terminal::text_style;

  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
  static bool fatal_error{};

  /* Each status report must start with one of:
   *
   * - ✅ for expected behavior
   * - ⚠️ for warnings
   * - ℹ️  for info
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

    return util::format("{}─ ✅{} operating system: {}", text_style::green, text_style::reset, os);
  }

  static jtl::immutable_string system_triple()
  {
    auto const &target_triple{ llvm::sys::getDefaultTargetTriple() };

    return util::format("{}─ ✅{} default triple: {}",
                        text_style::green,
                        text_style::reset,
                        target_triple);
  }

  static jtl::immutable_string jank_version()
  {
    return util::format("{}─ ✅{} jank version: {}",
                        text_style::green,
                        text_style::reset,
                        JANK_VERSION);
  }

  static jtl::immutable_string jank_cmake_build_type()
  {
    return util::format("{}─ ✅{} jank cmake build type: {}",
                        text_style::green,
                        text_style::reset,
                        JANK_CMAKE_BUILD_TYPE);
  }

  static jtl::immutable_string jank_asserts()
  {
#ifndef NDEBUG
    return util::format("{}─ ⚠️  jank assertions are enabled; performance will be impacted {}\n",
                        text_style::yellow,
                        text_style::reset);
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
    auto const col{ exists ? text_style::green
                           /* NOLINTNEXTLINE(readability-avoid-nested-conditional-operator) */
                           : (dev_build ? text_style::yellow : text_style::red) };

    fatal_error |= error;

    jtl::string_builder sb;
    util::format_to(sb,
                    "{}─ {}{} jank resource dir: {}{}{} {}{}{}",
                    col,
                    icon,
                    text_style::reset,
                    text_style::blue,
                    JANK_RESOURCE_DIR,
                    text_style::reset,
                    text_style::bright_black,
                    /* NOLINTNEXTLINE(readability-avoid-nested-conditional-operator) */
                    (relative ? "" : (exists ? " (found)" : " (not found)")),
                    text_style::reset);

    if(relative)
    {
      util::format_to(
        sb,
        "\n{}─ {}{} jank resolved resource dir: {}{}{} {}{}{}",
        col,
        icon,
        text_style::reset,
        text_style::blue,
        dir.c_str(),
        text_style::reset,
        text_style::bright_black,
        /* NOLINTNEXTLINE(readability-avoid-nested-conditional-operator) */
        (exists ? "(found)" : (dev_build ? "(ignored for dev build)" : "(not found)")),
        text_style::reset);
    }

    return sb.release();
  }

  static jtl::immutable_string jank_user_cache_dir()
  {
    auto const path{ util::user_cache_dir(util::binary_version()) };
    auto const configured_path_exists{ std::filesystem::exists(path.c_str()) };
    return util::format("{}─ ✅{} jank user cache dir: {}{}{} {}{}{}",
                        configured_path_exists ? text_style::green : text_style::yellow,
                        text_style::reset,
                        text_style::blue,
                        path,
                        text_style::reset,
                        text_style::bright_black,
                        configured_path_exists ? "(found)" : "(not found)",
                        text_style::reset);
  }

  static jtl::immutable_string clang_path()
  {
    auto const configured_path_exists{ std::filesystem::exists(JANK_CLANG_PATH) };
    jtl::string_builder sb;
    util::format_to(sb,
                    "{}─ {}{} configured clang path: {}{}{} {}{}{}",
                    configured_path_exists ? text_style::green : text_style::yellow,
                    configured_path_exists ? "✅" : "ℹ️",
                    text_style::reset,
                    text_style::blue,
                    JANK_CLANG_PATH,
                    text_style::reset,
                    text_style::bright_black,
                    configured_path_exists ? "(found)" : "(not found)",
                    text_style::reset);

    auto const found_clang{ util::find_clang() };
    auto const found_path_exists{ found_clang.is_some()
                                    ? std::filesystem::exists(found_clang.unwrap().c_str())
                                    : false };
    if(found_path_exists && found_clang.unwrap() != JANK_CLANG_PATH)
    {
      util::format_to(sb,
                      "\n{}─ ✅{} runtime clang path: {}{}{} {}(found){}",
                      text_style::green,
                      text_style::reset,
                      text_style::blue,
                      found_clang.unwrap(),
                      text_style::reset,
                      text_style::bright_black,
                      text_style::reset);
    }
    else if(!found_path_exists)
    {
      fatal_error = true;
      util::format_to(sb,
                      "\n{}─ ❌ clang version {} not found in configured location or on PATH{}",
                      text_style::red,
                      JANK_CLANG_MAJOR_VERSION,
                      text_style::reset);
    }

    return sb.release();
  }

  static jtl::immutable_string clang_resource_root()
  {
    auto const configured_path_exists{ std::filesystem::exists(JANK_CLANG_RESOURCE_DIR) };
    jtl::string_builder sb;
    util::format_to(sb,
                    "{}─ {}{} configured clang resource dir: {}{}{} {}{}{}",
                    configured_path_exists ? text_style::green : text_style::yellow,
                    configured_path_exists ? "✅" : "ℹ️",
                    text_style::reset,
                    text_style::blue,
                    JANK_CLANG_RESOURCE_DIR,
                    text_style::reset,
                    text_style::bright_black,
                    configured_path_exists ? "(found)" : "(not found)",
                    text_style::reset);

    auto const found_clang_resource_dir{ util::find_clang_resource_dir() };
    auto const found_path_exists{ found_clang_resource_dir.is_some()
                                    ? std::filesystem::exists(
                                        found_clang_resource_dir.unwrap().c_str())
                                    : false };
    if(found_path_exists && found_clang_resource_dir.unwrap() != JANK_CLANG_RESOURCE_DIR)
    {
      util::format_to(sb,
                      "\n{}─ ✅{} runtime clang resource dir: {}{}{} {}(found){}",
                      text_style::green,
                      text_style::reset,
                      text_style::blue,
                      found_clang_resource_dir.unwrap(),
                      text_style::reset,
                      text_style::bright_black,
                      text_style::reset);
    }
    else if(!found_path_exists)
    {
      fatal_error = true;
      util::format_to(sb,
                      "\n{}─ ❌ no viable clang version {} resource dir found{}",
                      text_style::red,
                      JANK_CLANG_MAJOR_VERSION,
                      text_style::reset);
    }

    return sb.release();
  }

  static jtl::immutable_string pch_location()
  {
    auto const pch_path{ util::find_pch(util::binary_version()) };
    if(pch_path.is_some())
    {
      return util::format("{}─ ✅{} jank pch path: {}{}{} {}(found){}",
                          text_style::green,
                          text_style::reset,
                          text_style::blue,
                          util::user_cache_dir(util::binary_version()),
                          text_style::reset,
                          text_style::bright_black,
                          text_style::reset);
    }

    return util::format("{}─ ℹ️{} jank pch dir: {}{}{} {}(no pch found){}",
                        text_style::yellow,
                        text_style::reset,
                        text_style::blue,
                        util::user_cache_dir(util::binary_version()),
                        text_style::reset,
                        text_style::bright_black,
                        text_style::reset);
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
      return util::format("{}─ ❌{} jank cannot jit compile c++",
                          text_style::red,
                          text_style::reset);
    }
    return util::format("{}─ ✅{} jank can jit compile c++", text_style::green, text_style::reset);
  }

  static jtl::immutable_string check_aot()
  {
    if(std::getenv("JANK_SKIP_AOT_CHECK"))
    {
      return util::format("{}─ ℹ️{} skipped aot check since JANK_SKIP_AOT_CHECK is defined",
                          text_style::yellow,
                          text_style::reset);
    }

    bool error{};

    JANK_TRY
    {
      auto const tmp{ std::filesystem::temp_directory_path() };
      std::string path_tmp{ (tmp / "jank-aot-XXXXXX").string() };
      int const fd{ mkstemp(path_tmp.data()) };
      close(fd);
      std::filesystem::remove(path_tmp);
      std::filesystem::create_directories(path_tmp);

      {
        std::ofstream ofs{ std::filesystem::path{ path_tmp } / "health.jank" };
        ofs << "(ns health)\n(defn -main [& args] (println \"healthy\"))";
      }
      auto const exe{ "a.out" };
      auto const exe_path{ std::filesystem::path{ path_tmp } / exe };

      auto const saved_opts{ util::cli::opts };
      util::cli::opts.target_module = "health";
      util::cli::opts.output_target = util::cli::compilation_target::object;
      util::cli::opts.target_dir = path_tmp;
      util::cli::opts.build_dir = util::format("{}/_cache", path_tmp);
      util::cli::opts.output_filename = exe;
      util::cli::opts.module_path = path_tmp;
      util::scope_exit const finally{ /* NOLINTNEXTLINE(bugprone-exception-escape) */
                                      [=] { util::cli::opts = saved_opts; }
      };

#ifdef JANK_PHASE_2
      jank_load_clojure_core();
#else
      runtime::__rt_ctx->load_module("clojure.core", runtime::module::origin::latest).expect_ok();
#endif

      runtime::__rt_ctx->module_loader.add_path(path_tmp);
      runtime::__rt_ctx->compile_module(util::cli::opts.target_module).expect_ok();

      jank::aot::processor const aot_prc{};
      aot_prc.build_executable(util::cli::opts.target_module).expect_ok();

      auto const stdout_file{ std::filesystem::path{ path_tmp } / "stdout" };
      std::string const stdout_file_str{ stdout_file.string() };
      auto const proc_code{ llvm::sys::ExecuteAndWait(
        exe_path.string(),
        { exe_path.string() },
        std::nullopt,
        { std::nullopt, stdout_file_str.c_str(), std::nullopt },
        5) };
      if(proc_code != 0)
      {
        util::println(stderr, R"(Compiled program exited with code '{}'.)", proc_code);
        error = true;
      }

      std::ifstream ifs{ stdout_file };
      std::string line;
      std::getline(ifs, line);
      if(line != "healthy")
      {
        util::println(stderr, "{}", line);
        while(std::getline(ifs, line))
        {
          util::println(stderr, "{}", line);
        }
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
      return util::format("{}─ ❌{} jank cannot aot compile working binaries",
                          text_style::red,
                          text_style::reset);
    }
    return util::format("{}─ ✅{} jank can aot compile working binaries",
                        text_style::green,
                        text_style::reset);
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
                        text_style::bright_black,
                        text_style::bright_blue,
                        title,
                        text_style::bright_black,
                        padding,
                        text_style::reset);
  }

  /* Runs through the various jank systems and outputs to stdout various status reports.
   * This is meant to be useful for debugging distribution/installation issues.
   *
   * Returns whether or not jank is healthy. */
  bool check_health()
  {
    auto const terminal_width{ jtl::terminal::get_size().width };
    auto const max_width{ std::min(terminal_width, 100ull) };

    util::println("{}", header("system", max_width));
    util::println("{}", system_os());
    util::println("{}", system_triple());
    util::println("");

    util::println("{}", header("jank install", max_width));
    util::println("{}", jank_version());
    util::println("{}", jank_cmake_build_type());
    util::print("{}", jank_asserts());
    util::println("{}", jank_resource_dir());
    util::println("{}", jank_user_cache_dir());
    util::println("");

    util::println("{}", header("clang install", max_width));
    util::println("{}", clang_path());
    util::println("{}", clang_resource_root());
    util::println("");

    /* If there's a fatal error with the install, don't even bother to check the runtime. */
    if(!fatal_error)
    {
      util::println("{}", header("jank runtime", max_width));
      auto const ret{ jank_init_dynamic(0, nullptr, true, nullptr, 0, [](int const, char const **) {
        jank_load_clojure_core_native();
        util::println("{}─ ✅{} jank runtime initialized", text_style::green, text_style::reset);
        util::println("{}", pch_location());
        util::println("{}", check_cpp_jit());
        util::println("{}", check_aot());
        util::println("");

        return 0;
      }) };
      if(ret != 0)
      {
        fatal_error = true;
      }
    }


    util::println("{}", header("support", max_width));
    util::println(
      "If you're having issues with jank, please either "
      "ask the jank community on the Clojurians Slack or report the issue on Github.\n");
    util::println("─ Slack: {}https://clojurians.slack.com/archives/C03SRH97FDK{}",
                  text_style::underline,
                  text_style::no_underline);
    util::println("─ Github: {}https://github.com/jank-lang/jank{}",
                  text_style::underline,
                  text_style::no_underline);

    return !fatal_error;
  }
}
