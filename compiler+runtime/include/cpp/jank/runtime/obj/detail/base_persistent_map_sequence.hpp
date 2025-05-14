#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>

namespace jank::runtime
{
  void to_string(object_ref o, util::string_builder &buff);
  void to_code_string(object_ref o, util::string_builder &buff);

  namespace obj
  {
    using cons_ref = oref<struct cons>;
  }
}

namespace jank::runtime::obj::detail
{
  template <typename PT, typename IT>
  struct base_persistent_map_sequence : gc
  {
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };

    base_persistent_map_sequence() = default;
    base_persistent_map_sequence(base_persistent_map_sequence &&) = default;
    base_persistent_map_sequence(base_persistent_map_sequence const &) = default;
    base_persistent_map_sequence(object_ref const c, IT const &b, IT const &e);

    /* behavior::object_like */
    bool equal(object const &o) const;
    void to_string_impl(util::string_builder &buff, bool const to_code) const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_string() const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    /* behavior::countable */
    usize count() const;

    /* behavior::seqable */
    oref<PT> seq();
    oref<PT> fresh_seq() const;

    /* behavior::sequenceable */
    obj::persistent_vector_ref first() const;
    oref<PT> next() const;

    /* behavior::sequenceable_in_place */
    oref<PT> next_in_place();

    /* behavior::conjable */
    obj::cons_ref conj(object_ref const head);

    object base{ PT::obj_type };
    object_ref coll{};
    IT begin{}, end{};
  };
}
