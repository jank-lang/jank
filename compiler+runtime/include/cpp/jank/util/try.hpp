#pragma once

#include <cpptrace/from_current.hpp>

#include <jank/runtime/object.hpp>

namespace jank
{
  namespace error
  {
    struct base;
  }

  using error_ref = jtl::ref<error::base>;
}

namespace jank::util
{
  void print_exception(std::exception const &e);
  void print_exception(runtime::object_ref const e);
  void print_exception(error_ref e);
}
