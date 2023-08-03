#pragma once

#include <unordered_map>

#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/option.hpp>

namespace jank::analyze
{
  struct expression;

  struct lifted_var
  {
    runtime::obj::symbol native_name;
    runtime::obj::symbol_ptr var_name{};
  };

  /* TODO: Track constant usages to figure out if boxing is needed at all,
   * rather than just doing both. */
  struct lifted_constant
  {
    runtime::obj::symbol native_name;
    option<runtime::obj::symbol> unboxed_native_name;
    runtime::object_ptr data{};
  };

  struct local_binding
  {
    runtime::obj::symbol_ptr name{};
    option<native_box<expression>> value_expr;
  };

  struct local_frame : gc
  {
    enum class frame_type
    {
      root,
      fn,
      let
    };

    static constexpr bool pointer_free{ false };

    local_frame() = delete;
    local_frame(local_frame const &) = default;
    local_frame(local_frame &&) noexcept = default;
    local_frame
    (
      frame_type const &type,
      runtime::context &ctx,
      option<native_box<local_frame>> const &p
    );

    local_frame& operator=(local_frame const &rhs);
    local_frame& operator=(local_frame &&rhs);

    struct find_result
    {
      local_binding &binding;
      native_vector<native_box<local_frame>> crossed_fns;
    };

    /* This is used to find both captures and regular locals, since it's
     * impossible to know which one a sym is without finding it. */
    option<find_result> find_local_or_capture(runtime::obj::symbol_ptr sym);
    static void register_captures(find_result const &result);

    runtime::obj::symbol_ptr lift_var(runtime::obj::symbol_ptr const &);
    option<std::reference_wrapper<lifted_var const>> find_lifted_var
    (runtime::obj::symbol_ptr const &) const;

    void lift_constant(runtime::object_ptr);
    option<std::reference_wrapper<lifted_constant const>> find_lifted_constant
    (runtime::object_ptr) const;

    frame_type type;
    option<native_box<local_frame>> parent;
    native_unordered_map<runtime::obj::symbol_ptr, local_binding> locals;
    native_unordered_map<runtime::obj::symbol_ptr, local_binding> captures;
    native_unordered_map<runtime::obj::symbol_ptr, lifted_var> lifted_vars;
    native_unordered_map<runtime::object_ptr, lifted_constant> lifted_constants;
    runtime::context &rt_ctx;
  };
  using local_frame_ptr = native_box<local_frame>;
}
