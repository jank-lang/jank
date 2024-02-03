#include <jank/runtime/util.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/persistent_set.hpp>

namespace jank::runtime
{
  obj::persistent_set::static_object(runtime::detail::native_persistent_set &&d)
    : data{ std::move(d) }
  {
  }

  obj::persistent_set::static_object(runtime::detail::native_persistent_set const &d)
    : data{ d }
  {
  }

  obj::persistent_set::static_object(runtime::detail::native_transient_set &&d)
    : data{ d.persistent() }
  {
  }

  native_bool obj::persistent_set::equal(object const &o) const
  {
    return detail::equal(o, data.begin(), data.end());
  }

  void obj::persistent_set::to_string(fmt::memory_buffer &buff) const
  {
    return behavior::detail::to_string(data.begin(), data.end(), "#{", '}', buff);
  }

  native_persistent_string obj::persistent_set::to_string() const
  {
    fmt::memory_buffer buff;
    behavior::detail::to_string(data.begin(), data.end(), "#{", '}', buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  /* TODO: Cache this. */
  native_hash obj::persistent_set::to_hash() const
  {
    return hash::unordered(data.begin(), data.end());
  }

  obj::persistent_set_sequence_ptr obj::persistent_set::seq() const
  {
    return fresh_seq();
  }

  obj::persistent_set_sequence_ptr obj::persistent_set::fresh_seq() const
  {
    if(data.empty())
    {
      return nullptr;
    }
    return make_box<obj::persistent_set_sequence>(this, data.begin(), data.end(), data.size());
  }

  size_t obj::persistent_set::count() const
  {
    return data.size();
  }

  object_ptr obj::persistent_set::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<obj::persistent_set>(data));
    ret->meta = meta;
    return ret;
  }

  obj::persistent_set_ptr obj::persistent_set::cons(object_ptr const head) const
  {
    auto vec(data.insert(head));
    auto ret(make_box<obj::persistent_set>(std::move(vec)));
    return ret;
  }

  object_ptr obj::persistent_set::call(object_ptr const o) const
  {
    auto const found(data.find(o));
    if(!found)
    {
      return obj::nil::nil_const();
    }
    return *found;
  }

  native_bool obj::persistent_set::contains(object_ptr const o) const
  {
    return data.find(o);
  }
}
