#pragma once

#include <unordered_map>
#include <string>
#include <functional>

#include <jank/runtime/obj/symbol.hpp>
#include <jank/option.hpp>

namespace jank::evaluate
{
  struct local_binding
  {
    runtime::obj::symbol_ptr name;
    runtime::object_ptr value;
  };

  struct frame
  {
    frame() = default;
    frame(frame const &) = default;
    frame(frame &&) noexcept = default;
    frame(option<std::reference_wrapper<frame const>> const &p)
      : parent{ p }
    { }

    option<local_binding> find(runtime::obj::symbol_ptr const &sym) const
    {
      auto const result(locals.find(sym));
      if(result != locals.end())
      { return result->second; }
      else if(parent.is_some())
      { return parent.unwrap().get().find(sym); }

      return none;
    }

    option<std::reference_wrapper<frame const>> parent;
    std::unordered_map<runtime::obj::symbol_ptr, local_binding> locals;
  };
}
