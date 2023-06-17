#pragma once

#include <jank/obj-model/bitfield/object.hpp>

namespace jank::obj_model::bitfield
{
  using static_keyword = typed_object<behavior_type_keyword, storage_type_keyword>;
  template <>
  struct typed_object<behavior_type_keyword, storage_type_keyword> : gc
  {
    static auto create(runtime::obj::symbol &&sym, bool const resolved)
    { return new (PointerFreeGC) static_keyword{ {}, { behavior_type_keyword, storage_type_keyword }, { std::move(sym) }, resolved }; }

    object base{ behavior_type_keyword, storage_type_keyword };
    runtime::obj::symbol sym;
    /* Not resolved means this is a :: keyword. If ns is set, when this is true, it's an ns alias.
     * Upon interning, this will be resolved. */
    bool resolved{ true };
  };
}
