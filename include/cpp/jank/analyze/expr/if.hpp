#pragma once

#include <memory>

#include <jank/option.hpp>
#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct if_ : expression_base
  {
    native_box<E> condition{};
    native_box<E> then{};
    option<native_box<E>> else_;
  };
}
