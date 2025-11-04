#pragma once

#include <jtl/ptr.hpp>

#include <jank/analyze/processor.hpp>

namespace Cpp
{
  class AotCall;
}

namespace llvm::orc
{
  class ThreadSafeModule;
}

namespace jank::runtime::obj
{
  using nil_ref = oref<struct nil>;
  using keyword_ref = oref<struct keyword>;
  using boolean_ref = oref<struct boolean>;
  using integer_ref = oref<struct integer>;
  using big_integer_ref = oref<struct big_integer>;
  using real_ref = oref<struct real>;
  using ratio_ref = oref<struct ratio>;
  using persistent_string_ref = oref<struct persistent_string>;
  using character_ref = oref<struct character>;
}

namespace jank::analyze
{
  using expression_ref = jtl::ref<struct expression>;

  namespace expr
  {
    using def_ref = jtl::ref<struct def>;
    using var_deref_ref = jtl::ref<struct var_deref>;
    using var_ref_ref = jtl::ref<struct var_ref>;
    using call_ref = jtl::ref<struct call>;
    using primitive_literal_ref = jtl::ref<struct primitive_literal>;
    using list_ref = jtl::ref<struct list>;
    using vector_ref = jtl::ref<struct vector>;
    using map_ref = jtl::ref<struct map>;
    using set_ref = jtl::ref<struct set>;
    using local_reference_ref = jtl::ref<struct local_reference>;
    using function_ref = jtl::ref<struct function>;
    using recur_ref = jtl::ref<struct recur>;
    using recursion_reference_ref = jtl::ref<struct recursion_reference>;
    using named_recursion_ref = jtl::ref<struct named_recursion>;
    using let_ref = jtl::ref<struct let>;
    using letfn_ref = jtl::ref<struct letfn>;
    using do_ref = jtl::ref<struct do_>;
    using if_ref = jtl::ref<struct if_>;
    using throw_ref = jtl::ref<struct throw_>;
    using try_ref = jtl::ref<struct try_>;
    using case_ref = jtl::ref<struct case_>;
    using cpp_raw_ref = jtl::ref<struct cpp_raw>;
    using cpp_type_ref = jtl::ref<struct cpp_type>;
    using cpp_value_ref = jtl::ref<struct cpp_value>;
    using cpp_cast_ref = jtl::ref<struct cpp_cast>;
    using cpp_call_ref = jtl::ref<struct cpp_call>;
    using cpp_constructor_call_ref = jtl::ref<struct cpp_constructor_call>;
    using cpp_member_call_ref = jtl::ref<struct cpp_member_call>;
    using cpp_member_access_ref = jtl::ref<struct cpp_member_access>;
    using cpp_builtin_operator_call_ref = jtl::ref<struct cpp_builtin_operator_call>;
    using cpp_box_ref = jtl::ref<struct cpp_box>;
    using cpp_unbox_ref = jtl::ref<struct cpp_unbox>;
    using cpp_new_ref = jtl::ref<struct cpp_new>;
    using cpp_delete_ref = jtl::ref<struct cpp_delete>;
  }
}

namespace jank::codegen
{
  using namespace jank::runtime;

  enum class compilation_target : u8
  {
    module,
    function,
    eval
  };

  constexpr char const *compilation_target_str(compilation_target const t)
  {
    switch(t)
    {
      case compilation_target::module:
        return "module";
      case compilation_target::function:
        return "function";
      case compilation_target::eval:
        return "eval";
      default:
        return "unknown";
    }
  }

  enum class var_root_kind : u8;

  struct reusable_context;

  struct llvm_processor
  {
    struct impl;

    llvm_processor() = delete;
    llvm_processor(analyze::expr::function_ref const expr,
                   jtl::immutable_string const &module,
                   compilation_target target);
    /* For this ctor, we're inheriting the context from another function, which means
     * we're building a nested function. */
    llvm_processor(analyze::expr::function_ref expr, jtl::ref<reusable_context> ctx);
    llvm_processor(llvm_processor const &) = delete;
    llvm_processor(llvm_processor &&) noexcept = default;

    jtl::string_result<void> gen() const;
    void optimize() const;
    void print() const;

    llvm::orc::ThreadSafeModule &get_module() const;
    jtl::immutable_string const &get_module_name() const;
    jtl::immutable_string get_root_fn_name() const;

    jtl::ptr<impl> _impl{};
  };
}
