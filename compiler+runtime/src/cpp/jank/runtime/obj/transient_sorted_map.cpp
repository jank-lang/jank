#include <jank/runtime/obj/transient_sorted_map.hpp>

namespace jank::runtime
{
  obj::transient_sorted_map::static_object(runtime::detail::native_persistent_sorted_map &&d)
    : data{ std::move(d).transient() }
  {
  }

  obj::transient_sorted_map::static_object(runtime::detail::native_persistent_sorted_map const &d)
    : data{ d.transient() }
  {
  }

  obj::transient_sorted_map::static_object(runtime::detail::native_transient_sorted_map &&d)
    : data{ std::move(d) }
  {
  }

  native_bool obj::transient_sorted_map::equal(object const &o) const
  {
    /* Transient equality, in Clojure, is based solely on identity. */
    return &base == &o;
  }

  void obj::transient_sorted_map::to_string(fmt::memory_buffer &buff) const
  {
    auto inserter(std::back_inserter(buff));
    fmt::format_to(inserter, "{}@{}", magic_enum::enum_name(base.type), fmt::ptr(&base));
  }

  native_persistent_string obj::transient_sorted_map::to_string() const
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  native_persistent_string obj::transient_sorted_map::to_code_string() const
  {
    return to_string();
  }

  native_hash obj::transient_sorted_map::to_hash() const
  {
    /* Hash is also based only on identity. Clojure uses default hashCode, which does the same. */
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  size_t obj::transient_sorted_map::count() const
  {
    assert_active();
    return data.size();
  }

  object_ptr obj::transient_sorted_map::get(object_ptr const key) const
  {
    assert_active();
    auto const res(data.find(key));
    if(res != data.end())
    {
      return res->second;
    }
    return obj::nil::nil_const();
  }

  object_ptr obj::transient_sorted_map::get(object_ptr const key, object_ptr const fallback) const
  {
    assert_active();
    auto const res(data.find(key));
    if(res != data.end())
    {
      return res->second;
    }
    return fallback;
  }

  object_ptr obj::transient_sorted_map::get_entry(object_ptr const key) const
  {
    assert_active();
    auto const res(data.find(key));
    if(res != data.end())
    {
      return make_box<obj::persistent_vector>(std::in_place, key, res->second);
    }
    return obj::nil::nil_const();
  }

  native_bool obj::transient_sorted_map::contains(object_ptr const key) const
  {
    assert_active();
    return data.find(key) != data.end();
  }

  obj::transient_sorted_map_ptr
  obj::transient_sorted_map::assoc_in_place(object_ptr const key, object_ptr const val)
  {
    assert_active();
    data.insert_or_assign(key, val);
    return this;
  }

  obj::transient_sorted_map_ptr obj::transient_sorted_map::dissoc_in_place(object_ptr const key)
  {
    assert_active();
    data.erase_key(key);
    return this;
  }

  obj::transient_sorted_map_ptr obj::transient_sorted_map::conj_in_place(object_ptr const head)
  {
    assert_active();

    if(head->type == object_type::persistent_array_map
       || head->type == object_type::persistent_sorted_map)
    {
      return expect_object<obj::transient_sorted_map>(runtime::merge(this, head));
    }

    if(head->type != object_type::persistent_vector)
    {
      throw std::runtime_error{ fmt::format("invalid map entry: {}", runtime::to_string(head)) };
    }

    auto const vec(expect_object<obj::persistent_vector>(head));
    if(vec->count() != 2)
    {
      throw std::runtime_error{ fmt::format("invalid map entry: {}", runtime::to_string(head)) };
    }

    data.insert_or_assign(vec->data[0], vec->data[1]);
    return this;
  }

  native_box<obj::transient_sorted_map::persistent_type> obj::transient_sorted_map::to_persistent()
  {
    assert_active();
    active = false;
    return make_box<obj::persistent_sorted_map>(std::move(data).persistent());
  }

  object_ptr obj::transient_sorted_map::call(object_ptr const o)
  {
    return get(o);
  }

  object_ptr obj::transient_sorted_map::call(object_ptr const o, object_ptr const fallback)
  {
    return get(o, fallback);
  }

  void obj::transient_sorted_map::assert_active() const
  {
    if(!active)
    {
      throw std::runtime_error{ "transient used after it's been made persistent" };
    }
  }
}
