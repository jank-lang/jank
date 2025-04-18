#pragma once

#include <mutex>

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/callable.hpp>

namespace jank::runtime::obj
{
  using symbol_ref = oref<struct symbol>;
  using persistent_hash_map_ref = oref<struct persistent_hash_map>;
  using multi_function_ref = oref<struct multi_function>;

  struct multi_function
    : gc
    , behavior::callable
  {
    static constexpr object_type obj_type{ object_type::multi_function };
    static constexpr bool pointer_free{ false };

    multi_function() = delete;
    multi_function(object_ref name, object_ref dispatch, object_ref default_, object_ref hierarchy);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string();
    void to_string(util::string_builder &buff);
    jtl::immutable_string to_code_string();
    uhash to_hash() const;

    /* behavior::callable */
    object_ref call() override;
    object_ref call(object_ref) override;
    object_ref call(object_ref, object_ref) override;
    object_ref call(object_ref, object_ref, object_ref) override;
    object_ref call(object_ref, object_ref, object_ref, object_ref) override;
    object_ref call(object_ref, object_ref, object_ref, object_ref, object_ref) override;
    object_ref
      call(object_ref, object_ref, object_ref, object_ref, object_ref, object_ref) override;
    object_ref
      call(object_ref, object_ref, object_ref, object_ref, object_ref, object_ref, object_ref)
        override;
    object_ref call(object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref) override;
    object_ref call(object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref) override;
    object_ref call(object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref) override;
    object_ref this_object_ref() final;

    multi_function_ref reset();
    persistent_hash_map_ref reset_cache();
    multi_function_ref add_method(object_ref dispatch_val, object_ref method);
    multi_function_ref remove_method(object_ref dispatch_val);
    multi_function_ref prefer_method(object_ref x, object_ref y);
    bool is_preferred(object_ref hierarchy, object_ref x, object_ref y) const;

    static bool is_a(object_ref hierarchy, object_ref x, object_ref y);
    bool is_dominant(object_ref hierarchy, object_ref x, object_ref y) const;

    object_ref get_fn(object_ref dispatch_val);
    object_ref get_method(object_ref dispatch_val);
    object_ref find_and_cache_best_method(object_ref dispatch_val);

    object base{ obj_type };
    object_ref dispatch{};
    object_ref default_dispatch_value{};
    object_ref hierarchy{};
    mutable object_ref cached_hierarchy{};
    persistent_hash_map_ref method_table{};
    mutable persistent_hash_map_ref method_cache{};
    persistent_hash_map_ref prefer_table{};
    symbol_ref name{};
    std::recursive_mutex data_lock;
  };
}
