#pragma once

#include <jank/runtime/seq.hpp>

namespace jank::detail
{
  template <typename T>
  concept debuggable = requires(T const * t){ t->to_runtime_data(); };

  template <typename T>
  requires debuggable<T>
  runtime::object_ptr to_runtime_data(T const &d)
  { return d.to_runtime_data(); }

  template <typename T>
  requires debuggable<T>
  runtime::object_ptr to_runtime_data(T const * const d)
  { return d->to_runtime_data(); }

  template <typename T>
  runtime::object_ptr to_runtime_data(native_box<T> const &d)
  { return make_box(fmt::format("box({})", reinterpret_cast<void const*>(d.data))); }

  inline runtime::object_ptr to_runtime_data(native_string const &d)
  { return make_box(d); }
  inline runtime::object_ptr to_runtime_data(runtime::obj::symbol const &d)
  { return make_box<runtime::obj::symbol>(d); }

  template <typename K, typename V>
  runtime::object_ptr to_runtime_data(native_unordered_map<K, V> const &m)
  {
    runtime::object_ptr ret(make_box<runtime::obj::map>());
    for(auto const &e : m)
    { ret = runtime::assoc(ret, to_runtime_data(e.first), to_runtime_data(e.second)); }
    return ret;
  }

  template <typename T>
  runtime::object_ptr to_runtime_data(option<T> const &m)
  {
    return runtime::obj::map::create_unique
    (
      make_box("__type"), make_box("option"),
      make_box("data"), (m.is_none() ? make_box("none") : to_runtime_data(m.unwrap()))
    );
  }

  template <typename T>
  requires runtime::behavior::objectable<T>
  runtime::object_ptr to_runtime_data(native_box<T> const &m)
  { return m; }

  template <typename T>
  requires runtime::behavior::objectable<T>
  runtime::object_ptr to_runtime_data(T const * const m)
  { return m; }
}
