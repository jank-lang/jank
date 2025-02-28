#pragma once

#include <jank/analyze/expr/do.hpp>

namespace jank::runtime::obj
{
  using symbol_ptr = native_box<struct symbol>;
  using persistent_hash_map_ptr = native_box<struct persistent_hash_map>;
  using persistent_hash_map_ptr = native_box<struct persistent_hash_map>;
}

namespace jank::analyze
{
  using local_binding_ptr = runtime::native_box<struct local_binding>;
}

namespace jank::analyze::expr
{
  using function_ptr = runtime::native_box<struct function>;

  struct function_context : gc
  {
    static constexpr native_bool pointer_free{ true };

    function_ptr fn{};
    native_persistent_string name;
    native_persistent_string unique_name;
    size_t param_count{};
    native_bool is_variadic{};
    native_bool is_tail_recursive{};
    /* TODO: is_pure */
  };

  using function_context_ptr = runtime::native_box<function_context>;

  struct function_arity
  {
    runtime::object_ptr to_runtime_data() const;

    native_vector<runtime::obj::symbol_ptr> params;
    do_ptr body{};
    local_frame_ptr frame{};
    function_context_ptr fn_ctx{};
  };

  struct arity_key
  {
    native_bool operator==(arity_key const &rhs) const;

    size_t param_count{};
    native_bool is_variadic{};
  };

  using function_ptr = runtime::native_box<struct function>;

  struct function : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::function };

    function();
    function(expression_position position,
             local_frame_ptr frame,
             native_bool needs_box,
             native_persistent_string const &name,
             native_persistent_string const &unique_name,
             native_vector<function_arity> &&arities,
             runtime::obj::persistent_hash_map_ptr meta);

    /* Aggregates all `frame->captures` from each arity so that we can know the overall
     * captures for all arities of this fn. This is necessary for codegen to IR, since we
     * generate a context struct which is shared across all arities, even if one arity
     * doesn't use any captures. */
    native_unordered_map<runtime::obj::symbol_ptr, local_binding_ptr> captures() const;
    runtime::object_ptr to_runtime_data() const override;

    native_persistent_string name;
    native_persistent_string unique_name;
    native_vector<function_arity> arities;
    runtime::obj::persistent_hash_map_ptr meta{};
  };
}

namespace std
{
  template <>
  struct hash<jank::analyze::expr::arity_key>
  {
    size_t operator()(jank::analyze::expr::arity_key const &k) const noexcept;
  };
}
