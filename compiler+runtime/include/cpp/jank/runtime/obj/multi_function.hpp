#pragma once

#include <mutex>

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/behavior/callable.hpp>

namespace jank::runtime::obj
{
  using multi_function_ptr = native_box<struct multi_function>;

  struct multi_function
    : gc
    , behavior::callable
  {
    static constexpr object_type obj_type{ object_type::multi_function };
    static constexpr native_bool pointer_free{ false };

    multi_function() = default;
    multi_function(object_ptr name, object_ptr dispatch, object_ptr default_, object_ptr hierarchy);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string();
    void to_string(fmt::memory_buffer &buff);
    native_persistent_string to_code_string();
    native_hash to_hash() const;

    /* behavior::callable */
    object_ptr call() override;
    object_ptr call(object_ptr) override;
    object_ptr call(object_ptr, object_ptr) override;
    object_ptr call(object_ptr, object_ptr, object_ptr) override;
    object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr) override;
    object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) override;
    object_ptr
      call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) override;
    object_ptr
      call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr)
        override;
    object_ptr call(object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr) override;
    object_ptr call(object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr) override;
    object_ptr call(object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr) override;
    object_ptr this_object_ptr() final;

    multi_function_ptr reset();
    obj::persistent_hash_map_ptr reset_cache();
    multi_function_ptr add_method(object_ptr dispatch_val, object_ptr method);
    multi_function_ptr remove_method(object_ptr dispatch_val);
    multi_function_ptr prefer_method(object_ptr x, object_ptr y);
    native_bool is_preferred(object_ptr hierarchy, object_ptr x, object_ptr y) const;

    static native_bool is_a(object_ptr hierarchy, object_ptr x, object_ptr y);
    native_bool is_dominant(object_ptr hierarchy, object_ptr x, object_ptr y) const;

    object_ptr get_fn(object_ptr dispatch_val);
    object_ptr get_method(object_ptr dispatch_val);
    object_ptr find_and_cache_best_method(object_ptr dispatch_val);

    object base{ object_type::multi_function };
    object_ptr dispatch{};
    object_ptr default_dispatch_value{};
    object_ptr hierarchy{};
    mutable object_ptr cached_hierarchy{};
    obj::persistent_hash_map_ptr method_table{};
    mutable obj::persistent_hash_map_ptr method_cache{};
    obj::persistent_hash_map_ptr prefer_table{};
    obj::symbol_ptr name{};
    std::recursive_mutex data_lock;
  };
}
