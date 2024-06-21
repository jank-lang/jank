#include <jank/runtime/util.hpp>
#include <jank/runtime/obj/transient_hash_map.hpp>

namespace jank::runtime
{
  obj::transient_hash_map::static_object(runtime::detail::native_persistent_hash_map &&d)
    : data{ std::move(d).transient() }
  {
  }

  obj::transient_hash_map::static_object(runtime::detail::native_persistent_hash_map const &d)
    : data{ d.transient() }
  {
  }

  obj::transient_hash_map::static_object(runtime::detail::native_transient_hash_map &&d)
    : data{ std::move(d) }
  {
  }

  native_bool obj::transient_hash_map::equal(object const &o) const
  {
    /* Transient equality, in Clojure, is based solely on identity. */
    return &base == &o;
  }

  void obj::transient_hash_map::to_string(fmt::memory_buffer &buff) const
  {
    auto inserter(std::back_inserter(buff));
    fmt::format_to(inserter, "{}@{}", magic_enum::enum_name(base.type), fmt::ptr(&base));
  }

  native_persistent_string obj::transient_hash_map::to_string() const
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  native_hash obj::transient_hash_map::to_hash() const
  {
    /* Hash is also based only on identity. Clojure uses default hashCode, which does the same. */
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  size_t obj::transient_hash_map::count() const
  {
    assert_active();
    return data.size();
  }

  object_ptr obj::transient_hash_map::get(object_ptr const key) const
  {
    assert_active();
    auto const res(data.find(key));
    if(res)
    {
      return *res;
    }
    return obj::nil::nil_const();
  }

  object_ptr obj::transient_hash_map::get(object_ptr const key, object_ptr const fallback) const
  {
    assert_active();
    auto const res(data.find(key));
    if(res)
    {
      return *res;
    }
    return fallback;
  }

  object_ptr obj::transient_hash_map::get_entry(object_ptr const key) const
  {
    assert_active();
    auto const res(data.find(key));
    if(res)
    {
      return make_box<obj::persistent_vector>(std::in_place, key, *res);
    }
    return obj::nil::nil_const();
  }

  native_bool obj::transient_hash_map::contains(object_ptr const key) const
  {
    assert_active();
    return data.find(key);
  }

  obj::transient_hash_map_ptr
  obj::transient_hash_map::assoc_in_place(object_ptr const key, object_ptr const val)
  {
    assert_active();
    data.set(key, val);
    return this;
  }

  obj::transient_hash_map_ptr obj::transient_hash_map::dissoc_in_place(object_ptr const key)
  {
    assert_active();
    data.erase(key);
    return this;
  }

  obj::transient_hash_map_ptr obj::transient_hash_map::cons_in_place(object_ptr const head)
  {
    assert_active();

    if(head->type == object_type::persistent_array_map
       || head->type == object_type::persistent_hash_map)
    {
      return expect_object<obj::transient_hash_map>(runtime::merge(this, head));
    }

    if(head->type != object_type::persistent_vector)
    {
      throw std::runtime_error{ fmt::format("invalid map entry: {}",
                                            runtime::detail::to_string(head)) };
    }

    auto const vec(expect_object<obj::persistent_vector>(head));
    if(vec->count() != 2)
    {
      throw std::runtime_error{ fmt::format("invalid map entry: {}",
                                            runtime::detail::to_string(head)) };
    }

    data.set(vec->data[0], vec->data[1]);
    return this;
  }

  native_box<obj::transient_hash_map::persistent_type> obj::transient_hash_map::to_persistent()
  {
    assert_active();
    active = false;
    return make_box<obj::persistent_hash_map>(std::move(data).persistent());
  }

  object_ptr obj::transient_hash_map::call(object_ptr const o)
  {
    assert_active();
    auto const found(data.find(o));
    if(!found)
    {
      return obj::nil::nil_const();
    }
    return *found;
  }

  object_ptr obj::transient_hash_map::call(object_ptr const o, object_ptr const fallback)
  {
    assert_active();
    auto const found(data.find(o));
    if(!found)
    {
      return fallback;
    }
    return *found;
  }

  void obj::transient_hash_map::assert_active() const
  {
    if(!active)
    {
      throw std::runtime_error{ "transient used after it's been made persistent" };
    }
  }
}
