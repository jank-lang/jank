#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <jtl/ptr.hpp>

namespace CppImpl
{
  struct TemplateArgInfo;
  enum Operator : unsigned char;
  enum OperatorArity : unsigned char;
  struct OverloadCandidateInfo;
}

namespace jank::analyze::cppinterop
{
  using clang_type = jtl::ptr<void>;
  using clang_decl = jtl::ptr<void>;

  enum class value_category : u8
  {
    none,
    lvalue,
    rvalue,
  };

  enum class qualifier : u8
  {
    const_ = 1 << 0,
    volatile_ = 1 << 1,
    restrict_ = 1 << 2
  };

  std::string mangle_rtti(clang_type type);
  void enable_debug_output(bool value = true);
  bool is_namespace(clang_decl scope);
  bool is_class(clang_decl scope);
  bool is_class_template(clang_decl handle);
  bool is_function(clang_decl scope);
  bool is_function_pointer_type(clang_type type);
  bool is_inline_friend_function(clang_decl scope);
  bool is_complete(clang_decl scope);
  bool is_builtin(clang_type type);
  bool is_integral(clang_type type);
  bool is_void(clang_type type);
  clang_type get_void_type();
  bool is_template(clang_decl handle);
  bool is_template_specialization(clang_decl handle);
  bool is_template_specialization_of(clang_decl spec, clang_decl templ);
  bool is_typedefed(clang_decl handle);
  clang_type get_common_type(clang_type lhs, clang_type rhs);
  bool is_implicitly_convertible(clang_type from_type, clang_type to_type);
  bool is_c_style_convertible(clang_type from_type, clang_type to_type);
  bool is_constructible(clang_type to_type, clang_type from_type);
  bool is_trivially_destructible(clang_type type);
  bool is_enum_scope(clang_decl handle);
  bool is_enum_constant(clang_decl handle);
  bool is_enum_type(clang_type type);
  bool has_type_qualifier(clang_type type, qualifier q);
  bool is_variable(clang_decl scope);
  std::string get_name(clang_decl klass);
  std::string get_qualified_name(clang_decl klass);
  std::string get_qualified_complete_name(clang_decl klass);
  std::string get_truncated_name(clang_decl klass);
  std::string get_qualified_complete_name_with_template_args(clang_decl klass);
  clang_decl get_global_scope();
  clang_decl get_underlying_scope(clang_decl scope);
  clang_decl get_scope_from_complete_name(std::string const &name);
  clang_decl get_named(std::string const &name, clang_decl parent = nullptr);
  clang_decl get_parent_scope(clang_decl scope);
  clang_decl get_scope_from_type(clang_type type);
  size_t get_num_bases(clang_decl klass);
  clang_decl get_base_class(clang_decl klass, size_t ibase);
  bool has_usable_copy_constructor(clang_decl scope);
  bool has_usable_move_constructor(clang_decl scope);
  bool has_deleted_move_constructor(clang_decl scope);
  std::vector<clang_decl> get_functions_using_name(clang_decl scope, std::string const &name);
  clang_type get_function_return_type(clang_decl func);
  size_t get_function_num_args(clang_decl func);
  size_t get_function_required_args(clang_decl func);
  clang_type get_function_arg_type(clang_decl func, size_t iarg);
  std::string get_function_signature(clang_type func);
  std::string get_function_source_info(clang_decl func);
  clang_type get_function_return_type_from_type(clang_type func);
  size_t get_function_num_args_from_type(clang_type func);
  clang_type get_function_arg_type_from_type(clang_type func, size_t iarg);
  bool is_function_variadic(clang_decl function);
  bool is_function_variadic_template(clang_decl fn);
  bool is_function_deleted(clang_decl function);
  bool is_function_type_const(clang_type function_type);
  bool is_templated_function(clang_decl func);
  void
  lookup_constructors(std::string const &name, clang_decl parent, std::vector<clang_decl> &funcs);
  bool is_method(clang_decl method);
  bool is_protected_method(clang_decl method);
  bool is_private_method(clang_decl method);
  bool is_constructor(clang_decl method);
  bool is_destructor(clang_decl method);
  bool is_static_method(clang_decl method);
  void get_datamembers(clang_decl scope, std::vector<clang_decl> &datamembers);
  void get_static_datamembers(clang_decl scope, std::vector<clang_decl> &datamembers);
  clang_decl lookup_datamember(std::string const &name, clang_decl parent);
  std::vector<clang_decl> lookup_methods(std::string const &name, clang_decl parent);
  intptr_t get_variable_offset(clang_decl var, clang_decl parent = nullptr);
  bool is_protected_variable(clang_decl var);
  bool is_private_variable(clang_decl var);
  bool is_static_variable(clang_decl var);
  bool is_non_static_variable(clang_decl scope);
  bool is_const_type(clang_type type);
  bool is_pointer_type(clang_type type);
  bool is_pointer_to_member_type(clang_type type);
  bool is_pointer_to_member_variable_type(clang_type type);
  bool is_pointer_to_member_function_type(clang_type type);
  clang_type get_parent_type_from_pointer_to_member(clang_type type);
  clang_type get_function_type_from_pointer_to_member(clang_type member_type, clang_type obj_type);
  bool is_array_type(clang_type type);
  bool is_sized_array_type(clang_type type);
  size_t get_array_size(clang_type type);
  clang_type get_array_element_type(clang_type type);
  clang_type get_array_type(clang_type type);
  clang_type get_array_type(clang_type type, size_t size);
  clang_type get_function_type(clang_type ret, std::vector<clang_type> const &params);
  clang_type get_pointee_type(clang_type type);
  clang_type get_pointer_type(clang_type type);
  clang_type get_pointer_to_member_type(clang_decl member);
  clang_type get_lvalue_reference_type(clang_type type);
  clang_type get_rvalue_reference_type(clang_type type);
  bool is_reference_type(clang_type type);
  bool is_rvalue_reference_type(clang_type type);
  value_category get_value_category(clang_type type);
  clang_type get_non_reference_type(clang_type type);
  clang_type get_underlying_type(clang_type type);
  clang_type get_type_without_cv(clang_type type);
  clang_type get_type_with_const(clang_type type);
  clang_type get_type_with_volatile(clang_type type);
  clang_type get_signed_type(clang_type type);
  clang_type get_unsigned_type(clang_type type);
  clang_type get_short_type(clang_type type);
  clang_type get_long_type(clang_type type);
  bool is_short_type(clang_type type);
  std::string get_type_as_string(clang_type type);
  std::string get_type_as_truncated_string(clang_type type);
  clang_type get_canonical_type(clang_type type);
  clang_type get_type(std::string const &type);
  clang_type get_type_from_scope(clang_decl klass);
  bool is_type_derived_from(clang_type derived, clang_type base);
  bool is_const_method(clang_decl method);
  std::string get_function_arg_name(clang_decl func, size_t param_index);
  void get_operator(CppImpl::Operator op,
                    std::vector<clang_type> const &arg_types,
                    std::vector<clang_decl> &operators,
                    CppImpl::OperatorArity kind);
  clang_type get_builtin_operator_type(CppImpl::Operator op,
                                       std::vector<clang_type> const &arg_types);
  clang_decl create_interpreter(std::vector<char const *> const &args = {},
                                std::vector<char const *> const &gpu_args = {},
                                std::map<char const *, std::string_view> const &vfs = {},
                                std::optional<int> const &cm = std::nullopt,
                                bool *pch_out_of_date = nullptr);
  std::string detect_resource_dir(char const *clang_binary_name = "clang");
  clang_decl instantiate_template(clang_decl tmpl,
                                  CppImpl::TemplateArgInfo const *template_args,
                                  size_t template_args_size,
                                  bool instantiate_body = false);
  bool instantiate_template(clang_decl spec);
  std::vector<clang_decl> best_overload_match(std::vector<clang_decl> const &candidates,
                                              std::vector<clang_type> const &arg_types,
                                              std::vector<clang_decl> const &arg_scopes);
  CppImpl::OverloadCandidateInfo
  get_overload_candidate_info(clang_decl candidate,
                              std::vector<clang_type> const &arg_types,
                              std::vector<clang_decl> const &arg_scopes);
}
