#pragma once

#include <folly/Synchronized.h>

#include <jank/runtime/object.hpp>
#include <jank/runtime/lazy_meta.hpp>

namespace jank::runtime
{
  struct lazy_meta
  {
    lazy_meta() = default;
    lazy_meta(jtl::immutable_string const &source);
    lazy_meta(object_ref const meta);

    object_ref get() const;
    void set(object_ref const o);

  private:
    struct mutable_state
    {
      /* If this is not empty, we need to read it to get our meta.
       * It's only read once and then cleared. */
      jtl::immutable_string source;
      object_ref meta;
    };

    mutable folly::Synchronized<mutable_state> state;
  };
}
