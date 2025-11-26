#pragma once

#include <folly/Synchronized.h>

#include <mutex>

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime::obj
{
  using cons_ref = oref<struct cons>;
  using lazy_sequence_ref = oref<struct lazy_sequence>;

  /* TODO: IPending analog, to implement `realized?`. */
  struct lazy_sequence : gc
  {
    static constexpr object_type obj_type{ object_type::lazy_sequence };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };

    lazy_sequence() = default;
    lazy_sequence(lazy_sequence &&) noexcept = default;
    lazy_sequence(lazy_sequence const &);
    lazy_sequence &operator=(lazy_sequence const &);
    lazy_sequence(object_ref fn);
    lazy_sequence(object_ref fn, object_ref sequence);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(jtl::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    /* behavior::seqable */
    object_ref seq() const;
    lazy_sequence_ref fresh_seq() const;

    /* behavior::sequenceable */
    object_ref first() const;
    object_ref next() const;
    obj::cons_ref conj(object_ref head) const;

    /* behavior::sequenceable_in_place */
    //lazy_sequence_ref next_in_place();

    /* behavior::metadatable */
    lazy_sequence_ref with_meta(object_ref m) const;

  private:
    struct state
    {
      object_ref fn{};
      object_ref sv{};
      object_ref s{};
    };

    folly::Synchronized<state, std::recursive_mutex>::LockedPtr lock_state() const;
    void force_locked(folly::Synchronized<state, std::recursive_mutex>::LockedPtr &lock) const;

    object_ref resolve_fn() const;
    object_ref resolve_seq() const;

    void realize() const;
    void force() const;
    void lock_and_force() const;
    object_ref sval() const;
    object_ref unwrap(object_ref ls) const;

  public:
    object base{ obj_type };
    mutable folly::Synchronized<state, std::recursive_mutex> state_;
    jtl::option<object_ref> meta;
  };
}
