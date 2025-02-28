#pragma once

#include <boost/filesystem/path.hpp>

#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/seq.hpp>

namespace jank::detail
{
  using namespace jank::runtime;

  template <typename T>
  concept debuggable = requires(T const *t) { t->to_runtime_data(); };

  template <typename T>
  requires debuggable<T>
  object_ptr to_runtime_data(T const &d)
  {
    return d.to_runtime_data();
  }

  template <typename T>
  requires debuggable<T>
  object_ptr to_runtime_data(T const * const d)
  {
    return d->to_runtime_data();
  }

  template <typename T>
  object_ptr to_runtime_data(native_box<T> const &d)
  {
    util::string_builder sb;
    return make_box(sb("box(")(reinterpret_cast<void const *>(d.data))(")").release());
  }

  inline object_ptr to_runtime_data(native_persistent_string const &d)
  {
    return make_box(d);
  }

  inline object_ptr to_runtime_data(obj::symbol const &d)
  {
    return make_box<obj::symbol>(d);
  }

  inline object_ptr to_runtime_data(boost::filesystem::path const &p)
  {
    return make_box(p.string());
  }

  template <typename K, typename V, typename H, typename C>
  object_ptr to_runtime_data(native_unordered_map<K, V, H, C> const &m)
  {
    object_ptr ret(make_box<obj::persistent_array_map>());
    for(auto const &e : m)
    {
      ret = assoc(ret, to_runtime_data(e.first), to_runtime_data(e.second));
    }
    return ret;
  }

  template <typename T>
  object_ptr to_runtime_data(option<T> const &m)
  {
    return obj::persistent_array_map::create_unique(
      make_box("__type"),
      make_box("option"),
      make_box("data"),
      (m.is_none() ? make_box("none") : to_runtime_data(m.unwrap())));
  }

  template <typename T>
  requires behavior::object_like<T>
  object_ptr to_runtime_data(native_box<T> const &m)
  {
    return m;
  }

  template <typename T>
  requires behavior::object_like<T>
  object_ptr to_runtime_data(T const * const m)
  {
    return m;
  }
}
