#pragma once

#include <jank/runtime/context.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime::obj
{
  struct jit_function : object, behavior::callable, behavior::metadatable
  {
    static constexpr bool pointer_free{ false };

    native_bool equal(object const &rhs) const final
    { return this == &rhs; }

    native_string to_string() const final
    { return "jit function"; }

    native_integer to_hash() const final
    { return reinterpret_cast<native_integer>(this); }

    behavior::callable const* as_callable() const final
    { return this; }

    object_ptr with_meta(object_ptr const m) const final
    {
      auto const meta(validate_meta(m));
      const_cast<jit_function*>(this)->meta = meta;
      return const_cast<jit_function *>(this);
    }

    behavior::metadatable const* as_metadatable() const final
    { return this; }
  };
}
