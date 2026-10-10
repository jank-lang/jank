#include <mutex>

#include <CppInterOp/CppInterOp.h>

#include <jank/analyze/cppinterop.hpp>
#include <jank/runtime/context.hpp>

/* Clang's Sema and ASTContext are not thread-safe, and even read-only queries mutate them
 * (DeclContext builds lookup tables lazily, and ASTContext memoizes type info), so any facade call
 * that reads them must lock the interpreter mutex. Only the simplest queries here can go without
 * the lock.
 *
 * We re-use the mutex from the interpreter because that's our single window into the Clang/LLVM
 * JIT runtime. Everything needs to go through there. */

namespace jank::analyze::cppinterop
{
  namespace
  {
    std::vector<void *> const &to_voidp(std::vector<clang_decl> const &handles)
    {
      return *reinterpret_cast<std::vector<void *> const *>(&handles);
    }

    std::vector<clang_decl> from_voidp(std::vector<void *> const &handles)
    {
      std::vector<clang_decl> result;
      result.reserve(handles.size());
      for(auto const handle : handles)
      {
        result.emplace_back(handle);
      }
      return result;
    }
  }

  std::string mangle_rtti(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::MangleRTTI(type);
  }

  void enable_debug_output(bool value)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    Cpp::EnableDebugOutput(value);
  }

  bool is_namespace(clang_decl scope)
  {
    return Cpp::IsNamespace(scope);
  }

  bool is_class(clang_decl scope)
  {
    return Cpp::IsClass(scope);
  }

  bool is_class_template(clang_decl handle)
  {
    return Cpp::IsClassTemplate(handle);
  }

  bool is_function(clang_decl scope)
  {
    return Cpp::IsFunction(scope);
  }

  bool is_function_pointer_type(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsFunctionPointerType(type);
  }

  bool is_inline_friend_function(clang_decl scope)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsInlineFriendFunction(scope);
  }

  bool is_complete(clang_decl scope)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsComplete(scope);
  }

  bool is_builtin(clang_type type)
  {
    return Cpp::IsBuiltin(type);
  }

  bool is_integral(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsIntegral(type);
  }

  bool is_void(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsVoid(type);
  }

  clang_type get_void_type()
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetVoidType();
  }

  bool is_template(clang_decl handle)
  {
    return Cpp::IsTemplate(handle);
  }

  bool is_template_specialization(clang_decl handle)
  {
    return Cpp::IsTemplateSpecialization(handle);
  }

  bool is_template_specialization_of(clang_decl spec, clang_decl templ)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsTemplateSpecializationOf(spec, templ);
  }

  bool is_typedefed(clang_decl handle)
  {
    return Cpp::IsTypedefed(handle);
  }

  clang_type get_common_type(clang_type lhs, clang_type rhs)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetCommonType(lhs, rhs);
  }

  bool is_implicitly_convertible(clang_type from_type, clang_type to_type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsImplicitlyConvertible(from_type, to_type);
  }

  bool is_c_style_convertible(clang_type from_type, clang_type to_type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsCStyleConvertible(from_type, to_type);
  }

  bool is_constructible(clang_type to_type, clang_type from_type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsConstructible(to_type, from_type);
  }

  bool is_trivially_destructible(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsTriviallyDestructible(type);
  }

  bool is_enum_scope(clang_decl handle)
  {
    return Cpp::IsEnumScope(handle);
  }

  bool is_enum_constant(clang_decl handle)
  {
    return Cpp::IsEnumConstant(handle);
  }

  bool is_enum_type(clang_type type)
  {
    return Cpp::IsEnumType(type);
  }

  bool has_type_qualifier(clang_type type, qualifier q)
  {
    return Cpp::HasTypeQualifier(type, static_cast<Cpp::QualKind>(q));
  }

  bool is_variable(clang_decl scope)
  {
    return Cpp::IsVariable(scope);
  }

  std::string get_name(clang_decl klass)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetName(klass);
  }

  std::string get_qualified_name(clang_decl klass)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetQualifiedName(klass);
  }

  std::string get_qualified_complete_name(clang_decl klass)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetQualifiedCompleteName(klass);
  }

  std::string get_truncated_name(clang_decl klass)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetTruncatedName(klass);
  }

  std::string get_qualified_complete_name_with_template_args(clang_decl klass)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetQualifiedCompleteNameWithTemplateArgs(klass);
  }

  clang_decl get_global_scope()
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetGlobalScope();
  }

  clang_decl get_underlying_scope(clang_decl scope)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetUnderlyingScope(scope);
  }

  clang_decl get_scope_from_complete_name(std::string const &name)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetScopeFromCompleteName(name);
  }

  clang_decl get_named(std::string const &name, clang_decl parent)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetNamed(name, parent);
  }

  clang_decl get_parent_scope(clang_decl scope)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetParentScope(scope);
  }

  clang_decl get_scope_from_type(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetScopeFromType(type);
  }

  size_t get_num_bases(clang_decl klass)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetNumBases(klass);
  }

  clang_decl get_base_class(clang_decl klass, size_t ibase)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetBaseClass(klass, ibase);
  }

  bool has_usable_copy_constructor(clang_decl scope)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::HasUsableCopyConstructor(scope);
  }

  bool has_usable_move_constructor(clang_decl scope)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::HasUsableMoveConstructor(scope);
  }

  bool has_deleted_move_constructor(clang_decl scope)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::HasDeletedMoveConstructor(scope);
  }

  std::vector<clang_decl> get_functions_using_name(clang_decl scope, std::string const &name)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return from_voidp(Cpp::GetFunctionsUsingName(scope, name));
  }

  clang_type get_function_return_type(clang_decl func)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetFunctionReturnType(func);
  }

  size_t get_function_num_args(clang_decl func)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetFunctionNumArgs(func);
  }

  size_t get_function_required_args(clang_decl func)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetFunctionRequiredArgs(func);
  }

  clang_type get_function_arg_type(clang_decl func, size_t iarg)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetFunctionArgType(func, iarg);
  }

  std::string get_function_signature(clang_type func)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetFunctionSignature(func);
  }

  std::string get_function_source_info(clang_decl func)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetFunctionSourceInfo(func);
  }

  clang_type get_function_return_type_from_type(clang_type func)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetFunctionReturnTypeFromType(func);
  }

  size_t get_function_num_args_from_type(clang_type func)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetFunctionNumArgsFromType(func);
  }

  clang_type get_function_arg_type_from_type(clang_type func, size_t iarg)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetFunctionArgTypeFromType(func, iarg);
  }

  bool is_function_variadic(clang_decl function)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsFunctionVariadic(function);
  }

  bool is_function_variadic_template(clang_decl fn)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsFunctionVariadicTemplate(fn);
  }

  bool is_function_deleted(clang_decl function)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsFunctionDeleted(function);
  }

  bool is_function_type_const(clang_type function_type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsFunctionTypeConst(function_type);
  }

  bool is_templated_function(clang_decl func)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsTemplatedFunction(func);
  }

  void
  lookup_constructors(std::string const &name, clang_decl parent, std::vector<clang_decl> &funcs)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    std::vector<void *> cpp_funcs;
    Cpp::LookupConstructors(name, parent, cpp_funcs);
    funcs = from_voidp(cpp_funcs);
  }

  bool is_method(clang_decl method)
  {
    return Cpp::IsMethod(method);
  }

  bool is_protected_method(clang_decl method)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsProtectedMethod(method);
  }

  bool is_private_method(clang_decl method)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsPrivateMethod(method);
  }

  bool is_constructor(clang_decl method)
  {
    return Cpp::IsConstructor(method);
  }

  bool is_destructor(clang_decl method)
  {
    return Cpp::IsDestructor(method);
  }

  bool is_static_method(clang_decl method)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsStaticMethod(method);
  }

  void get_datamembers(clang_decl scope, std::vector<clang_decl> &datamembers)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    std::vector<void *> cpp_datamembers;
    Cpp::GetDatamembers(scope, cpp_datamembers);
    datamembers = from_voidp(cpp_datamembers);
  }

  void get_static_datamembers(clang_decl scope, std::vector<clang_decl> &datamembers)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    std::vector<void *> cpp_datamembers;
    Cpp::GetStaticDatamembers(scope, cpp_datamembers);
    datamembers = from_voidp(cpp_datamembers);
  }

  clang_decl lookup_datamember(std::string const &name, clang_decl parent)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::LookupDatamember(name, parent);
  }

  std::vector<clang_decl> lookup_methods(std::string const &name, clang_decl parent)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return from_voidp(Cpp::LookupMethods(name, parent));
  }

  intptr_t get_variable_offset(clang_decl var, clang_decl parent)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetVariableOffset(var, parent);
  }

  bool is_protected_variable(clang_decl var)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsProtectedVariable(var);
  }

  bool is_private_variable(clang_decl var)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsPrivateVariable(var);
  }

  bool is_static_variable(clang_decl var)
  {
    return Cpp::IsStaticVariable(var);
  }

  bool is_non_static_variable(clang_decl scope)
  {
    return Cpp::IsNonStaticVariable(scope);
  }

  bool is_const_type(clang_type type)
  {
    return Cpp::IsConstType(type);
  }

  bool is_pointer_type(clang_type type)
  {
    return Cpp::IsPointerType(type);
  }

  bool is_pointer_to_member_type(clang_type type)
  {
    return Cpp::IsPointerToMemberType(type);
  }

  bool is_pointer_to_member_variable_type(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsPointerToMemberVariableType(type);
  }

  bool is_pointer_to_member_function_type(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsPointerToMemberFunctionType(type);
  }

  clang_type get_parent_type_from_pointer_to_member(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetParentTypeFromPointerToMember(type);
  }

  clang_type get_function_type_from_pointer_to_member(clang_type member_type, clang_type obj_type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetFunctionTypeFromPointerToMember(member_type, obj_type);
  }

  bool is_array_type(clang_type type)
  {
    return Cpp::IsArrayType(type);
  }

  bool is_sized_array_type(clang_type type)
  {
    return Cpp::IsSizedArrayType(type);
  }

  size_t get_array_size(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetArraySize(type);
  }

  clang_type get_array_element_type(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetArrayElementType(type);
  }

  clang_type get_array_type(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetArrayType(type);
  }

  clang_type get_array_type(clang_type type, size_t size)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetArrayType(type, size);
  }

  clang_type get_function_type(clang_type ret, std::vector<clang_type> const &params)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetFunctionType(ret, to_voidp(params));
  }

  clang_type get_pointee_type(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetPointeeType(type);
  }

  clang_type get_pointer_type(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetPointerType(type);
  }

  clang_type get_pointer_to_member_type(clang_decl member)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetPointerToMemberType(member);
  }

  clang_type get_lvalue_reference_type(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetLValueReferenceType(type);
  }

  clang_type get_rvalue_reference_type(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetRValueReferenceType(type);
  }

  bool is_reference_type(clang_type type)
  {
    return Cpp::IsReferenceType(type);
  }

  bool is_rvalue_reference_type(clang_type type)
  {
    return Cpp::IsRvalueReferenceType(type);
  }

  value_category get_value_category(clang_type type)
  {
    return static_cast<value_category>(Cpp::GetValueKind(type));
  }

  clang_type get_non_reference_type(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetNonReferenceType(type);
  }

  clang_type get_underlying_type(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetUnderlyingType(type);
  }

  clang_type get_type_without_cv(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetTypeWithoutCv(type);
  }

  clang_type get_type_with_const(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetTypeWithConst(type);
  }

  clang_type get_type_with_volatile(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetTypeWithVolatile(type);
  }

  clang_type get_signed_type(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetSignedType(type);
  }

  clang_type get_unsigned_type(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetUnsignedType(type);
  }

  clang_type get_short_type(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetShortType(type);
  }

  clang_type get_long_type(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetLongType(type);
  }

  bool is_short_type(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsShortType(type);
  }

  std::string get_type_as_string(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetTypeAsString(type);
  }

  std::string get_type_as_truncated_string(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetTypeAsTruncatedString(type);
  }

  clang_type get_canonical_type(clang_type type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetCanonicalType(type);
  }

  clang_type get_type(std::string const &type)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetType(type);
  }

  clang_type get_type_from_scope(clang_decl klass)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetTypeFromScope(klass);
  }

  bool is_type_derived_from(clang_type derived, clang_type base)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsTypeDerivedFrom(derived, base);
  }

  bool is_const_method(clang_decl method)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::IsConstMethod(method);
  }

  std::string get_function_arg_name(clang_decl func, size_t param_index)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetFunctionArgName(func, param_index);
  }

  void get_operator(CppImpl::Operator op,
                    std::vector<clang_type> const &arg_types,
                    std::vector<clang_decl> &operators,
                    CppImpl::OperatorArity kind)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    std::vector<void *> cpp_operators;
    Cpp::GetOperator(op, to_voidp(arg_types), cpp_operators, kind);
    operators = from_voidp(cpp_operators);
  }

  clang_type
  get_builtin_operator_type(CppImpl::Operator op, std::vector<clang_type> const &arg_types)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetBuiltinOperatorType(op, to_voidp(arg_types));
  }

  clang_decl create_interpreter(std::vector<char const *> const &args,
                                std::vector<char const *> const &gpu_args,
                                std::map<char const *, std::string_view> const &vfs,
                                std::optional<int> const &cm,
                                bool *pch_out_of_date)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::CreateInterpreter(args, gpu_args, vfs, cm, pch_out_of_date);
  }

  // Deliberately unlocked: this only spawns an external clang process and touches no Sema or
  // ASTContext state, so it should not block interpreter work on other threads.
  std::string detect_resource_dir(char const *clang_binary_name)
  {
    return Cpp::DetectResourceDir(clang_binary_name);
  }

  clang_decl instantiate_template(clang_decl tmpl,
                                  CppImpl::TemplateArgInfo const *template_args,
                                  size_t template_args_size,
                                  bool instantiate_body)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::InstantiateTemplate(tmpl, template_args, template_args_size, instantiate_body);
  }

  bool instantiate_template(clang_decl spec)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::InstantiateTemplate(spec);
  }

  std::vector<clang_decl> best_overload_match(std::vector<clang_decl> const &candidates,
                                              std::vector<clang_type> const &arg_types,
                                              std::vector<clang_decl> const &arg_scopes)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return from_voidp(
      Cpp::BestOverloadMatch(to_voidp(candidates), to_voidp(arg_types), to_voidp(arg_scopes)));
  }

  CppImpl::OverloadCandidateInfo
  get_overload_candidate_info(clang_decl candidate,
                              std::vector<clang_type> const &arg_types,
                              std::vector<clang_decl> const &arg_scopes)
  {
    auto const lock{ runtime::__rt_ctx->jit_prc.interpreter.lock() };
    return Cpp::GetOverloadCandidateInfo(candidate, to_voidp(arg_types), to_voidp(arg_scopes));
  }
}
