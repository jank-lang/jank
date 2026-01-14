#pragma once

#include <jtl/ptr.hpp>
#include <jtl/option.hpp>

#include <jank/runtime/obj/symbol.hpp>

namespace jank::runtime
{
  struct context;
  using var_ref = oref<struct var>;

  namespace obj
  {
    using symbol_ref = oref<struct symbol>;
  }
}

namespace jank::analyze::cpp_util
{
  jtl::ptr<void> untyped_object_ref_type();
}

namespace jank::analyze
{
  struct expression;

  namespace expr
  {
    using function_context_ref = jtl::ref<struct function_context>;
  }

  struct local_binding
  {
    runtime::obj::symbol_ref name{};
    jtl::immutable_string native_name{};
    jtl::option<jtl::ref<expression>> value_expr{};
    jtl::ptr<struct local_frame> originating_frame{};
    bool needs_box{ true };
    bool has_boxed_usage{};
    bool has_unboxed_usage{};
    /* TODO: This gets stomped when a binding is shadowed. Do we
     * need to handle shadowing more delicately? */
    jtl::ptr<void> type{ analyze::cpp_util::untyped_object_ref_type() };

    runtime::object_ref to_runtime_data() const;
  };

  using local_binding_ptr = jtl::ptr<local_binding>;

  struct local_frame
  {
    enum class frame_type : u8
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

    static constexpr bool pointer_free{ false };

    local_frame() = delete;
    local_frame(local_frame const &) = default;
    local_frame(local_frame &&) noexcept = default;
    local_frame(frame_type const &type, jtl::option<jtl::ptr<local_frame>> const &p);

    struct binding_find_result
    {
      local_binding_ptr binding;
      native_vector<jtl::ptr<local_frame>> crossed_fns;
    };

    struct named_recursion_find_result
    {
      jtl::ptr<local_frame> fn_frame;
      native_vector<jtl::ptr<local_frame>> crossed_fns;
    };

    /* This is used to find both captures and regular locals, since it's
     * impossible to know which one a sym is without finding it. */
    jtl::option<binding_find_result> find_local_or_capture(runtime::obj::symbol_ref const sym);
    static void register_captures(binding_find_result const &result);
    static void
    register_captures(jtl::ptr<local_frame> frame, named_recursion_find_result const &result);

    /* This can be used when you have a capture, but you want to trace it back to the
     * originating local. */
    jtl::option<binding_find_result> find_originating_local(runtime::obj::symbol_ref const sym);

    jtl::option<named_recursion_find_result>
    find_named_recursion(runtime::obj::symbol_ref const sym);

    static bool within_same_fn(jtl::ptr<local_frame>, jtl::ptr<local_frame>);

    static local_frame const &find_closest_fn_frame(local_frame const &frame);
    static local_frame &find_closest_fn_frame(local_frame &frame);

    runtime::object_ref to_runtime_data() const;

    frame_type type;
    jtl::option<jtl::ptr<local_frame>> parent;
    native_unordered_map<runtime::obj::symbol_ref, local_binding> locals;
    native_unordered_map<runtime::obj::symbol_ref, local_binding> captures;
    /* This is only set if the frame type is fn. */
    jtl::ptr<expr::function_context> fn_ctx;
  };

  /* TODO: Use a ref. */
  using local_frame_ptr = jtl::ptr<local_frame>;
}
