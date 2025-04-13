#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using persistent_array_map_ref = jtl::oref<struct persistent_array_map>;
  using cons_ref = jtl::oref<struct cons>;
  using nil_ref = jtl::oref<struct nil>;

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
    object_ptr get(object_ptr const key);
    object_ptr get(object_ptr const key, object_ptr const fallback);
    object_ptr get_entry(object_ptr key);
    native_bool contains(object_ptr key) const;

    /* behavior::associatively_writable */
    obj::persistent_array_map_ref assoc(object_ptr key, object_ptr val) const;
    nil_ref dissoc(object_ptr key) const;

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
