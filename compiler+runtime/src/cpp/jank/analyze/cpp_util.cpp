#include <clang/Interpreter/CppInterOp.h>

#include <jank/analyze/cpp_util.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::analyze::cpp_util
{
  jtl::ptr<void> resolve_type(jtl::immutable_string const &sym)
  {
    return Cpp::GetType(sym);
  }

  /* std.string.iterator gives us the iterator in std::string. */
  jtl::string_result<jtl::ptr<void>> resolve_scope(jtl::immutable_string const &sym)
  {
    jtl::ptr<void> scope{ Cpp::GetGlobalScope() };
    usize new_start{};
    while(true)
    {
      auto const dot{ sym.find('.', new_start) };
      if(dot == jtl::immutable_string::npos)
      {
        /* Finding dots will still leave us with the last part of the symbol to lookup. */
        scope = Cpp::GetNamed(sym.substr(new_start), scope);
        break;
      }
      auto const subs{ sym.substr(new_start, dot - new_start) };
      new_start = dot + 1;
      scope = Cpp::GetNamed(subs, scope);
      if(!scope)
      {
        return err(util::format("Unable to find scope for symbol '{}'", sym));
      }
    }

    if(scope)
    {
      return ok(scope);
    }
    return err(util::format("Unable to find scope for symbol '{}'", sym));
  }
}
