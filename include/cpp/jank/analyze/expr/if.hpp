#pragma once

#include <memory>

#include <jank/option.hpp>
#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct if_ : expression_base
  {
    std::shared_ptr<E> condition;
    std::shared_ptr<E> then;
    option<std::shared_ptr<E>> else_;
  };
}
