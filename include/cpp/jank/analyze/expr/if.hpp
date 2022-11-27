#pragma once

#include <memory>

#include <jank/option.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct if_
  {
    std::shared_ptr<E> condition;
    std::shared_ptr<E> then;
    option<std::shared_ptr<E>> else_;
  };
}
