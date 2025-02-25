#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/runtime/obj/transient_sorted_map.hpp>
#include <jank/runtime/obj/persistent_sorted_map.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/rtti.hpp>

namespace jank::runtime::obj
{
  transient_sorted_map::transient_sorted_map(runtime::detail::native_persistent_sorted_map &&d)
    : data{ std::move(d).transient() }
  {
  }

  transient_sorted_map::transient_sorted_map(runtime::detail::native_persistent_sorted_map const &d)
    : data{ d.transient() }
  {
  }

  transient_sorted_map::transient_sorted_map(runtime::detail::native_transient_sorted_map &&d)
    : data{ std::move(d) }
  {
  }

  transient_sorted_map_ptr transient_sorted_map::empty()
  {
    return make_box<transient_sorted_map>();
  }

  native_bool transient_sorted_map::equal(object const &o) const
  {
    /* Transient equality, in Clojure, is based solely on identity. */
    return &base == &o;
  }

  void transient_sorted_map::to_string(util::string_builder &buff) const
  {
    auto inserter(std::back_inserter(buff));
    fmt::format_to(inserter, "{}@{}", object_type_str(base.type), fmt::ptr(&base));
  }

  native_persistent_string transient_sorted_map::to_string() const
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  native_persistent_string transient_sorted_map::to_code_string() const
  {
    return to_string();
  }

  native_hash transient_sorted_map::to_hash() const
  {
    /* Hash is also based only on identity. Clojure uses default hashCode, which does the same. */
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  size_t transient_sorted_map::count() const
  {
    assert_active();
    return data.size();
  }

  object_ptr transient_sorted_map::get(object_ptr const key) const
  {
    assert_active();
    auto const res(data.find(key));
    if(res != data.end())
    {
      return res->second;
    }
    return nil::nil_const();
  }

  object_ptr transient_sorted_map::get(object_ptr const key, object_ptr const fallback) const
  {
    assert_active();
    auto const res(data.find(key));
    if(res != data.end())
    {
      return res->second;
    }
    return fallback;
  }

  object_ptr transient_sorted_map::get_entry(object_ptr const key) const
  {
    assert_active();
    auto const res(data.find(key));
    if(res != data.end())
    {
      return make_box<persistent_vector>(std::in_place, key, res->second);
    }
    return nil::nil_const();
  }

  native_bool transient_sorted_map::contains(object_ptr const key) const
  {
    assert_active();
    return data.find(key) != data.end();
  }

  transient_sorted_map_ptr
  transient_sorted_map::assoc_in_place(object_ptr const key, object_ptr const val)
  {
    assert_active();
    data.insert_or_assign(key, val);
    return this;
  }

  transient_sorted_map_ptr transient_sorted_map::dissoc_in_place(object_ptr const key)
  {
    assert_active();
    data.erase_key(key);
    return this;
  }

  transient_sorted_map_ptr transient_sorted_map::conj_in_place(object_ptr const head)
  {
    assert_active();

    if(head == obj::nil::nil_const())
    {
      return this;
    }

    if(is_map(head))
    {
      return expect_object<transient_sorted_map>(runtime::merge_in_place(this, head));
    }

    if(head->type != object_type::persistent_vector)
    {
      throw std::runtime_error{ fmt::format("invalid map entry: {}", runtime::to_string(head)) };
    }

    auto const vec(expect_object<persistent_vector>(head));
    if(vec->count() != 2)
    {
      throw std::runtime_error{ fmt::format("invalid map entry: {}", runtime::to_string(head)) };
    }

    data.insert_or_assign(vec->data[0], vec->data[1]);
    return this;
  }

  transient_sorted_map::persistent_type_ptr transient_sorted_map::to_persistent()
  {
    assert_active();
    active = false;
    return make_box<persistent_sorted_map>(std::move(data).persistent());
  }

  object_ptr transient_sorted_map::call(object_ptr const o) const
  {
    return get(o);
  }

  object_ptr transient_sorted_map::call(object_ptr const o, object_ptr const fallback) const
  {
    return get(o, fallback);
  }

  void transient_sorted_map::assert_active() const
  {
    if(!active)
    {
      throw std::runtime_error{ "transient used after it's been made persistent" };
    }
  }
}
