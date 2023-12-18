#pragma once

#include <memory>

#include <jank/option.hpp>
#include <jank/analyze/expression_base.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct if_ : expression_base
  {
    native_box<E> condition{};
    native_box<E> then{};
    option<native_box<E>> else_;

    runtime::object_ptr to_runtime_data() const
    {
      return runtime::obj::persistent_array_map::create_unique
      (
        make_box("__type"), make_box("expr::if"),
        make_box("condition"), detail::to_runtime_data(*condition),
        make_box("then"), detail::to_runtime_data(*then),
        make_box("else"), detail::to_runtime_data(else_)
      );
    }
  };
}
