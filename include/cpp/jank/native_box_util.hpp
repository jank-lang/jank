#pragma once

#include <jank/runtime/object.hpp>

namespace jank
{
  namespace runtime::obj
  {
    using nil = static_object<object_type::nil>;
    using boolean = static_object<object_type::boolean>;
    using integer = static_object<object_type::integer>;
    using real = static_object<object_type::real>;
    using string = static_object<object_type::persistent_string>;
    using list = static_object<object_type::persistent_list>;
  }

  native_box<runtime::obj::nil> make_box(std::nullptr_t const &);
  native_box<runtime::obj::boolean> make_box(native_bool const b);
  native_box<runtime::obj::integer> make_box(int const i);
  native_box<runtime::obj::integer> make_box(native_integer const i);
  native_box<runtime::obj::integer> make_box(size_t const i);
  native_box<runtime::obj::real> make_box(native_real const r);
  native_box<runtime::obj::string> make_box(native_persistent_string_view const &s);
}
