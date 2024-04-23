#pragma once

#include <jank/runtime/detail/object_util.hpp>
#include <jank/runtime/obj/cons.hpp>

namespace jank::runtime::obj::detail
{
  template <typename Derived, typename It>
  struct iterator_sequence
  {
    iterator_sequence() = default;

    iterator_sequence(object_ptr const &c, It const &b, It const &e, size_t const s)
      : coll{ c }
      , begin{ b }
      , end{ e }
      , size{ s }
    {
      if(begin == end)
      {
        throw std::runtime_error{ "iterator_sequence for empty sequence" };
      }
    }

    /* behavior::objectable */
    native_bool equal(object const &o) const
    {
      return visit_object(
        [this](auto const typed_o) {
          using T = typename decltype(typed_o)::value_type;

          if constexpr(!behavior::seqable<T>)
          {
            return false;
          }
          else
          {
            auto seq(typed_o->seq());
            for(auto it(begin); it != end; ++it, seq = seq->next_in_place())
            {
              if(seq == nullptr || !runtime::detail::equal(*it, seq->first()))
              {
                return false;
              }
            }
            return true;
          }
        },
        &o);
    }

    void to_string(fmt::memory_buffer &buff) const
    {
      return behavior::detail::to_string(begin, end, "(", ')', buff);
    }

    native_persistent_string to_string() const
    {
      fmt::memory_buffer buff;
      behavior::detail::to_string(begin, end, "(", ')', buff);
      return native_persistent_string{ buff.data(), buff.size() };
    }

    native_hash to_hash() const
    {
      return hash::ordered(begin, end);
    }

    /* behavior::seqable */
    native_box<Derived> seq()
    {
      return static_cast<Derived *>(this);
    }

    native_box<Derived> fresh_seq() const
    {
      return make_box<Derived>(coll, begin, end, size);
    }

    /* behavior::countable */
    size_t count() const
    {
      return size;
    }

    /* behavior::sequenceable */
    object_ptr first() const
    {
      return *begin;
    }

    native_box<Derived> next() const
    {
      auto n(begin);
      ++n;

      if(n == end)
      {
        return nullptr;
      }

      return make_box<Derived>(coll, n, end, size);
    }

    /* behavior::sequenceable_in_place */
    native_box<Derived> next_in_place()
    {
      ++begin;

      if(begin == end)
      {
        return nullptr;
      }

      return static_cast<Derived *>(this);
    }

    obj::cons_ptr conj(object_ptr const head)
    {
      return make_box<obj::cons>(head, static_cast<Derived *>(this));
    }

    object_ptr coll{};
    /* Not default constructible. */
    It begin, end;
    size_t size{};
  };
}
