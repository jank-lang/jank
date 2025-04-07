#include <regex>

#include <cpptrace/from_current.hpp>
#include <cpptrace/formatting.hpp>

#include <jank/util/try.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/error.hpp>
#include <jank/error/report.hpp>

namespace jank::util
{
  /* jank's stack frame symbols can be immense, due to templated return and
   * parameter types. For a quick glance at a stacktrace to see what's up,
   * we don't need more than the function name, file, and line. */
  static cpptrace::stacktrace_frame strip_frame_symbol(cpptrace::stacktrace_frame &&frame)
  {
    auto const paren{ frame.symbol.find('(') };
    if(paren == std::string::npos)
    {
      /* TODO: Remove this once cpptrace supports JIT frames. */
      if(frame.symbol.empty())
      {
        frame.symbol = "<jank jit frame -- not yet supported>";
      }

      return std::move(frame);
    }

    /* Remove the parameters. */
    frame.symbol.erase(paren);

    static std::regex const template_return_type{ "^.*>\\s+[a-zA-Z]" };
    std::smatch match;
    if(std::regex_search(frame.symbol, match, template_return_type))
    {
      frame.symbol.erase(0, match.length() - 1);
      return std::move(frame);
    }

    static std::regex const normal_return_type{ "^.*\\s+[a-zA-Z]" };
    if(std::regex_search(frame.symbol, match, normal_return_type))
    {
      frame.symbol.erase(0, match.length() - 1);
      return std::move(frame);
    }

    return std::move(frame);
  }

  /* Returns true if a frame should be kept. This allows us to trim out some pipework
   * at the start/end of each stack trace. */
  static bool filter_frame(cpptrace::stacktrace_frame const &frame)
  {
    static std::set<native_persistent_string_view> const symbols_to_ignore{
      /* (Top) Linux exception pipework. */
      "get_adjusted_ptr",
      "__gxx_personality_v0",
      "_Unwind_RaiseException",
      "__cxa_throw",

      /* (Bottom) Linux startup pipework. */
      "__libc_start_call_main",
      "__libc_start_main_alias_1",
      "_start"
    };
    return !symbols_to_ignore.contains(frame.symbol);
  }

  static auto const formatter{ cpptrace::formatter{}
                                 .header("Stack trace (most recent call first):")
                                 .addresses(cpptrace::formatter::address_mode::none)
                                 .paths(cpptrace::formatter::path_mode::basename)
                                 .columns(false)
                                 .snippets(false)
                                 .transform(&strip_frame_symbol)
                                 .filtered_frame_placeholders(false)
                                 .filter(&filter_frame) };

  static void print_exception_stack_trace()
  {
    formatter.print(cpptrace::from_current_exception());
  }

  static void print_exception_stack_trace(cpptrace::stacktrace const &trace)
  {
    formatter.print(trace);
  }

  void print_exception(std::exception const &e)
  {
    util::println("Uncaught exception: {}", e.what());
    print_exception_stack_trace();
  }

  void print_exception(runtime::object_ptr const e)
  {
    if(e->type == runtime::object_type::persistent_string)
    {
      util::println("Uncaught exception: {}", runtime::to_string(e));
    }
    else
    {
      util::println("Uncaught exception: {}", runtime::to_code_string(e));
    }
    print_exception_stack_trace();
  }

  void print_exception(jtl::immutable_string const &e)
  {
    util::println("Uncaught exception: {}", e);
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
    cpptrace::stacktrace *deepest_trace{ original->trace.get() };
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
