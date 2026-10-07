#include <jank/analyze/cppinterop.hpp>

#include <CppInterOp/CppInterOp.h>

namespace jank::analyze::cppinterop
{
  namespace
  {
    std::vector<void *> to_voidp(std::vector<clang_decl> const &handles)
    {
      std::vector<void *> result;
      result.reserve(handles.size());
      for(auto const handle : handles)
      {
        result.push_back(handle.data);
      }
      return result;
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

  std::vector<clang_decl> BestOverloadMatch(std::vector<clang_decl> const &candidates,
                                            std::vector<clang_type> const &arg_types,
                                            std::vector<clang_decl> const &arg_scopes)
  {
    auto const cpp_candidates{ to_voidp(candidates) };
    auto const cpp_arg_scopes{ to_voidp(arg_scopes) };

    /* TODO: Update BestOverloadMatch to take vector<type> */
    std::vector<Cpp::TemplateArgInfo> arg_type_infos;
    arg_type_infos.reserve(arg_types.size());
    for(auto const type : arg_types)
    {
      arg_type_infos.emplace_back(type);
    }
    return from_voidp(Cpp::BestOverloadMatch(cpp_candidates, arg_type_infos, cpp_arg_scopes));
  }

  clang_decl CreateInterpreter(std::vector<char const *> const &args,
                               std::vector<char const *> const &gpu_args,
                               std::map<char const *, std::string_view> const &vfs,
                               std::optional<int> const &cm,
                               bool *pch_out_of_date)
  {
    return Cpp::CreateInterpreter(args, gpu_args, vfs, cm, pch_out_of_date);
  }

  std::string DetectResourceDir(char const *clang_binary_name)
  {
    return Cpp::DetectResourceDir(clang_binary_name);
  }

  void EnableDebugOutput(bool value)
  {
    Cpp::EnableDebugOutput(value);
  }

  clang_type GetArrayElementType(clang_type type)
  {
    return Cpp::GetArrayElementType(type);
  }

  size_t GetArraySize(clang_type type)
  {
    return Cpp::GetArraySize(type);
  }

  clang_type GetArrayType(clang_type type)
  {
    return Cpp::GetArrayType(type);
  }

  clang_type GetArrayType(clang_type type, size_t size)
  {
    return Cpp::GetArrayType(type, size);
  }

  clang_decl GetBaseClass(clang_decl klass, size_t ibase)
  {
    return Cpp::GetBaseClass(klass, ibase);
  }

  clang_type GetCanonicalType(clang_type type)
  {
    return Cpp::GetCanonicalType(type);
  }

  clang_type GetCommonType(clang_type lhs, clang_type rhs)
  {
    return Cpp::GetCommonType(lhs, rhs);
  }

  void GetDatamembers(clang_decl scope, std::vector<clang_decl> &datamembers)
  {
    std::vector<void *> cpp_datamembers;
    Cpp::GetDatamembers(scope, cpp_datamembers);
    datamembers = from_voidp(cpp_datamembers);
  }

  std::string GetFunctionArgName(clang_decl func, size_t param_index)
  {
    return Cpp::GetFunctionArgName(func, param_index);
  }

  clang_type GetFunctionArgType(clang_decl func, size_t iarg)
  {
    return Cpp::GetFunctionArgType(func, iarg);
  }

  clang_type GetFunctionArgTypeFromType(clang_type func, size_t iarg)
  {
    return Cpp::GetFunctionArgTypeFromType(func, iarg);
  }

  size_t GetFunctionNumArgs(clang_decl func)
  {
    return Cpp::GetFunctionNumArgs(func);
  }

  size_t GetFunctionNumArgsFromType(clang_type func)
  {
    return Cpp::GetFunctionNumArgsFromType(func);
  }

  size_t GetFunctionRequiredArgs(clang_decl func)
  {
    return Cpp::GetFunctionRequiredArgs(func);
  }

  clang_type GetFunctionReturnType(clang_decl func)
  {
    return Cpp::GetFunctionReturnType(func);
  }

  clang_type GetFunctionReturnTypeFromType(clang_type func)
  {
    return Cpp::GetFunctionReturnTypeFromType(func);
  }

  std::string GetFunctionSignature(clang_type func)
  {
    return Cpp::GetFunctionSignature(func);
  }

  std::string GetFunctionSourceInfo(clang_decl func)
  {
    return Cpp::GetFunctionSourceInfo(func);
  }

  clang_type GetFunctionType(clang_type ret, std::vector<clang_type> const &params)
  {
    auto const cpp_params{ to_voidp(params) };
    return Cpp::GetFunctionType(ret, cpp_params);
  }

  clang_type GetFunctionTypeFromPointerToMember(clang_type member_type, clang_type obj_type)
  {
    return Cpp::GetFunctionTypeFromPointerToMember(member_type, obj_type);
  }

  std::vector<clang_decl> GetFunctionsUsingName(clang_decl scope, std::string const &name)
  {
    return from_voidp(Cpp::GetFunctionsUsingName(scope, name));
  }

  clang_decl GetGlobalScope()
  {
    return Cpp::GetGlobalScope();
  }

  clang_type GetLValueReferenceType(clang_type type)
  {
    return Cpp::GetLValueReferenceType(type);
  }

  clang_type GetLongType(clang_type type)
  {
    return Cpp::GetLongType(type);
  }

  std::string GetName(clang_decl klass)
  {
    return Cpp::GetName(klass);
  }

  std::string GetTruncatedName(clang_decl klass)
  {
    return Cpp::GetTruncatedName(klass);
  }

  clang_decl GetNamed(std::string const &name, clang_decl parent)
  {
    return Cpp::GetNamed(name, parent);
  }

  clang_type GetNonReferenceType(clang_type type)
  {
    return Cpp::GetNonReferenceType(type);
  }

  bool IsClass(clang_decl scope)
  {
    return Cpp::IsClass(scope);
  }

  size_t GetNumBases(clang_decl klass)
  {
    return Cpp::GetNumBases(klass);
  }

  void GetOperator(CppImpl::Operator op,
                   std::vector<clang_type> const &arg_types,
                   std::vector<clang_decl> &operators,
                   CppImpl::OperatorArity kind)
  {
    std::vector<void *> cpp_operators;
    /* TODO: Update GetOperator to take vector<type> */
    std::vector<Cpp::TemplateArgInfo> arg_type_infos;
    arg_type_infos.reserve(arg_types.size());
    for(auto const type : arg_types)
    {
      arg_type_infos.emplace_back(type);
    }
    Cpp::GetOperator(op, arg_type_infos, cpp_operators, kind);
    operators = from_voidp(cpp_operators);
  }

  CppImpl::OverloadCandidateInfo GetOverloadCandidateInfo(clang_decl candidate,
                                                          std::vector<clang_type> const &arg_types,
                                                          std::vector<clang_decl> const &arg_scopes)
  {
    auto const cpp_arg_scopes{ to_voidp(arg_scopes) };
    /* TODO: Update GetOverloadCandidateInfo to take vector<type> */
    std::vector<Cpp::TemplateArgInfo> arg_type_infos;
    arg_type_infos.reserve(arg_types.size());
    for(auto const type : arg_types)
    {
      arg_type_infos.emplace_back(type);
    }
    return Cpp::GetOverloadCandidateInfo(candidate, arg_type_infos, cpp_arg_scopes);
  }

  clang_decl GetParentScope(clang_decl scope)
  {
    return Cpp::GetParentScope(scope);
  }

  clang_type GetParentTypeFromPointerToMember(clang_type type)
  {
    return Cpp::GetParentTypeFromPointerToMember(type);
  }

  clang_type GetPointeeType(clang_type type)
  {
    return Cpp::GetPointeeType(type);
  }

  clang_type GetPointerToMemberType(clang_decl member)
  {
    return Cpp::GetPointerToMemberType(member);
  }

  clang_type GetPointerType(clang_type type)
  {
    return Cpp::GetPointerType(type);
  }

  std::string GetQualifiedCompleteName(clang_decl klass)
  {
    return Cpp::GetQualifiedCompleteName(klass);
  }

  std::string GetQualifiedCompleteNameWithTemplateArgs(clang_decl klass)
  {
    return Cpp::GetQualifiedCompleteNameWithTemplateArgs(klass);
  }

  std::string GetQualifiedName(clang_decl klass)
  {
    return Cpp::GetQualifiedName(klass);
  }

  clang_type GetRValueReferenceType(clang_type type)
  {
    return Cpp::GetRValueReferenceType(type);
  }

  clang_decl GetScopeFromCompleteName(std::string const &name)
  {
    return Cpp::GetScopeFromCompleteName(name);
  }

  clang_decl GetScopeFromType(clang_type type)
  {
    return Cpp::GetScopeFromType(type);
  }

  clang_type GetShortType(clang_type type)
  {
    return Cpp::GetShortType(type);
  }

  clang_type GetSignedType(clang_type type)
  {
    return Cpp::GetSignedType(type);
  }

  void GetStaticDatamembers(clang_decl scope, std::vector<clang_decl> &datamembers)
  {
    std::vector<void *> cpp_datamembers;
    Cpp::GetStaticDatamembers(scope, cpp_datamembers);
    datamembers = from_voidp(cpp_datamembers);
  }

  clang_type GetType(std::string const &type)
  {
    return Cpp::GetType(type);
  }

  std::string GetTypeAsString(clang_type type)
  {
    return Cpp::GetTypeAsString(type);
  }

  std::string GetTypeAsTruncatedString(clang_type type)
  {
    return Cpp::GetTypeAsTruncatedString(type);
  }

  clang_type GetTypeFromScope(clang_decl klass)
  {
    return Cpp::GetTypeFromScope(klass);
  }

  clang_type GetTypeWithConst(clang_type type)
  {
    return Cpp::GetTypeWithConst(type);
  }

  clang_type GetTypeWithVolatile(clang_type type)
  {
    return Cpp::GetTypeWithVolatile(type);
  }

  clang_type GetTypeWithoutCv(clang_type type)
  {
    return Cpp::GetTypeWithoutCv(type);
  }

  clang_decl GetUnderlyingScope(clang_decl scope)
  {
    return Cpp::GetUnderlyingScope(scope);
  }

  clang_type GetUnderlyingType(clang_type type)
  {
    return Cpp::GetUnderlyingType(type);
  }

  clang_type GetUnsignedType(clang_type type)
  {
    return Cpp::GetUnsignedType(type);
  }

  intptr_t GetVariableOffset(clang_decl var, clang_decl parent)
  {
    return Cpp::GetVariableOffset(var, parent);
  }

  clang_type GetVoidType()
  {
    return Cpp::GetVoidType();
  }

  bool HasDeletedMoveConstructor(clang_decl scope)
  {
    return Cpp::HasDeletedMoveConstructor(scope);
  }

  bool HasUsableCopyConstructor(clang_decl scope)
  {
    return Cpp::HasUsableCopyConstructor(scope);
  }

  bool HasUsableMoveConstructor(clang_decl scope)
  {
    return Cpp::HasUsableMoveConstructor(scope);
  }

  clang_decl InstantiateTemplate(clang_decl tmpl,
                                 CppImpl::TemplateArgInfo const *template_args,
                                 size_t template_args_size,
                                 bool instantiate_body)
  {
    return Cpp::InstantiateTemplate(tmpl, template_args, template_args_size, instantiate_body);
  }

  bool InstantiateTemplate(clang_decl spec)
  {
    return Cpp::InstantiateTemplate(spec);
  }

  bool IsArrayType(clang_type type)
  {
    return Cpp::IsArrayType(type);
  }

  bool IsBuiltin(clang_type type)
  {
    return Cpp::IsBuiltin(type);
  }

  bool IsCStyleConvertible(clang_type from_type, clang_type to_type)
  {
    return Cpp::IsCStyleConvertible(from_type, to_type);
  }

  bool IsClassTemplate(clang_decl handle)
  {
    return Cpp::IsClassTemplate(handle);
  }

  bool IsComplete(clang_decl scope)
  {
    return Cpp::IsComplete(scope);
  }

  bool IsConstMethod(clang_decl method)
  {
    return Cpp::IsConstMethod(method);
  }

  bool IsConstType(clang_type type)
  {
    return Cpp::IsConstType(type);
  }

  bool IsConstructible(clang_type to_type, clang_type from_type)
  {
    return Cpp::IsConstructible(to_type, from_type);
  }

  bool IsDestructor(clang_decl method)
  {
    return Cpp::IsDestructor(method);
  }

  bool IsEnumConstant(clang_decl handle)
  {
    return Cpp::IsEnumConstant(handle);
  }

  bool IsEnumScope(clang_decl handle)
  {
    return Cpp::IsEnumScope(handle);
  }

  bool IsEnumType(clang_type type)
  {
    return Cpp::IsEnumType(type);
  }

  bool IsFunction(clang_decl scope)
  {
    return Cpp::IsFunction(scope);
  }

  bool IsFunctionDeleted(clang_decl function)
  {
    return Cpp::IsFunctionDeleted(function);
  }

  bool IsFunctionPointerType(clang_type type)
  {
    return Cpp::IsFunctionPointerType(type);
  }

  bool IsFunctionTypeConst(clang_type function_type)
  {
    return Cpp::IsFunctionTypeConst(function_type);
  }

  bool IsFunctionVariadic(clang_decl function)
  {
    return Cpp::IsFunctionVariadic(function);
  }

  bool IsFunctionVariadicTemplate(clang_decl fn)
  {
    return Cpp::IsFunctionVariadicTemplate(fn);
  }

  bool IsImplicitlyConvertible(clang_type from_type, clang_type to_type)
  {
    return Cpp::IsImplicitlyConvertible(from_type, to_type);
  }

  bool IsInlineFriendFunction(clang_decl scope)
  {
    return Cpp::IsInlineFriendFunction(scope);
  }

  bool IsIntegral(clang_type type)
  {
    return Cpp::IsIntegral(type);
  }

  bool IsMethod(clang_decl method)
  {
    return Cpp::IsMethod(method);
  }

  bool IsNamespace(clang_decl scope)
  {
    return Cpp::IsNamespace(scope);
  }

  bool IsPointerToMemberFunctionType(clang_type type)
  {
    return Cpp::IsPointerToMemberFunctionType(type);
  }

  bool IsPointerToMemberType(clang_type type)
  {
    return Cpp::IsPointerToMemberType(type);
  }

  bool IsPointerToMemberVariableType(clang_type type)
  {
    return Cpp::IsPointerToMemberVariableType(type);
  }

  bool IsPointerType(clang_type type)
  {
    return Cpp::IsPointerType(type);
  }

  bool IsPrivateMethod(clang_decl method)
  {
    return Cpp::IsPrivateMethod(method);
  }

  bool IsConstructor(clang_decl method)
  {
    return Cpp::IsConstructor(method);
  }

  bool IsPrivateVariable(clang_decl var)
  {
    return Cpp::IsPrivateVariable(var);
  }

  bool IsProtectedMethod(clang_decl method)
  {
    return Cpp::IsProtectedMethod(method);
  }

  bool IsProtectedVariable(clang_decl var)
  {
    return Cpp::IsProtectedVariable(var);
  }

  bool IsReferenceType(clang_type type)
  {
    return Cpp::IsReferenceType(type);
  }

  bool IsRvalueReferenceType(clang_type type)
  {
    return Cpp::IsRvalueReferenceType(type);
  }

  bool IsShortType(clang_type type)
  {
    return Cpp::IsShortType(type);
  }

  bool IsStaticMethod(clang_decl method)
  {
    return Cpp::IsStaticMethod(method);
  }

  bool IsStaticVariable(clang_decl var)
  {
    return Cpp::IsStaticVariable(var);
  }

  bool IsTemplate(clang_decl handle)
  {
    return Cpp::IsTemplate(handle);
  }

  bool IsTemplateSpecialization(clang_decl handle)
  {
    return Cpp::IsTemplateSpecialization(handle);
  }

  bool IsTemplateSpecializationOf(clang_decl spec, clang_decl templ)
  {
    return Cpp::IsTemplateSpecializationOf(spec, templ);
  }

  bool IsTemplatedFunction(clang_decl func)
  {
    return Cpp::IsTemplatedFunction(func);
  }

  bool IsTriviallyDestructible(clang_type type)
  {
    return Cpp::IsTriviallyDestructible(type);
  }

  bool IsTypeDerivedFrom(clang_type derived, clang_type base)
  {
    return Cpp::IsTypeDerivedFrom(derived, base);
  }

  bool IsTypedefed(clang_decl handle)
  {
    return Cpp::IsTypedefed(handle);
  }

  bool IsVariable(clang_decl scope)
  {
    return Cpp::IsVariable(scope);
  }

  bool IsNonStaticVariable(clang_decl scope)
  {
    return Cpp::IsNonStaticVariable(scope);
  }

  bool IsVoid(clang_type type)
  {
    return Cpp::IsVoid(type);
  }

  void
  LookupConstructors(std::string const &name, clang_decl parent, std::vector<clang_decl> &funcs)
  {
    std::vector<void *> cpp_funcs;
    Cpp::LookupConstructors(name, parent, cpp_funcs);
    funcs = from_voidp(cpp_funcs);
  }

  clang_decl LookupDatamember(std::string const &name, clang_decl parent)
  {
    return Cpp::LookupDatamember(name, parent);
  }

  std::vector<clang_decl> LookupMethods(std::string const &name, clang_decl parent)
  {
    return from_voidp(Cpp::LookupMethods(name, parent));
  }

  std::string MangleRTTI(clang_type type)
  {
    return Cpp::MangleRTTI(type);
  }
}
