#pragma once

#include <memory>

#include <jank/runtime/obj/symbol.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expression_base.hpp>
#include <jank/option.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct def : expression_base
  {
    runtime::obj::symbol_ptr name{};
    option<native_box<E>> value;

    runtime::object_ptr to_runtime_data() const
    {
      return runtime::obj::map::create_unique
      (
        make_box("__type"), make_box("expr::def"),
        make_box("name"), name,
        make_box("value"), detail::to_runtime_data(value)
      );
    }
  };
}
