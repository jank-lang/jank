#include <algorithm>
#include <regex>

#include <cpptrace/from_current.hpp>
#include <cpptrace/formatting.hpp>
#include <cpptrace/gdb_jit.hpp>

#include <jank/util/try.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/error.hpp>
#include <jank/error/report.hpp>
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

    static std::regex const jtl_ref_expr{ R"(jtl::ref<jank::analyze::expression>\s*)" };
    static std::regex const jtl_ref_expr_t{ R"(jtl::ref<jank::analyze::expr::(\S+)>\s*)" };
    static std::regex const runtime_oref_object{
      R"(jank::runtime::oref<jank::runtime::object>\s*)"
    };
    static std::regex const runtime_oref_t{ R"(jank::runtime::oref<jank::runtime::(\S+)>\s*)" };

    frame.symbol = std::regex_replace(frame.symbol, jtl_ref_expr, "expression_ref");
    frame.symbol = std::regex_replace(frame.symbol, jtl_ref_expr_t, "expr::$1_ref");
    frame.symbol = std::regex_replace(frame.symbol, runtime_oref_object, "object_ref");
    frame.symbol = std::regex_replace(frame.symbol, runtime_oref_t, "$1_ref");

    return frame;
  }

  static cpptrace::stacktrace_frame
  resolve_materialized_object_frame(cpptrace::stacktrace_frame frame)
  {
    if((!frame.filename.empty() && frame.line.has_value()) || runtime::__rt_ctx == nullptr)
    {
      return prettify_symbols(jtl::move(frame));
    }

    cpptrace::jit_symbol_info jit_symbol;
    if(!cpptrace::lookup_jit_symbol(frame.raw_address, jit_symbol))
    {
      return prettify_symbols(jtl::move(frame));
    }

    auto const resolved{ runtime::__rt_ctx->jit_prc.lookup_materialized_object_frame(
      jit_symbol.symbol,
      frame.raw_address,
      jit_symbol.symbol_address,
      jit_symbol.symbol_size) };
    if(resolved.is_none())
    {
      return prettify_symbols(jtl::move(frame));
    }

    cpptrace::object_trace object_trace;
    object_trace.frames.push_back({ frame.raw_address,
                                    resolved.unwrap().object_address,
                                    resolved.unwrap().object_path.c_str() });
    auto const resolved_trace{ object_trace.resolve() };
    if(resolved_trace.frames.empty())
    {
      return prettify_symbols(jtl::move(frame));
    }

    auto const &resolved_frame{ resolved_trace.frames.front() };
    frame.object_address = resolved_frame.object_address;
    if(frame.filename.empty())
    {
      frame.filename = resolved_frame.filename;
    }
    if(!frame.line.has_value())
    {
      frame.line = resolved_frame.line;
    }
    if(!frame.column.has_value())
    {
      frame.column = resolved_frame.column;
    }
    if(frame.symbol.empty())
    {
      frame.symbol = resolved_frame.symbol.empty() ? jit_symbol.symbol : resolved_frame.symbol;
    }

    return prettify_symbols(jtl::move(frame));
  }

  static auto const formatter{ cpptrace::formatter{}
                                 .header("Stack trace (most recent call first):")
                                 //.addresses(cpptrace::formatter::address_mode::none)
                                 .paths(cpptrace::formatter::path_mode::basename)
                                 .columns(false)
                                 .snippets(false)
                                 .transform(&resolve_materialized_object_frame)
                                 //.transform(&strip_frame_symbol)
                                 .filtered_frame_placeholders(false)
                                 .filter(&filter_frame) };

  static void print_exception_stack_trace()
  {
    cpptrace::experimental::register_jit_objects_from_gdb_jit_interface();
    formatter.print(cpptrace::from_current_exception());
  }

  static void print_exception_stack_trace(cpptrace::stacktrace const &trace)
  {
    cpptrace::experimental::register_jit_objects_from_gdb_jit_interface();
    formatter.print(trace);
  }

  void print_exception(std::exception const &e)
  {
    util::println("Uncaught exception: {}\n", e.what());
    print_exception_stack_trace();
  }

  void print_exception(runtime::object_ref const e)
  {
    if(e.get_type() == runtime::object_type::persistent_string)
    {
      util::println("Uncaught exception: {}\n", e.to_string());
    }
    else
    {
      util::println("Uncaught exception: {}\n", e.to_code_string());
    }
    print_exception_stack_trace();
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
    cpptrace::stacktrace const *deepest_trace{ original->trace.get() };
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
