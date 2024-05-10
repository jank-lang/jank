#pragma once

#include <jank/analyze/expression_base.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct catch_
  {
    runtime::obj::symbol_ptr sym{};
    do_<E> body{};

    runtime::object_ptr to_runtime_data() const
    {
      using namespace runtime::obj;

      return persistent_array_map::create_unique(make_box("__type"),
                                                 make_box("expr::try::catch"),
                                                 make_box("body"),
                                                 body.to_runtime_data(),
                                                 make_box("sym"),
                                                 sym);
    }
  };

  template <typename E>
  struct try_ : expression_base
  {
    do_<E> body{};
    catch_<E> catch_body{};
    option<do_<E>> finally_body{};

    runtime::object_ptr to_runtime_data() const
    {
      using namespace runtime::obj;

      return runtime::merge(
        static_cast<expression_base const *>(this)->to_runtime_data(),
        persistent_array_map::create_unique(make_box("__type"),
                                            make_box("expr::try"),
                                            make_box("body"),
                                            body.to_runtime_data(),
                                            make_box("catch"),
                                            jank::detail::to_runtime_data(catch_body),
                                            make_box("finally"),
                                            jank::detail::to_runtime_data(finally_body)));
    }
  };
}
