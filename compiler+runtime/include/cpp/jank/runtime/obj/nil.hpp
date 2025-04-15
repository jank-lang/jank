#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using persistent_array_map_ref = oref<struct persistent_array_map>;
  using cons_ref = oref<struct cons>;
  using nil_ref = oref<struct nil>;

  struct nil : gc
  {
    static constexpr object_type obj_type{ object_type::nil };
    static constexpr native_bool pointer_free{ true };

    static nil_ref nil_const();

    nil() = default;

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string const &to_string() const;
    jtl::immutable_string const &to_code_string() const;
    void to_string(util::string_builder &buff) const;
    native_hash to_hash() const;

    /* behavior::comparable */
    native_integer compare(object const &) const;

    /* behavior::comparable extended */
    native_integer compare(nil const &) const;

    /* behavior::associatively_readable */
    object_ref get(object_ref const key);
    object_ref get(object_ref const key, object_ref const fallback);
    object_ref get_entry(object_ref key);
    native_bool contains(object_ref key) const;

    /* behavior::associatively_writable */
    obj::persistent_array_map_ref assoc(object_ref key, object_ref val) const;
    nil_ref dissoc(object_ref key) const;

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
}
