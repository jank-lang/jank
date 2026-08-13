#include <algorithm>
#include <regex>

#include <cpptrace/from_current.hpp>
#include <cpptrace/formatting.hpp>

#include <jank/util/try.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/error.hpp>
#include <jank/error/report.hpp>
#include <jank/jit/object.hpp>
#include <jank/runtime/context.hpp>

namespace jank::util
{
  /* Returns true if a frame should be kept. This allows us to trim out some pipework
   * at the start/end of each stack trace. */
  static bool filter_frame(cpptrace::stacktrace_frame const &frame)
  {
    static native_set<jtl::immutable_string_view> const symbols_to_ignore{
      /* (Top) Linux exception pipework. */
      "get_adjusted_ptr",
      "__gxx_personality_v0",
      "_Unwind_RaiseException",
      "__cxa_throw",

      /* (Bottom) Linux startup pipework. */
      "__libc_start_call_main",
      "__libc_start_main_impl",
      "__libc_start_main_alias_1",
      "_start"
    };
    return !symbols_to_ignore.contains(frame.symbol)
      && frame.symbol.find("cpptrace::") == std::string::npos
      && frame.symbol.find("_ZN8cpptrace2v1") == std::string::npos;
  }

  static cpptrace::stacktrace_frame prettify_symbols(cpptrace::stacktrace_frame frame)
  {
    /* Clojure functions get turned from `clojure_core_subs_XXX_2`
     * into `clojure.core/subs (arity 2)`. */
    if(frame.filename.ends_with(".jank") || frame.filename.ends_with(".cljc"))
    {
      static std::regex const clojure_fn_name{ R"((\S+)_(\S+)_\d+_(\d+))" };
      std::smatch match;
      if(std::regex_search(frame.symbol, match, clojure_fn_name) && match.size() == 4)
      {
        auto ns{ match[1].str() };
        std::ranges::replace(ns, '_', '.');
        frame.symbol = util::format("{}/{} (arity {})", ns, match[2].str(), match[3].str());
        return frame;
      }
    }

    /* For non-Clojure functions, we want to clean up some common templates, since they
     * can be quite noisy. */
    static std::regex const jtl_ref_expr{ R"(jtl::ref<jank::analyze::expression>)" };
    static std::regex const jtl_ref_expr_t{ R"(jtl::ref<jank::analyze::expr::(\S+)>)" };
    static std::regex const runtime_oref_object{ R"(jank::runtime::oref<jank::runtime::object>)" };
    static std::regex const runtime_oref_t{ R"(jank::runtime::oref<jank::runtime::(\S+)>)" };
    static std::regex const runtime_obj{ R"(jank::runtime::obj::)" };
    static std::regex const auto_ret{ R"(^auto )" };

    frame.symbol = std::regex_replace(frame.symbol, jtl_ref_expr, "expression_ref");
    frame.symbol = std::regex_replace(frame.symbol, jtl_ref_expr_t, "expr::$1_ref");
    frame.symbol = std::regex_replace(frame.symbol, runtime_obj, "");
    frame.symbol = std::regex_replace(frame.symbol, runtime_oref_object, "object_ref");
    frame.symbol = std::regex_replace(frame.symbol, runtime_oref_t, "$1_ref");
    frame.symbol = std::regex_replace(frame.symbol, auto_ret, "");

    return frame;
  }

  static cpptrace::stacktrace_frame transform_frame(cpptrace::stacktrace_frame frame)
  {
    return prettify_symbols(jit::resolve_materialized_object_frame(jtl::move(frame)));
  }

  static auto const formatter{ cpptrace::formatter{}
                                 .header("Stack trace (most recent call first):")
                                 .addresses(cpptrace::formatter::address_mode::none)
                                 .paths(cpptrace::formatter::path_mode::basename)
                                 .symbols(cpptrace::formatter::symbol_mode::pruned)
                                 .colors(cpptrace::formatter::color_mode::automatic)
                                 .hide_exception_machinery(true)
                                 .columns(false)
                                 .snippets(false)
                                 .transform(&transform_frame)
                                 .filtered_frame_placeholders(false)
                                 .filter(&filter_frame) };

  static void print_exception_stack_trace(cpptrace::stacktrace const &trace)
  {
    formatter.print(trace);
  }

  static void print_exception_stack_trace(cpptrace::raw_trace const &trace)
  {
    formatter.print(trace.resolve());
  }

