#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  bool is_map(object_ref o);
  bool equal(object_ref l, object_ref r);
  void to_string(object_ref o, jtl::string_builder &buff);
  void to_code_string(object_ref o, jtl::string_builder &buff);

  namespace behavior::detail
  {
    object_ref validate_meta(object_ref const m);
  }
}

namespace jank::runtime::obj::detail
{
  /* Array maps and hash maps share a lot of common code, so we have a common base.
   * No virtual fns are used, so this structure won't survive release optimizations. */
  template <typename PT, typename ST, typename V>
  struct base_persistent_map : object
  {
    static constexpr bool pointer_free{ false };
    static constexpr bool is_map_like{ true };

    using value_type = V;

    base_persistent_map();
    base_persistent_map(jtl::option<object_ref> const &meta);

    /* behavior::object_like */
    bool equal(object const &o) const override;
    static void to_string_impl(typename V::const_iterator const &begin,
                               typename V::const_iterator const &end,
                               jtl::string_builder &buff,
                               bool const to_code);
    void to_string(jtl::string_builder &buff) const override;

    jtl::immutable_string to_string() const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::seqable */
    oref<ST> seq() const;
    oref<ST> fresh_seq() const;

    /* behavior::countable */
    usize count() const;

    /* behavior::metadatable */
    oref<PT> with_meta(object_ref const m) const;

    /* behavior::conjable */
    object_ref conj(object_ref const head) const;

    jtl::option<object_ref> meta;
    mutable uhash hash{};
  };
}
