#pragma once

#include <jank/obj-model/variant/object.hpp>

namespace jank::obj_model::variant
{
  using static_keyword = typed_object<object_type::keyword>;
  template <>
  struct typed_object<object_type::keyword> : gc
  {
    typed_object(runtime::obj::symbol &&sym, bool const resolved)
      : sym{ std::move(sym) }, resolved{ resolved }
    { }

    runtime::obj::symbol sym;
    /* Not resolved means this is a :: keyword. If ns is set, when this is true, it's an ns alias.
     * Upon interning, this will be resolved. */
    bool resolved{ true };
  };
}