  cpptrace::stacktrace resolve(cpptrace::raw_trace const &trace)
  {
    auto resolved{ trace.resolve() };

    resolved.frames.erase(std::ranges::remove_if(resolved.frames,
                                                 [](cpptrace::stacktrace_frame const &frame) {
                                                   return !filter_frame(frame);
                                                 })
                            .begin(),
                          resolved.frames.end());

    for(auto &frame : resolved.frames)
    {
      frame = transform_frame(std::move(frame));
      frame.symbol = cpptrace::prune_symbol(frame.symbol);
      frame.filename = cpptrace::basename(frame.filename);
    }

    /* This was taken from cpptrace, but it's not public there. */
    // Look for c++ exception machinery and skip it if it's present, otherwise start at the beginning
    // On itanium this is identifiable by __cxa_throw
    // https://itanium-cxx-abi.github.io/cxx-abi/abi-eh.html 2.4.1
    // On windows this is identifiable by CxxThrowException (maybe with an underscore?)
    // https://www.youtube.com/watch?v=COEv2kq_Ht8 40:10
    // https://github.com/CppCon/CppCon2018/blob/master/Presentations/unwinding_the_stack_exploring_how_cpp_exceptions_work_on_windows/unwinding_the_stack_exploring_how_cpp_exceptions_work_on_windows__james_mcnellis__cppcon_2018.pdf slide 157
    // https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/cxxthrowexception?view=msvc-170
    auto const it{ std::ranges::find_if(resolved.frames,
                                        [](cpptrace::stacktrace_frame const &frame) {
                                          return frame.symbol == "__cxa_throw"
                                            || frame.symbol == "CxxThrowException"
                                            || frame.symbol == "_CxxThrowException";
                                        }) };
    auto const start{ it == resolved.end() ? 0 : it - resolved.begin() + 1 };
    resolved.frames.erase(resolved.frames.begin(), resolved.frames.begin() + start);

    return resolved;
  }

  static void print_exception(jtl::immutable_string const &message)
  {
    static native_set<jtl::immutable_string> const core_libs{
      "clojure_core",
      "clojure_core_reducers",
      "clojure_core_protocols",
      "clojure_data",
      "clojure_datafy",
      "clojure_edn",
      "clojure_instant",
      "clojure_main",
      "clojure_pprint",
      "clojure_reflect",
      "clojure_repl",
      "clojure_set",
      "clojure_spec_alpha",
      "clojure_spec_gen_alpha",
      "clojure_spec_test_alpha",
      "clojure_string",
      "clojure_template",
      "clojure_uuid",
      "clojure_walk",
      "clojure_xml",
      "clojure_zip",
    };

    static auto const is_core_lib{ [&](jtl::immutable_string const &symbol) {
      for(auto const &lib : core_libs)
      {
        if(symbol.starts_with(lib))
        {
          return true;
        }
      }
      return false;
    } };

    auto const &trace{ cpptrace::from_current_exception() };

    read::source source{ read::source::unknown() };
    for(auto const &frame : trace.frames)
    {
      auto const &resolved_frame(jit::resolve_materialized_object_frame(frame));
      auto const is_jank_file{ resolved_frame.filename.ends_with(".jank")
                               || resolved_frame.filename.ends_with(".cljc") };
      auto const is_core{ is_core_lib(resolved_frame.symbol) };
      if(is_jank_file && !is_core && resolved_frame.line.has_value())
      {
        read::source_position const location{ 0, resolved_frame.line.value(), 1 };
        source = read::source(resolved_frame.filename, "", location, location);
        break;
      }
    }

    auto const err{ make_error(error::kind::runtime_uncaught_exception, message, source) };
    err->notes.at(0).kind = error::note::kind::error_line;
    err->notes.at(0).message = "Found on this line. (No column info)";

    print_exception(err);
    print_exception_stack_trace(trace);
  }

  void print_exception(std::exception const &e)
  {
    print_exception(e.what());
  }

  void print_exception(runtime::object_ref const e)
  {
    if(e.get_type() == runtime::object_type::persistent_string)
    {
      print_exception(e.to_string());
    }
    else
    {
      print_exception(e.to_code_string());
    }
  }

  void print_exception(error_ref const e)
  {
    error::report(e);

    /* We want to find the deepest stack trace, since that will
     * be closest to the actual problem. However, if there is no
     * stack trace, we don't want to print it. We only have
     * stack traces for thrown exceptions, whereas errors from
     * compilation will not have exceptions. So this helps keep our
     * compiler error output cleaner, since the stack trace isn't
     * actually going to provide any useful info. */
    jtl::ptr<error::base> original{ e };
    auto const *deepest_trace{ original->trace.get() };
    while(original->cause)
    {
      original = original->cause;
      if(original->trace)
      {
        deepest_trace = original->trace.get();
      }
    }
    if(deepest_trace)
    {
      print_exception_stack_trace(*deepest_trace);
    }
  }
}
