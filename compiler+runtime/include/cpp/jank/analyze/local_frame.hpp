#pragma once

#include <jank/option.hpp>
#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  struct context;

  namespace obj
  {
    using symbol_ptr = native_box<struct symbol>;
  }
}

namespace jank::analyze
{
  struct expression;
  using jank::runtime::native_box;

  namespace expr
  {
    using function_context_ptr = native_box<struct function_context>;
  }

  struct lifted_var
  {
    runtime::obj::symbol_ptr var_name{};

    runtime::object_ptr to_runtime_data() const;
  };

  /* TODO: Track constant usages to figure out if boxing is needed at all,
   * rather than just doing both. */
  struct lifted_constant
  {
    runtime::object_ptr data{};

    runtime::object_ptr to_runtime_data() const;
  };

  struct local_binding
  {
    runtime::obj::symbol_ptr name{};
    option<native_box<expression>> value_expr{};
    native_box<struct local_frame> originating_frame{};
    native_bool needs_box{ true };
    native_bool has_boxed_usage{};
    native_bool has_unboxed_usage{};
    /* The name of the function itself. */
    native_bool is_named_recur{};

    runtime::object_ptr to_runtime_data() const;
  };

  struct local_frame : gc
  {
    enum class frame_type : uint8_t
    {
      root,
      fn,
      let,
      letfn,
      try_,
      catch_,
      finally
    };

    static constexpr char const *frame_type_str(frame_type const type)
    {
      switch(type)
      {
        case frame_type::root:
          return "root";
        case frame_type::fn:
          return "fn";
        case frame_type::let:
          return "let";
        case frame_type::letfn:
          return "letfn";
        case frame_type::try_:
          return "try_";
        case frame_type::catch_:
          return "catch_";
        case frame_type::finally:
          return "finally";
      }
      return "unknown";
    }

    static constexpr native_bool pointer_free{ false };

    local_frame() = delete;
    local_frame(local_frame const &) = default;
    local_frame(local_frame &&) noexcept = default;
    local_frame(frame_type const &type,
                runtime::context &ctx,
                option<native_box<local_frame>> const &p);

    local_frame &operator=(local_frame const &rhs);
    local_frame &operator=(local_frame &&rhs) noexcept;

    struct find_result
    {
      local_binding &binding;
      native_vector<native_box<local_frame>> crossed_fns;
    };

    /* This is used to find both captures and regular locals, since it's
     * impossible to know which one a sym is without finding it. */
    option<find_result> find_local_or_capture(runtime::obj::symbol_ptr sym);
    static void register_captures(find_result const &result);

    /* This can be used when you have a capture, but you want to trace it back to the
     * originating local. */
    option<find_result> find_originating_local(runtime::obj::symbol_ptr sym);

    option<expr::function_context_ptr> find_named_recursion(runtime::obj::symbol_ptr sym);

    static native_bool within_same_fn(native_box<local_frame>, native_box<local_frame>);

    runtime::obj::symbol_ptr lift_var(runtime::obj::symbol_ptr const &);
    option<std::reference_wrapper<lifted_var const>>
    find_lifted_var(runtime::obj::symbol_ptr const &) const;

    void lift_constant(runtime::object_ptr);
    option<std::reference_wrapper<lifted_constant const>>
      find_lifted_constant(runtime::object_ptr) const;

    static local_frame const &find_closest_fn_frame(local_frame const &frame);
    static local_frame &find_closest_fn_frame(local_frame &frame);

    runtime::object_ptr to_runtime_data() const;

    frame_type type;
    option<native_box<local_frame>> parent;
    /* TODO: local_binding_ptr */
    native_unordered_map<runtime::obj::symbol_ptr, local_binding> locals;
    native_unordered_map<runtime::obj::symbol_ptr, local_binding> captures;
    native_unordered_map<runtime::obj::symbol_ptr, lifted_var> lifted_vars;
    native_unordered_map<runtime::object_ptr,
                         lifted_constant,
                         std::hash<runtime::object_ptr>,
                         runtime::very_equal_to>
      lifted_constants;
    /* This is only set if the frame type is fn. */
    expr::function_context_ptr fn_ctx{};
    /* TODO: Remove this. */
    runtime::context &rt_ctx;
  };

  using local_frame_ptr = native_box<local_frame>;
}
