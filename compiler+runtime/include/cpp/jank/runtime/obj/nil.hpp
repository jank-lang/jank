#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using persistent_array_map_ref = oref<struct persistent_array_map>;
  using cons_ref = oref<struct cons>;
  using nil_ref = oref<struct nil>;

  struct nil
  {
    static constexpr object_type obj_type{ object_type::nil };
    static constexpr bool pointer_free{ true };

    nil() = default;

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string const &to_string() const;
    jtl::immutable_string const &to_code_string() const;
    void to_string(jtl::string_builder &buff) const;
    uhash to_hash() const;

    /* behavior::comparable */
    i64 compare(object const &) const;

    /* behavior::comparable extended */
    i64 compare(nil const &) const;

    /* behavior::associatively_readable */
    object_ref get(object_ref const key);
    object_ref get(object_ref const key, object_ref const fallback);
    object_ref get_entry(object_ref const key);
    bool contains(object_ref const key) const;

    /* behavior::associatively_writable */
    obj::persistent_array_map_ref assoc(object_ref const key, object_ref const val) const;
    nil_ref dissoc(object_ref const key) const;

    /* behavior::seqable */
    nil_ref seq();
    nil_ref fresh_seq() const;

    /* behavior::sequenceable */
    nil_ref first() const;
    nil_ref next() const;

    /* behavior::sequenceable_in_place */
    nil_ref next_in_place();

    object base{ obj_type };
  };
}

namespace jank::runtime
{
  bool operator==(object *, obj::nil_ref);
  bool operator!=(object *, obj::nil_ref);

  obj::nil_ref jank_nil();
}
