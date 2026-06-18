#pragma once

#include <folly/Synchronized.h>

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  using ns_ref = oref<struct ns>;

  /* Each runtime object which can hold metadata holds this `lazy_meta` instead. It's a
   * synchronized container of either a meta object or a string of EDN to lazily read/eval
   * as meta. Upon calling `get()`, if we have a source string, we'll read/eval and then
   * clear the string and just set the meta map.
   *
   * It's important to note that this synchronization is also needed to ensure changing
   * the meta on values is thread-safe. */
  struct lazy_meta
  {
    lazy_meta() = default;
    lazy_meta(jtl::immutable_string const &source);
    lazy_meta(jtl::immutable_string const &source, ns_ref const ns);
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
    ns_ref ns;
  };
}
