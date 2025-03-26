#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime::obj
{
  using cons_ptr = native_box<struct cons>;
  using lazy_sequence_ptr = native_box<struct lazy_sequence>;

  /* TODO: IPending analog, to implement `realized?`. */
  struct lazy_sequence : gc
  {
    static constexpr object_type obj_type{ object_type::lazy_sequence };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    lazy_sequence() = default;
    lazy_sequence(lazy_sequence &&) noexcept = default;
    lazy_sequence(lazy_sequence const &) = default;
    lazy_sequence(object_ptr fn);
    lazy_sequence(object_ptr fn, object_ptr sequence);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(util::string_builder &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::seqable */
    lazy_sequence_ptr seq() const;
    lazy_sequence_ptr fresh_seq() const;

    /* behavior::sequenceable */
    object_ptr first() const;
    lazy_sequence_ptr next() const;
    obj::cons_ptr conj(object_ptr head) const;

    /* behavior::sequenceable_in_place */
    lazy_sequence_ptr next_in_place();

    /* behavior::metadatable */
    lazy_sequence_ptr with_meta(object_ptr m) const;

  private:
    object_ptr resolve_fn() const;
    object_ptr resolve_seq() const;

  public:
    /* TODO: Synchronize. */
    object base{ obj_type };
    mutable object_ptr fn{};
    mutable object_ptr fn_result{};
    mutable object_ptr sequence{};
    jtl::option<object_ptr> meta;
  };
}
