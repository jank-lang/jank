#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using cons_ref = oref<struct cons>;
}

namespace jank::runtime::obj::detail
{
  template <typename Derived, typename It>
  struct iterator_sequence : object
  {
    static constexpr object_behavior obj_behaviors{ object_behavior::seqable
                                                    | object_behavior::fresh_seqable
                                                    | object_behavior::sequence_like
                                                    | object_behavior::sequence_like_in_place };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };

    /* NOLINTNEXTLINE(bugprone-crtp-constructor-accessibility) */
    iterator_sequence();

    /* NOLINTNEXTLINE(bugprone-crtp-constructor-accessibility) */
    iterator_sequence(object_ref const c, It const &b, It const &e, usize const s);

    /* behavior::object_like */
    bool equal(object const &o) const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_string() const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::seqable */
    object_ref seq() const override;

    /* behavior::fresh_seqable */
    object_ref fresh_seq() const override;

    /* behavior::countable */
    usize count() const;

    /* behavior::sequence_like */
    object_ref first() const override;
    object_ref next() const override;

    /* behavior::sequence_like_in_place */
    object_ref next_in_place() override;

    /* behavior::conjable */
    obj::cons_ref conj(object_ref const head);

    object_ref coll{};
    /* Not default constructible. */
    It begin, end;
    usize size{};
  };
}
