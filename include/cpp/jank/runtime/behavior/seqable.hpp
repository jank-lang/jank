#pragma once

#include <jank/native_box.hpp>
#include <jank/runtime/detail/object_util.hpp>
#include <jank/runtime/behavior/consable.hpp>

namespace jank::runtime::behavior
{
  /* TODO: Return ptr to nil on empty seq. */
  template <typename T>
  concept seqable = requires(T * const t) {
    /* Returns a (potentially shared) seq, which could just be `this`, if we're already a
     * seq. However, must return a nullptr for empty seqs. Returning a non-null pointer to
     * an empty seq is UB. */
    {
      t->seq()
    } -> std::convertible_to<object_ptr>;

    /* Returns a unique seq which can be updated in place. This is an optimization which allows
     * one allocation for a fresh seq which can then be mutated any number of times to traverse
     * the data. Also must return nullptr when the sequence is empty. */
    {
      t->fresh_seq()
    } -> std::convertible_to<object_ptr>;
  };

  template <typename T>
  concept sequenceable = requires(T * const t) {
    {
      t->first()
    } -> std::convertible_to<object_ptr>;

    /* Steps the sequence forward and returns nullptr if there is no more remaining
     * or a pointer to the remaining sequence.
     *
     * Next must always return a fresh seq. */
    {
      t->next()
    }; // -> sequenceable;
  } && consable<T>;

  template <typename T>
  concept sequenceable_in_place = requires(T * const t) {
    /* Each call to next() allocates a new sequence_ptr, since it's polymorphic. When iterating
     * over a large sequence, this can mean a _lot_ of allocations. However, if you own the
     * sequence_ptr you have, typically meaning it wasn't a parameter, then you can mutate it
     * in place using this function. No allocations will happen.
     *
     * If you don't own your sequence_ptr, you can call next() on it once, to get one you
     * do own, and then next_in_place() on that to your heart's content. */
    {
      t->next_in_place()
    }; // -> sequenceable;
  };

  namespace detail
  {
    template <typename It>
    void to_string(It const &begin,
                   It const &end,
                   native_persistent_string_view const open,
                   char const close,
                   fmt::memory_buffer &buff)
    {
      auto inserter(std::back_inserter(buff));
      for(auto const c : open)
      {
        inserter = c;
      }
      for(auto i(begin); i != end; ++i)
      {
        runtime::detail::to_string(*i, buff);
        auto n(i);
        if(++n != end)
        {
          inserter = ' ';
        }
      }
      inserter = close;
    }
  }
}
