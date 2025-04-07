#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using cons_ref = jtl::object_ref<struct cons>;
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
    jtl::object_ref<Derived> seq();
    jtl::object_ref<Derived> fresh_seq() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::sequenceable */
    object_ptr first() const;

    jtl::object_ref<Derived> next() const;

    /* behavior::sequenceable_in_place */
    jtl::object_ref<Derived> next_in_place();

    /* behavior::conjable */
    obj::cons_ref conj(object_ptr const head);

    object_ptr coll{};
    /* Not default constructible. */
    It begin, end;
    size_t size{};
  };
}
