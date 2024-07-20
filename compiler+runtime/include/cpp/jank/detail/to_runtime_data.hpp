#pragma once

#include <boost/filesystem/path.hpp>

namespace jank::detail
{
  template <typename T>
  concept debuggable = requires(T const *t) { t->to_runtime_data(); };

  template <typename T>
  requires debuggable<T>
  runtime::object_ptr to_runtime_data(T const &d)
  {
    return d.to_runtime_data();
  }

  template <typename T>
  requires debuggable<T>
  runtime::object_ptr to_runtime_data(T const * const d)
  {
    return d->to_runtime_data();
  }

  template <typename T>
  runtime::object_ptr to_runtime_data(native_box<T> const &d)
  {
    return make_box(fmt::format("box({})", reinterpret_cast<void const *>(d.data)));
  }

  inline runtime::object_ptr to_runtime_data(native_persistent_string const &d)
  {
    return make_box(d);
  }

  inline runtime::object_ptr to_runtime_data(runtime::obj::symbol const &d)
  {
    return make_box<runtime::obj::symbol>(d);
  }

  inline runtime::object_ptr to_runtime_data(boost::filesystem::path const &p)
  {
    return make_box(p.string());
  }

  template <typename K, typename V, typename H, typename C>
  runtime::object_ptr to_runtime_data(native_unordered_map<K, V, H, C> const &m)
  {
    runtime::object_ptr ret(make_box<runtime::obj::persistent_array_map>());
    for(auto const &e : m)
    {
      ret = runtime::assoc(ret, to_runtime_data(e.first), to_runtime_data(e.second));
    }
    return ret;
  }

  template <typename T>
  runtime::object_ptr to_runtime_data(option<T> const &m)
  {
    return runtime::obj::persistent_array_map::create_unique(
      make_box("__type"),
      make_box("option"),
      make_box("data"),
      (m.is_none() ? make_box("none") : to_runtime_data(m.unwrap())));
  }

  template <typename T>
  requires runtime::behavior::object_like<T>
  runtime::object_ptr to_runtime_data(native_box<T> const &m)
  {
    return m;
  }

  template <typename T>
  requires runtime::behavior::object_like<T>
  runtime::object_ptr to_runtime_data(T const * const m)
  {
    return m;
  }
}
