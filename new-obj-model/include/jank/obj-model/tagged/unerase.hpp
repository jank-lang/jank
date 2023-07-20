#pragma once

#include <jank/obj-model/tagged/object.hpp>
#include <jank/obj-model/tagged/map.hpp>
#include <jank/obj-model/tagged/keyword.hpp>

namespace jank::obj_model::tagged
{
  /* TODO: Rename to visit. */
  template <typename R, typename F, typename ...Args>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline constexpr R unerase_type(object *const erased, F &&fn, Args ...args)
  {
    switch(erased->type)
    {
      case jank::obj_model::tagged::object_type::nil:
      {
        return fn(reinterpret_cast<static_nil*>(erased), args...);
      } break;
      case jank::obj_model::tagged::object_type::keyword:
      {
        return fn(reinterpret_cast<static_keyword*>(erased), args...);
      } break;
      case jank::obj_model::tagged::object_type::map:
      {
        return fn(reinterpret_cast<static_map*>(erased), args...);
      } break;
      default:
      {
        throw std::runtime_error{ "invalid erased object type" };
      } break;
    }
  }
}
