#pragma once

#include <jank/obj-model/tagged/object.hpp>

namespace jank::obj_model::tagged
{
  using static_keyword = typed_object<object_type::keyword>;
  template <>
  struct typed_object<object_type::keyword> : gc
  {
    static auto create(runtime::obj::symbol &&sym, bool const resolved)
    { return new (PointerFreeGC) static_keyword{ {}, { object_type::keyword }, { std::move(sym) }, resolved }; }

    object base{ object_type::keyword };
    runtime::obj::symbol sym;
    /* Not resolved means this is a :: keyword. If ns is set, when this is true, it's an ns alias.
     * Upon interning, this will be resolved. */
    bool resolved{ true };
  };
}
