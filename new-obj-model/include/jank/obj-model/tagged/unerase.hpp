#pragma once

#include <jank/obj-model/tagged/object.hpp>
#include <jank/obj-model/tagged/map.hpp>
#include <jank/obj-model/tagged/keyword.hpp>

namespace jank::obj_model::tagged
{
  template <typename F>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline void unerase_type(object *const erased, F &&fn)
  {
    switch(erased->type)
    {
      case jank::obj_model::tagged::object_type::nil:
      {
        fn(reinterpret_cast<static_nil*>(erased));
      } break;
      case jank::obj_model::tagged::object_type::keyword:
      {
        fn(reinterpret_cast<static_keyword*>(erased));
      } break;
      case jank::obj_model::tagged::object_type::map:
      {
        fn(reinterpret_cast<static_map*>(erased));
      } break;
      default:
      {
        throw std::runtime_error{ "invalid erased object type" };
      } break;
    }
  }
}
