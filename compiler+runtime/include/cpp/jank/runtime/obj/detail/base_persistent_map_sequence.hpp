#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>

namespace jank::runtime
{
  void to_string(object_ptr o, util::string_builder &buff);
  void to_code_string(object_ptr o, util::string_builder &buff);

  namespace obj
  {
    using cons_ptr = native_box<struct cons>;
  }
}

namespace jank::runtime::obj::detail
{
  template <typename PT, typename IT>
  struct base_persistent_map_sequence : gc
  {
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    base_persistent_map_sequence() = default;
    base_persistent_map_sequence(base_persistent_map_sequence &&) = default;
    base_persistent_map_sequence(base_persistent_map_sequence const &) = default;
    base_persistent_map_sequence(object_ptr const c, IT const &b, IT const &e);

    /* behavior::object_like */
    native_bool equal(object const &o) const;
    void to_string_impl(util::string_builder &buff, native_bool const to_code) const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_string() const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::seqable */
    native_box<PT> seq();
    native_box<PT> fresh_seq() const;

    /* behavior::sequenceable */
    obj::persistent_vector_ptr first() const;
    native_box<PT> next() const;

    /* behavior::sequenceable_in_place */
    native_box<PT> next_in_place();

    /* behavior::conjable */
    obj::cons_ptr conj(object_ptr const head);

    object base{ PT::obj_type };
    object_ptr coll{};
    IT begin{}, end{};
  };
}
