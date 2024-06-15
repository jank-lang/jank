#pragma once

#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/detail/object_util.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using persistent_list = static_object<object_type::persistent_list>;
    using persistent_list_ptr = native_box<persistent_list>;
  }

  template <typename T>
  requires behavior::seqable<T>
  auto seq(native_box<T> const s)
  {
    return s->seq();
  }

  object_ptr seq(object_ptr s);

  template <typename T>
  requires behavior::seqable<T>
  auto fresh_seq(native_box<T> const s)
  {
    return s->fresh_seq();
  }

  object_ptr fresh_seq(object_ptr s);

  template <typename T>
  requires behavior::sequenceable<T>
  auto next(native_box<T> const s)
  {
    return s->next();
  }

  object_ptr next(object_ptr s);

  template <typename T>
  requires behavior::sequenceable_in_place<T>
  auto next_in_place(native_box<T> const s)
  {
    return s->next_in_place();
  }

  template <typename T>
  requires(behavior::sequenceable<T> && !behavior::sequenceable_in_place<T>)
  auto next_in_place(native_box<T> const s)
  {
    /* Not all sequences can be updated in place. For those, just gracefully
     * do a normal next. */
    return s->next();
  }

  object_ptr next_in_place(object_ptr s);

  template <typename T>
  requires behavior::sequenceable<T>
  auto first(native_box<T> const s)
  {
    return s->first();
  }

  object_ptr first(object_ptr s);

  template <typename T>
  requires behavior::sequenceable<T>
  auto second(native_box<T> const s)
  {
    return next(s)->first();
  }

  object_ptr second(object_ptr s);

  native_bool is_nil(object_ptr o);
  native_bool is_some(object_ptr o);
  native_bool is_map(object_ptr o);
  object_ptr cons(object_ptr head, object_ptr tail);
  object_ptr conj(object_ptr s, object_ptr o);
  object_ptr disj(object_ptr s, object_ptr o);
  object_ptr assoc(object_ptr m, object_ptr k, object_ptr v);
  object_ptr dissoc(object_ptr m, object_ptr k);
  object_ptr get(object_ptr m, object_ptr key);
  object_ptr get(object_ptr m, object_ptr key, object_ptr fallback);
  object_ptr get_in(object_ptr m, object_ptr keys);
  object_ptr get_in(object_ptr m, object_ptr keys, object_ptr fallback);
  object_ptr find(object_ptr s, object_ptr key);
  native_bool contains(object_ptr s, object_ptr key);
  object_ptr merge(object_ptr m, object_ptr other);
  object_ptr meta(object_ptr m);
  object_ptr subvec(object_ptr o, native_integer start, native_integer end);
  object_ptr nth(object_ptr o, object_ptr idx);
  object_ptr nth(object_ptr o, object_ptr idx, object_ptr fallback);
  object_ptr peek(object_ptr o);
  object_ptr pop(object_ptr o);
  obj::persistent_list_ptr into_list(object_ptr s);

  namespace detail
  {
    size_t sequence_length(object_ptr const s);
    size_t sequence_length(object_ptr const s, size_t const max);

    template <typename T>
    requires behavior::sequenceable<T>
    void to_string(native_box<T> const s, fmt::memory_buffer &buff)
    {
      auto inserter(std::back_inserter(buff));
      if(!s)
      {
        fmt::format_to(inserter, "()");
        return;
      }

      fmt::format_to(inserter, "(");
      native_bool needs_space{};
      for(auto i(s->fresh_seq()); i != nullptr; i = runtime::next_in_place(i))
      {
        if(needs_space)
        {
          fmt::format_to(inserter, " ");
        }
        detail::to_string(first(i), buff);
        needs_space = true;
      }
      fmt::format_to(inserter, ")");
    }

    template <typename T>
    requires behavior::sequenceable<T>
    native_persistent_string to_string(native_box<T> const s)
    {
      fmt::memory_buffer buff;
      detail::to_string(s, buff);
      return native_persistent_string{ buff.data(), buff.size() };
    }

    template <typename It>
    native_bool equal(object const &o, It const begin, It const end)
    {
      return visit_object(
        [](auto const typed_o, auto const begin, auto const end) -> native_bool {
          using T = typename decltype(typed_o)::value_type;

          /* nil is seqable, but we don't want it to be equal to an empty collection.
           * An empty seq itself is nil, but that's different. */
          if constexpr(std::same_as<T, obj::nil> || !behavior::seqable<T>)
          {
            return false;
          }
          else
          {
            auto seq(typed_o->fresh_seq());
            auto it(begin);
            for(; it != end; ++it, seq = runtime::next_in_place(seq))
            {
              if(seq == nullptr || !runtime::detail::equal(*it, seq->first()))
              {
                return false;
              }
            }
            return seq == nullptr && it == end;
          }
        },
        &o,
        begin,
        end);
    }
  }
}
