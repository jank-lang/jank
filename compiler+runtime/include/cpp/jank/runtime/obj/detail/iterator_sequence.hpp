#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using cons_ref = oref<struct cons>;
}

namespace jank::runtime::obj::detail
{
  template <typename Derived, typename It>
  struct iterator_sequence
  {
    /* NOLINTNEXTLINE(bugprone-crtp-constructor-accessibility) */
    iterator_sequence() = default;

    /* NOLINTNEXTLINE(bugprone-crtp-constructor-accessibility) */
    iterator_sequence(object_ref const &c, It const &b, It const &e, usize const s);

    /* behavior::object_like */
    bool equal(object const &o) const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_string() const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::seqable */
    oref<Derived> seq();
    oref<Derived> fresh_seq() const;

    /* behavior::countable */
    usize count() const;

    /* behavior::sequenceable */
    object_ref first() const;

    oref<Derived> next() const;

    /* behavior::sequenceable_in_place */
    oref<Derived> next_in_place();

    /* behavior::conjable */
    obj::cons_ref conj(object_ref const head);

    object_ref coll{};
    /* Not default constructible. */
    It begin, end;
    usize size{};
  };
}
