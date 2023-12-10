#include <iostream>
#include <sstream>

#include <jank/runtime/util.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/vector.hpp>

namespace jank::runtime
{
  obj::persistent_hash_map::static_object(detail::native_array_map const &m, object_ptr const key, object_ptr const val)
  {
    detail::native_transient_hash_map transient;
    for(auto const &e : m)
    { transient.set(e.first, e.second); }
    transient.set(key, val);
    data = transient.persistent();
  }
  obj::persistent_hash_map::static_object(value_type &&d)
    : data{ std::move(d) }
  { }
  obj::persistent_hash_map::static_object(value_type const &d)
    : data{ d }
  { }
  obj::persistent_hash_map::static_object(runtime::detail::native_transient_hash_map &&d)
    : data{ d.persistent() }
  { }

  obj::persistent_hash_map_ptr obj::persistent_hash_map::create_from_seq(object_ptr const seq)
  {
    return make_box<obj::persistent_hash_map>
    (
      visit_object
      (
        [](auto const typed_seq) -> obj::persistent_hash_map::value_type
        {
          using T = typename decltype(typed_seq)::value_type;

          if constexpr(behavior::seqable<T>)
          {
            detail::native_transient_hash_map transient;
            for(auto it(typed_seq->fresh_seq()); it != nullptr; it = it->next_in_place())
            {
              auto const key(it->first());
              it = it->next_in_place();
              if(!it)
              {
                throw std::runtime_error
                { fmt::format("Odd number of elements: {}", typed_seq->to_string()) };
              }
              auto const val(it->first());
              transient.set(key, val);
            }
            return transient.persistent();
          }
          else
          {
            throw std::runtime_error
            { fmt::format("Not seqable: {}", typed_seq->to_string()) };
          }
        },
        seq
      )
    );
  }

  object_ptr obj::persistent_hash_map::get(object_ptr const key) const
  {
    auto const res(data.find(key));
    if(res)
    { return *res; }
    return obj::nil::nil_const();
  }
  object_ptr obj::persistent_hash_map::get(object_ptr const key, object_ptr const fallback) const
  {
    auto const res(data.find(key));
    if(res)
    { return *res; }
    return fallback;
  }

  object_ptr obj::persistent_hash_map::get_entry(object_ptr const key) const
  {
    auto const res(data.find(key));
    if(res)
    { return make_box<obj::vector>(key, *res); }
    return obj::nil::nil_const();
  }

  native_bool obj::persistent_hash_map::contains(object_ptr const key) const
  { return data.find(key); }

  obj::persistent_hash_map_ptr obj::persistent_hash_map::assoc(object_ptr const key, object_ptr const val) const
  {
    auto copy(data.set(key, val));
    return make_box<obj::persistent_hash_map>(std::move(copy));
  }

  obj::persistent_hash_map_ptr obj::persistent_hash_map::cons(object_ptr const head) const
  {
    if(head->type != object_type::vector)
    { throw std::runtime_error{ fmt::format("invalid map entry: {}", runtime::detail::to_string(head)) }; }

    auto const vec(expect_object<obj::vector>(head));
    if(vec->count() != 2)
    { throw std::runtime_error{ fmt::format("invalid map entry: {}", runtime::detail::to_string(head)) }; }

    auto copy(data.set(vec->data[0], vec->data[1]));
    return make_box<obj::persistent_hash_map>(std::move(copy));
  }
}
