#pragma once

#include <jank/runtime/obj/symbol.hpp>
#include <jank/analyze/expression_base.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  struct local_reference : expression_base
  {
    obj::symbol_ptr name{};
    local_binding const &binding;

    object_ptr to_runtime_data() const
    {
      return merge(
        static_cast<expression_base const *>(this)->to_runtime_data(),
        obj::persistent_array_map::create_unique(make_box("__type"),
                                                 make_box("expr::local_reference"),
                                                 make_box("name"),
                                                 name,
                                                 make_box("binding"),
                                                 jank::detail::to_runtime_data(binding)));
    }
  };
}
