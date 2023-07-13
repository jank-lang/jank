#pragma once

#include <jank/obj-model/variant/object.hpp>
#include <jank/obj-model/variant/map_type.hpp>

namespace jank::obj_model::variant
{
  using static_map = typed_object<object_type::map>;
  template <>
  struct typed_object<object_type::map>
  {
    typed_object() = default;
    template <typename ...Args>
    typed_object(Args &&...args)
      : data{ in_place_unique{}, jank::make_array_box<object_ptr>(erase_type(std::forward<Args>(args))...), sizeof...(Args) }
    { }

    object_ptr get(object_ptr key) const
    {
      auto const res(data.find(key));
      if(res)
      { return res; }
      return nullptr;
    }
    object_ptr get(object_ptr key, object_ptr fallback) const
    {
      auto const res(data.find(key));
      if(res)
      { return res; }
      return fallback;
    }
    object_ptr assoc(object_ptr key, object_ptr val) const
    { return nullptr; }

    size_t count() const
    { return data.size(); }

    map_type_impl<object_ptr, object_ptr> data{};
    object_ptr meta{};
  };
}
