#pragma once

#include <jank/obj-model/bitfield/object.hpp>
#include <jank/obj-model/bitfield/map.hpp>
#include <jank/obj-model/bitfield/keyword.hpp>

namespace jank::obj_model::bitfield
{
  template <typename F>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline void unerase_type_fn(object *const erased, F &&fn)
  {
    switch(erased->behavior)
    {
      case jank::obj_model::bitfield::behavior_type::behavior_type_nil:
      {
        fn(reinterpret_cast<static_nil*>(erased));
      } break;
      case jank::obj_model::bitfield::behavior_type::behavior_type_keyword:
      {
        fn(reinterpret_cast<static_keyword*>(erased));
      } break;
      case jank::obj_model::bitfield::behavior_type::behavior_type_map:
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

#define unerase_type(erased, fn) \
{ \
  auto const jank__erased((erased)); \
  auto const jank__gen_fn((fn)); \
  switch(jank__erased->behavior) \
  { \
    case jank::obj_model::bitfield::behavior_type::behavior_type_nil: \
    { \
      jank__gen_fn(reinterpret_cast<jank::obj_model::bitfield::static_nil*>(jank__erased)); \
    } break; \
    case jank::obj_model::bitfield::behavior_type::behavior_type_keyword: \
    { \
      jank__gen_fn(reinterpret_cast<jank::obj_model::bitfield::static_keyword*>(jank__erased)); \
    } break; \
    case jank::obj_model::bitfield::behavior_type::behavior_type_map: \
    { \
      jank__gen_fn(reinterpret_cast<jank::obj_model::bitfield::static_map*>(jank__erased)); \
    } break; \
    default: \
    { \
      throw std::runtime_error{ "invalid erased object type" }; \
    } break; \
  } \
}
