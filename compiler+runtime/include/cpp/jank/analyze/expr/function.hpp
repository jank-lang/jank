#pragma once

#include <jank/analyze/expr/do.hpp>

namespace jank::runtime::obj
{
  using symbol_ref = oref<struct symbol>;
  using persistent_hash_map_ref = oref<struct persistent_hash_map>;
  using persistent_hash_map_ref = oref<struct persistent_hash_map>;
}

namespace jank::analyze
{
  using local_binding_ptr = jtl::ptr<struct local_binding>;
}

namespace jank::analyze::expr
{
  using function_ref = jtl::ref<struct function>;

  struct function_context : gc
  {
    static constexpr bool pointer_free{ true };

    jtl::ptr<function> fn;
    jtl::immutable_string name;
    jtl::immutable_string unique_name;
    usize param_count{};
    bool is_variadic{};
    bool is_tail_recursive{};
    /* TODO: is_pure */
  };

  using function_context_ref = jtl::ref<function_context>;

  struct function_arity
  {
    runtime::object_ref to_runtime_data() const;

    native_vector<runtime::obj::symbol_ref> params;
    do_ref body;
    local_frame_ptr frame;
    function_context_ref fn_ctx;
  };

  struct arity_key
  {
    bool operator==(arity_key const &rhs) const;

    usize param_count{};
    bool is_variadic{};
  };

  struct function : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::function };

    function();
    function(expression_position position,
             local_frame_ptr frame,
             bool needs_box,
             jtl::immutable_string const &name,
             jtl::immutable_string const &unique_name,
             native_vector<function_arity> &&arities,
             runtime::obj::persistent_hash_map_ref meta);

    /* Aggregates all `frame->captures` from each arity so that we can know the overall
     * captures for all arities of this fn. This is necessary for codegen to IR, since we
     * generate a context struct which is shared across all arities, even if one arity
     * doesn't use any captures. */
    native_unordered_map<runtime::obj::symbol_ref, local_binding_ptr> captures() const;
    runtime::object_ref to_runtime_data() const override;

    jtl::immutable_string name;
    jtl::immutable_string unique_name;
    native_vector<function_arity> arities;
    runtime::obj::persistent_hash_map_ref meta{};
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
