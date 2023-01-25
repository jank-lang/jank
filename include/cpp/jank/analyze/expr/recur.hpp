#pragma once

#include <memory>

#include <jank/runtime/obj/list.hpp>
#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct recur : expression_base
  {
    runtime::obj::list_ptr args{};
    native_vector<native_box<E>> arg_exprs;
  };
}
