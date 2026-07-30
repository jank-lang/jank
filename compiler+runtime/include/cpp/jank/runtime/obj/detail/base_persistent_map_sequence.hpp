#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>

namespace jank::runtime
{
  void to_string(object_ref const o, jtl::string_builder &buff);
  void to_code_string(object_ref const o, jtl::string_builder &buff);

  namespace obj
  {
    using cons_ref = oref<struct cons>;
  }
}

namespace jank::runtime::obj::detail
{
  template <typename PT, typename IT>
  struct base_persistent_map_sequence : object
  {
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };
    static constexpr object_behavior obj_behaviors{ object_behavior::seqable
                                                    | object_behavior::sequence_like
                                                    | object_behavior::sequence_like_in_place };

    base_persistent_map_sequence(base_persistent_map_sequence &&) = default;
    base_persistent_map_sequence(base_persistent_map_sequence const &) = default;
    base_persistent_map_sequence(object_ref const c, IT const &b, IT const &e);

    /* behavior::object_like */
    bool equal(object const &o) const override;
    void to_string_impl(jtl::string_builder &buff, bool const to_code) const;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_string() const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::countable */
    usize count() const;

    /* behavior::seqable */
    object_ref seq() const override;
    object_ref fresh_seq() const override;

    /* behavior::sequence_like */
    object_ref first() const override;
    object_ref next() const override;

    /* behavior::sequence_like_in_place */
    object_ref next_in_place() override;

    /* behavior::conjable */
    obj::cons_ref conj(object_ref const head);

    /*** XXX: Everything here is immutable after initialization. ***/
    object_ref coll{};
    IT begin{}, end{};
  };
}
