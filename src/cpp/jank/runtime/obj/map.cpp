#include <iostream>
#include <sstream>

#include <jank/runtime/util.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/map.hpp>
#include <jank/runtime/obj/vector.hpp>

namespace jank::runtime
{
  obj::map::static_object(native_box<obj::map> const meta)
    : meta{ meta }
  { }

  obj::map::static_object(runtime::detail::persistent_map &&d)
    : data{ std::move(d) }
  { }

  obj::map::static_object(runtime::detail::persistent_map const &d)
    : data{ d }
  { }

  native_bool obj::map::equal(object const &o) const
  {
    if(o.type != object_type::map)
    { return false; }

    auto const m(expect_object<obj::map>(&o));
    return to_hash() == m->to_hash();
  }

  void to_string_impl
  (
    runtime::detail::persistent_map::const_iterator const &begin,
    runtime::detail::persistent_map::const_iterator const &end,
    fmt::memory_buffer &buff
  )
  {
    auto inserter(std::back_inserter(buff));
    inserter = '{';
    for(auto i(begin); i != end; ++i)
    {
      auto const pair(*i);
      runtime::detail::to_string(pair.first, buff);
      inserter = ' ';
      runtime::detail::to_string(pair.second, buff);
      auto n(i);
      if(++n != end)
      {
        inserter = ',';
        inserter = ' ';
      }
    }
    inserter = '}';
  }
  void obj::map::to_string(fmt::memory_buffer &buff) const
  { to_string_impl(data.begin(), data.end(), buff); }
  native_string obj::map::to_string() const
  {
    fmt::memory_buffer buff;
    to_string_impl(data.begin(), data.end(), buff);
    return native_string{ buff.data(), buff.size() };
  }
  /* TODO: Cache this. */
  native_integer obj::map::to_hash() const
  {
    auto seed(static_cast<native_integer>(data.size()));
    for(auto const &e : data)
    {
      seed = runtime::detail::hash_combine(seed, *e.first);
      seed = runtime::detail::hash_combine(seed, *e.second);
    }
    return seed;
  }

  obj::persistent_map_sequence_ptr obj::map::seq() const
  {
    if(data.size() == 0)
    { return nullptr; }
    return jank::make_box<obj::persistent_map_sequence>(this, data.begin(), data.end());
  }

  obj::persistent_map_sequence_ptr obj::map::fresh_seq() const
  {
    if(data.size() == 0)
    { return nullptr; }
    return jank::make_box<obj::persistent_map_sequence>(this, data.begin(), data.end());
  }

  size_t obj::map::count() const
  { return data.size(); }

  object_ptr obj::map::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<obj::map>(data));
    ret->meta = meta;
    return ret;
  }

  object_ptr obj::map::get(object_ptr const key) const
  {
    auto const res(data.find(key));
    if(res)
    { return res; }
    return obj::nil::nil_const();
  }
  object_ptr obj::map::get(object_ptr const key, object_ptr const fallback) const
  {
    auto const res(data.find(key));
    if(res)
    { return res; }
    return fallback;
  }

  object_ptr obj::map::assoc(object_ptr const key, object_ptr const val) const
  {
    auto copy(data.clone());
    copy.insert_or_assign(key, val);
    return make_box<obj::map>(std::move(copy));
  }
}
