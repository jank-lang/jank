#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using cons_ptr = native_box<struct cons>;
}

namespace jank::runtime::obj::detail
{
  template <typename Derived, typename It>
  struct iterator_sequence
  {
    /* NOLINTNEXTLINE(bugprone-crtp-constructor-accessibility) */
    iterator_sequence() = default;

    /* NOLINTNEXTLINE(bugprone-crtp-constructor-accessibility) */
    iterator_sequence(object_ptr const &c, It const &b, It const &e, size_t const s);

    /* behavior::object_like */
    native_bool equal(object const &o) const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_string() const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::seqable */
    native_box<Derived> seq();
    native_box<Derived> fresh_seq() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::sequenceable */
    object_ptr first() const;

    native_box<Derived> next() const;

    /* behavior::sequenceable_in_place */
    native_box<Derived> next_in_place();

    /* behavior::conjable */
    obj::cons_ptr conj(object_ptr const head);

    object_ptr coll{};
    /* Not default constructible. */
    It begin, end;
    size_t size{};
  };
}
