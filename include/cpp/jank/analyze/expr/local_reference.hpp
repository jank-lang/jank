#pragma once

#include <jank/runtime/obj/symbol.hpp>
#include <jank/analyze/expression_base.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  struct local_reference : expression_base
  {
    runtime::obj::symbol_ptr name{};
    local_binding const &binding;

    runtime::object_ptr to_runtime_data() const
    {
      return runtime::merge(
        static_cast<expression_base const *>(this)->to_runtime_data(),
        runtime::obj::persistent_array_map::create_unique(make_box("__type"),
                                                          make_box("expr::local_reference"),
                                                          make_box("name"),
                                                          name,
                                                          make_box("binding"),
                                                          detail::to_runtime_data(binding)));
    }
  };
}
