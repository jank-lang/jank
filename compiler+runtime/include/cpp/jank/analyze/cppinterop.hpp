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

  std::string MangleRTTI(clang_type type);
  void EnableDebugOutput(bool value = true);
  bool IsNamespace(clang_decl scope);
  bool IsClass(clang_decl scope);
  bool IsClassTemplate(clang_decl handle);
  bool IsFunction(clang_decl scope);
  bool IsFunctionPointerType(clang_type type);
  bool IsInlineFriendFunction(clang_decl scope);
  bool IsComplete(clang_decl scope);
  bool IsBuiltin(clang_type type);
  bool IsIntegral(clang_type type);
  bool IsVoid(clang_type type);
  clang_type GetVoidType();
  bool IsTemplate(clang_decl handle);
  bool IsTemplateSpecialization(clang_decl handle);
  bool IsTemplateSpecializationOf(clang_decl spec, clang_decl templ);
  bool IsTypedefed(clang_decl handle);
  clang_type GetCommonType(clang_type lhs, clang_type rhs);
  bool IsImplicitlyConvertible(clang_type from_type, clang_type to_type);
  bool IsCStyleConvertible(clang_type from_type, clang_type to_type);
  bool IsConstructible(clang_type to_type, clang_type from_type);
  bool IsTriviallyDestructible(clang_type type);
  bool IsEnumScope(clang_decl handle);
  bool IsEnumConstant(clang_decl handle);
  bool IsEnumType(clang_type type);
  bool IsVariable(clang_decl scope);
  std::string GetName(clang_decl klass);
  std::string GetQualifiedName(clang_decl klass);
  std::string GetQualifiedCompleteName(clang_decl klass);
  std::string GetTruncatedName(clang_decl klass);
  std::string GetQualifiedCompleteNameWithTemplateArgs(clang_decl klass);
  clang_decl GetGlobalScope();
  clang_decl GetUnderlyingScope(clang_decl scope);
  clang_decl GetScopeFromCompleteName(std::string const &name);
  clang_decl GetNamed(std::string const &name, clang_decl parent = nullptr);
  clang_decl GetParentScope(clang_decl scope);
  clang_decl GetScopeFromType(clang_type type);
  size_t GetNumBases(clang_decl klass);
  clang_decl GetBaseClass(clang_decl klass, size_t ibase);
  bool HasUsableCopyConstructor(clang_decl scope);
  bool HasUsableMoveConstructor(clang_decl scope);
  bool HasDeletedMoveConstructor(clang_decl scope);
  std::vector<clang_decl> GetFunctionsUsingName(clang_decl scope, std::string const &name);
  clang_type GetFunctionReturnType(clang_decl func);
  size_t GetFunctionNumArgs(clang_decl func);
  size_t GetFunctionRequiredArgs(clang_decl func);
  clang_type GetFunctionArgType(clang_decl func, size_t iarg);
  std::string GetFunctionSignature(clang_type func);
  std::string GetFunctionSourceInfo(clang_decl func);
  clang_type GetFunctionReturnTypeFromType(clang_type func);
  size_t GetFunctionNumArgsFromType(clang_type func);
  clang_type GetFunctionArgTypeFromType(clang_type func, size_t iarg);
  bool IsFunctionVariadic(clang_decl function);
  bool IsFunctionVariadicTemplate(clang_decl fn);
  bool IsFunctionDeleted(clang_decl function);
  bool IsFunctionTypeConst(clang_type function_type);
  bool IsTemplatedFunction(clang_decl func);
  void
  LookupConstructors(std::string const &name, clang_decl parent, std::vector<clang_decl> &funcs);
  bool IsMethod(clang_decl method);
  bool IsProtectedMethod(clang_decl method);
  bool IsPrivateMethod(clang_decl method);
  bool IsConstructor(clang_decl method);
  bool IsDestructor(clang_decl method);
  bool IsStaticMethod(clang_decl method);
  void GetDatamembers(clang_decl scope, std::vector<clang_decl> &datamembers);
  void GetStaticDatamembers(clang_decl scope, std::vector<clang_decl> &datamembers);
  clang_decl LookupDatamember(std::string const &name, clang_decl parent);
  std::vector<clang_decl> LookupMethods(std::string const &name, clang_decl parent);
  intptr_t GetVariableOffset(clang_decl var, clang_decl parent = nullptr);
  bool IsProtectedVariable(clang_decl var);
  bool IsPrivateVariable(clang_decl var);
  bool IsStaticVariable(clang_decl var);
  bool IsNonStaticVariable(clang_decl scope);
  bool IsConstType(clang_type type);
  bool IsPointerType(clang_type type);
  bool IsPointerToMemberType(clang_type type);
  bool IsPointerToMemberVariableType(clang_type type);
  bool IsPointerToMemberFunctionType(clang_type type);
  clang_type GetParentTypeFromPointerToMember(clang_type type);
  clang_type GetFunctionTypeFromPointerToMember(clang_type member_type, clang_type obj_type);
  bool IsArrayType(clang_type type);
  size_t GetArraySize(clang_type type);
  clang_type GetArrayElementType(clang_type type);
  clang_type GetArrayType(clang_type type);
  clang_type GetArrayType(clang_type type, size_t size);
  clang_type GetFunctionType(clang_type ret, std::vector<clang_type> const &params);
  clang_type GetPointeeType(clang_type type);
  clang_type GetPointerType(clang_type type);
  clang_type GetPointerToMemberType(clang_decl member);
  clang_type GetLValueReferenceType(clang_type type);
  clang_type GetRValueReferenceType(clang_type type);
  bool IsReferenceType(clang_type type);
  bool IsRvalueReferenceType(clang_type type);
  clang_type GetNonReferenceType(clang_type type);
  clang_type GetUnderlyingType(clang_type type);
  clang_type GetTypeWithoutCv(clang_type type);
  clang_type GetTypeWithConst(clang_type type);
  clang_type GetTypeWithVolatile(clang_type type);
  clang_type GetSignedType(clang_type type);
  clang_type GetUnsignedType(clang_type type);
  clang_type GetShortType(clang_type type);
  clang_type GetLongType(clang_type type);
  bool IsShortType(clang_type type);
  std::string GetTypeAsString(clang_type type);
  std::string GetTypeAsTruncatedString(clang_type type);
  clang_type GetCanonicalType(clang_type type);
  clang_type GetType(std::string const &type);
  clang_type GetTypeFromScope(clang_decl klass);
  bool IsTypeDerivedFrom(clang_type derived, clang_type base);
  bool IsConstMethod(clang_decl method);
  std::string GetFunctionArgName(clang_decl func, size_t param_index);
  void GetOperator(CppImpl::Operator op,
                   std::vector<clang_type> const &arg_types,
                   std::vector<clang_decl> &operators,
                   CppImpl::OperatorArity kind);
  clang_decl CreateInterpreter(std::vector<char const *> const &args = {},
                               std::vector<char const *> const &gpu_args = {},
                               std::map<char const *, std::string_view> const &vfs = {},
                               std::optional<int> const &cm = std::nullopt,
                               bool *pch_out_of_date = nullptr);
  std::string DetectResourceDir(char const *clang_binary_name = "clang");
  clang_decl InstantiateTemplate(clang_decl tmpl,
                                 CppImpl::TemplateArgInfo const *template_args,
                                 size_t template_args_size,
                                 bool instantiate_body = false);
  bool InstantiateTemplate(clang_decl spec);
  std::vector<clang_decl> BestOverloadMatch(std::vector<clang_decl> const &candidates,
                                            std::vector<clang_type> const &arg_types,
                                            std::vector<clang_decl> const &arg_scopes);
  CppImpl::OverloadCandidateInfo
  GetOverloadCandidateInfo(clang_decl candidate,
                           std::vector<clang_type> const &arg_types,
                           std::vector<clang_decl> const &arg_scopes);
}
