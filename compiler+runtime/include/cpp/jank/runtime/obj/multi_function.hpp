#pragma once

#include <mutex>

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/callable.hpp>

namespace jank::runtime::obj
{
  using symbol_ptr = native_box<struct symbol>;
  using persistent_hash_map_ptr = native_box<struct persistent_hash_map>;
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
    jtl::immutable_string to_string();
    void to_string(util::string_builder &buff);
    jtl::immutable_string to_code_string();
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
    persistent_hash_map_ptr reset_cache();
    multi_function_ptr add_method(object_ptr dispatch_val, object_ptr method);
    multi_function_ptr remove_method(object_ptr dispatch_val);
    multi_function_ptr prefer_method(object_ptr x, object_ptr y);
    native_bool is_preferred(object_ptr hierarchy, object_ptr x, object_ptr y) const;

    static native_bool is_a(object_ptr hierarchy, object_ptr x, object_ptr y);
    native_bool is_dominant(object_ptr hierarchy, object_ptr x, object_ptr y) const;

    object_ptr get_fn(object_ptr dispatch_val);
    object_ptr get_method(object_ptr dispatch_val);
    object_ptr find_and_cache_best_method(object_ptr dispatch_val);

    object base{ obj_type };
    object_ptr dispatch{};
    object_ptr default_dispatch_value{};
    object_ptr hierarchy{};
    mutable object_ptr cached_hierarchy{};
    persistent_hash_map_ptr method_table{};
    mutable persistent_hash_map_ptr method_cache{};
    persistent_hash_map_ptr prefer_table{};
    symbol_ptr name{};
    std::recursive_mutex data_lock;
  };
}
