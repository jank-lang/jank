#include <iostream>
#include <sstream>

#include <jank/runtime/util.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/vector.hpp>

namespace jank::runtime
{
  obj::vector::static_object(runtime::detail::peristent_vector &&d)
    : data{ std::move(d) }
  { }

  obj::vector::static_object(runtime::detail::peristent_vector const &d)
    : data{ d }
  { }

  obj::vector_ptr obj::vector::create(object_ptr const s)
  {
    if(s == nullptr)
    { return jank::make_box<obj::vector>(); }

    return visit_object
    (
      s,
      [](auto const typed_s) -> obj::vector_ptr
      {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          runtime::detail::transient_vector v;
          for(auto i(typed_s->fresh_seq()); i != nullptr; i = i->next_in_place())
          { v.push_back(i->first()); }
          return jank::make_box<obj::vector>(v.persistent());
        }
        else
        { throw std::runtime_error{ fmt::format("invalid sequence: {}", typed_s->to_string()) }; }
      }
    );
  }

  native_bool obj::vector::equal(object const &o) const
  { return detail::equal(o, data.begin(), data.end()); }

  void obj::vector::to_string(fmt::memory_buffer &buff) const
  { return behavior::detail::to_string(data.begin(), data.end(), '[', ']', buff); }

  native_string obj::vector::to_string() const
  {
    fmt::memory_buffer buff;
    behavior::detail::to_string(data.begin(), data.end(), '[', ']', buff);
    return native_string{ buff.data(), buff.size() };
  }

  /* TODO: Cache this. */
  native_integer obj::vector::to_hash() const
  {
    auto seed(static_cast<native_integer>(data.size()));
    for(auto const &e : data)
    { seed = runtime::detail::hash_combine(seed, *e); }
    return seed;
  }

  obj::persistent_vector_sequence_ptr obj::vector::seq() const
  {
    if(data.empty())
    { return nullptr; }
    return jank::make_box<obj::persistent_vector_sequence>(const_cast<obj::vector*>(this));
  }

  obj::persistent_vector_sequence_ptr obj::vector::fresh_seq() const
  {
    if(data.empty())
    { return nullptr; }
    return jank::make_box<obj::persistent_vector_sequence>(const_cast<obj::vector*>(this));
  }

  size_t obj::vector::count() const
  { return data.size(); }

  obj::vector_ptr obj::vector::cons(object_ptr head) const
  {
    auto vec(data.push_back(head));
    auto ret(make_box<obj::vector>(std::move(vec)));
    return ret;
  }

  object_ptr obj::vector::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(jank::make_box<obj::vector>(data));
    ret->meta = meta;
    return ret;
  }

  object_ptr obj::vector::get(object_ptr const key) const
  {
    if(key->type == object_type::integer)
    {
      auto const i(static_cast<size_t>(expect_object<obj::integer>(key)->data));
      if(data.size() <= i)
      { return obj::nil::nil_const(); }
      return data[i];
    }
    else
    {
      throw std::runtime_error
      { fmt::format("get on a vector must be an integer; found {}", runtime::detail::to_string(key)) };
    }
  }

  object_ptr obj::vector::get(object_ptr const key, object_ptr const fallback) const
  {
    if(key->type == object_type::integer)
    {
      auto const i(static_cast<size_t>(expect_object<obj::integer>(key)->data));
      if(data.size() <= i)
      { return fallback; }
      return data[i];
    }
    else
    {
      throw std::runtime_error
      { fmt::format("get on a vector must be an integer; found {}", runtime::detail::to_string(key)) };
    }
  }
}
