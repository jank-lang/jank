#include <iostream>
#include <sstream>

#include <jank/runtime/util.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/set.hpp>

namespace jank::runtime
{
  obj::set::static_object(runtime::detail::native_persistent_set &&d)
    : data{ std::move(d) }
  { }

  obj::set::static_object(runtime::detail::native_persistent_set const &d)
    : data{ d }
  { }

  native_bool obj::set::equal(object const &o) const
  { return detail::equal(o, data.begin(), data.end()); }

  void obj::set::to_string(fmt::memory_buffer &buff) const
  { return behavior::detail::to_string(data.begin(), data.end(), "#{", '}', buff); }

  native_string obj::set::to_string() const
  {
    fmt::memory_buffer buff;
    behavior::detail::to_string(data.begin(), data.end(), "#{", '}', buff);
    return native_string{ buff.data(), buff.size() };
  }

  /* TODO: Cache this. */
  native_integer obj::set::to_hash() const
  {
    auto seed(static_cast<native_integer>(data.size()));
    for(auto const &e : data)
    { seed = runtime::detail::hash_combine(seed, *e); }
    return seed;
  }

  obj::persistent_set_sequence_ptr obj::set::seq() const
  { return fresh_seq(); }
  obj::persistent_set_sequence_ptr obj::set::fresh_seq() const
  {
    if(data.empty())
    { return nullptr; }
    return make_box<obj::persistent_set_sequence>(this, data.begin(), data.end(), data.size());
  }

  size_t obj::set::count() const
  { return data.size(); }

  object_ptr obj::set::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<obj::set>(data));
    ret->meta = meta;
    return ret;
  }

  obj::set_ptr obj::set::cons(object_ptr const head) const
  {
    auto vec(data.insert(head));
    auto ret(make_box<obj::set>(std::move(vec)));
    return ret;
  }

  native_bool obj::set::contains(object_ptr const o) const
  { return data.find(o); }
}
