#pragma once

#include <jank/obj-model/tagged/object.hpp>
#include <jank/obj-model/tagged/map_type.hpp>

namespace jank::obj_model::tagged
{
  using static_map = typed_object<object_type::map>;
  template <>
  struct typed_object<object_type::map> : gc
  {
    static auto create()
    { return new (GC) static_map{ }; }
    static auto create(object_ptr *arr, size_t const n)
    { return new (GC) static_map{ {}, { object_type::map }, { in_place_unique{}, arr, n }, {} }; }
    template <typename T, typename ...Args>
    requires (!std::same_as<std::decay_t<T>, object_ptr*>)
    static auto create(T &&t, Args &&...args)
    { return new (GC) static_map{ {}, { object_type::map }, { in_place_unique{}, jank::make_array_box<object_ptr>(erase_type(std::forward<T>(t)), erase_type(std::forward<Args>(args))...), 1 + sizeof...(Args) }, {} }; }

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

    object base{ object_type::map };
    map_type_impl<object_ptr, object_ptr> data{};
    object_ptr meta{};
  };
}
