#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/callable.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using persistent_hash_map = static_object<object_type::persistent_hash_map>;
    using persistent_hash_map_ptr = native_box<persistent_hash_map>;
  }

  template <>
  struct static_object<object_type::multi_function>
    : gc
    , behavior::callable
  {
    static constexpr native_bool pointer_free{ false };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(object_ptr name, object_ptr dispatch, object_ptr default_, object_ptr hierarchy);

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_persistent_string to_string();
    void to_string(fmt::memory_buffer &buff);
    native_hash to_hash() const;

    /* behavior::callable */
    object_ptr call() const override;
    object_ptr call(object_ptr) const override;
    object_ptr call(object_ptr, object_ptr) const override;
    object_ptr call(object_ptr, object_ptr, object_ptr) const override;
    object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr) const override;
    object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const override;
    object_ptr
      call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const override;
    object_ptr
      call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr)
        const override;
    object_ptr call(object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr) const override;
    object_ptr call(object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr) const override;
    object_ptr call(object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr) const override;
    object_ptr this_object_ptr() const final;

    object base{ object_type::multi_function };
    behavior::callable_ptr dispatch{};
    object_ptr default_dispatch_value{};
    object_ptr hierarchy{};
    object_ptr cached_hierarchy{};
    obj::persistent_hash_map_ptr method_table{};
    obj::persistent_hash_map_ptr prefer_table{};
    obj::persistent_hash_map_ptr method_cache{};
    obj::symbol_ptr name;
  };

  namespace obj
  {
    using multi_function = static_object<object_type::multi_function>;
    using multi_function_ptr = native_box<multi_function>;
  }
}
