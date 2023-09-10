#include <iostream>
#include <sstream>

#include <jank/runtime/util.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/list.hpp>

namespace jank::runtime
{
  obj::list::static_object(runtime::detail::native_persistent_list &&d)
    : data{ std::move(d) }
  { }
  obj::list::static_object(runtime::detail::native_persistent_list const &d)
    : data{ d }
  { }

  obj::list_ptr obj::list::create(object_ptr const s)
  {
    if(s == nullptr)
    { return jank::make_box<obj::list>(); }

    return visit_object
    (
      [](auto const typed_s) -> obj::list_ptr
      {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          native_vector<object_ptr> v;
          for(auto i(typed_s->fresh_seq()); i != nullptr; i = i->next_in_place())
          { v.emplace_back(i->first()); }
          return jank::make_box<obj::list>(runtime::detail::native_persistent_list{ v.rbegin(), v.rend() });
        }
        else
        { throw std::runtime_error{ fmt::format("invalid sequence: {}", typed_s->to_string()) }; }
      },
      s
    );
  }

  native_bool obj::list::equal(object const &o) const
  { return detail::equal(o, data.begin(), data.end()); }

  void obj::list::to_string(fmt::memory_buffer &buff) const
  { return behavior::detail::to_string(data.begin(), data.end(), '(', ')', buff); }

  native_string obj::list::to_string() const
  {
    fmt::memory_buffer buff;
    behavior::detail::to_string(data.begin(), data.end(), '(', ')', buff);
    return native_string{ buff.data(), buff.size() };
  }
  /* TODO: Cache this. */
  native_integer obj::list::to_hash() const
  {
    auto seed(static_cast<native_integer>(data.size()));
    for(auto const &e : data)
    { seed = runtime::detail::hash_combine(seed, *e); }
    return seed;
  }

  obj::persistent_list_sequence_ptr obj::list::seq() const
  {
    if(data.size() == 0)
    { return nullptr; }
    return jank::make_box
    <
      obj::persistent_list_sequence
    >(this, data.begin(), data.end(), data.size());
  }
  obj::persistent_list_sequence_ptr obj::list::fresh_seq() const
  {
    if(data.size() == 0)
    { return nullptr; }
    return jank::make_box
    <
      obj::persistent_list_sequence
    >(this, data.begin(), data.end(), data.size());
  }
  size_t obj::list::count() const
  { return data.size(); }

  obj::list_ptr obj::list::cons(object_ptr head) const
  {
    auto l(data.cons(head));
    auto ret(jank::make_box<obj::list>(std::move(l)));
    return ret;
  }

  object_ptr obj::list::with_meta(object_ptr m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(jank::make_box<obj::list>(data));
    ret->meta = meta;
    return ret;
  }
}
