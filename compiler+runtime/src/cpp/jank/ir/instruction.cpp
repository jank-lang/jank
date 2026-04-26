#include <CppInterOp/CppInterOp.h>

#include <jank/ir/instruction.hpp>
#include <jank/analyze/cpp_util.hpp>
#include <jank/analyze/visit.hpp>

namespace jank::ir
{
  instruction::instruction(instruction_kind const kind,
                           identifier const &name,
                           jtl::ptr<void> const type)
    : kind{ kind }
    , name{ name }
    , type{ type }
  {
  }

  bool instruction::is_terminator() const
  {
    return false;
  }

}

namespace jank::ir::inst
{
  using namespace analyze::cpp_util;

  parameter::parameter(identifier const &name, jtl::ptr<void> const type)
    : instruction{ instruction_kind::parameter, name, type }
  {
  }

  capture::capture(identifier const &name,
                   jtl::ptr<void> const type,
                   jtl::immutable_string const &value)
    : instruction{ instruction_kind::capture, name, type }
    , value{ value }
  {
  }

  literal::literal(identifier const &name,
                   jtl::ptr<void> const type,
                   runtime::object_ref const obj,
                   identifier const &value)
    : instruction{ instruction_kind::literal, name, type }
    , obj{ obj }
    , value{ value }
  {
  }

  persistent_list::persistent_list(identifier const &name,
                                   native_vector<identifier> &&values,
                                   jtl::option<identifier> const &meta)
    : instruction{ instruction_kind::persistent_list, name, persistent_list_ref_type() }
    , values{ jtl::move(values) }
    , meta{ meta }
  {
  }

  persistent_vector::persistent_vector(identifier const &name,
                                       native_vector<identifier> &&values,
                                       jtl::option<identifier> const &meta)
    : instruction{ instruction_kind::persistent_vector, name, persistent_vector_ref_type() }
    , values{ jtl::move(values) }
    , meta{ meta }
  {
  }

  persistent_array_map::persistent_array_map(
    identifier const &name,
    native_vector<std::pair<identifier, identifier>> &&values,
    jtl::option<identifier> const &meta)
    : instruction{ instruction_kind::persistent_array_map, name, persistent_array_map_ref_type() }
    , values{ jtl::move(values) }
    , meta{ meta }
  {
  }

  persistent_hash_map::persistent_hash_map(
    identifier const &name,
    native_vector<std::pair<identifier, identifier>> &&values,
    jtl::option<identifier> const &meta)
    : instruction{ instruction_kind::persistent_hash_map, name, persistent_hash_map_ref_type() }
    , values{ jtl::move(values) }
    , meta{ meta }
  {
  }

  persistent_hash_set::persistent_hash_set(identifier const &name,
                                           native_vector<identifier> &&values,
                                           jtl::option<identifier> const &meta)
    : instruction{ instruction_kind::persistent_hash_set, name, persistent_hash_set_ref_type() }
    , values{ jtl::move(values) }
    , meta{ meta }
  {
  }

  function::function(identifier const &name,
                     native_unordered_map<u8, jtl::immutable_string> &&arities,
                     runtime::callable_arity_flags const arity_flags)
    : instruction{ instruction_kind::function, name, jit_function_ref_type() }
    , arities{ jtl::move(arities) }
    , arity_flags{ arity_flags }
  {
  }

  closure::closure(identifier const &name,
                   jtl::immutable_string const &context,
                   native_unordered_map<u8, jtl::immutable_string> &&arities,
                   native_unordered_map<jtl::immutable_string, detail::typed_identifier> &&captures,
                   runtime::callable_arity_flags const arity_flags)
    : instruction{ instruction_kind::closure, name, jit_closure_ref_type() }
    , context{ context }
    , arities{ jtl::move(arities) }
    , captures{ jtl::move(captures) }
    , arity_flags{ arity_flags }
  {
  }

  letfn::letfn(identifier const &name, native_vector<jtl::immutable_string> &&bindings)
    : instruction{ instruction_kind::letfn, name, Cpp::GetVoidType() }
    , bindings{ jtl::move(bindings) }
  {
  }

  def::def(identifier const &name,
           jtl::ptr<void> const type,
           jtl::immutable_string const &qualified_var,
           jtl::option<identifier> const &value,
           identifier const &meta,
           bool const is_dynamic)
    : instruction{ instruction_kind::def, name, type }
    , qualified_var{ qualified_var }
    , value{ value }
    , meta{ meta }
    , is_dynamic{ is_dynamic }
  {
  }

  var_deref::var_deref(identifier const &name, identifier const &var)
    : instruction{ instruction_kind::var_deref, name, untyped_object_ref_type() }
    , var{ var }
  {
  }

  var_ref::var_ref(identifier const &name, jtl::ptr<void> const type, identifier const &var)
    : instruction{ instruction_kind::var_ref, name, type }
    , var{ var }
  {
  }

  type_erase::type_erase(identifier const &name, identifier const &value)
    : instruction{ instruction_kind::type_erase, name, untyped_object_ref_type() }
    , value{ value }
  {
  }

  dynamic_call::dynamic_call(identifier const &name,
                             jtl::ptr<void> const type,
                             identifier const &fn,
                             native_vector<identifier> &&args)
    : instruction{ instruction_kind::dynamic_call, name, type }
    , fn{ fn }
    , args{ jtl::move(args) }
  {
  }

  named_recursion::named_recursion(identifier const &name,
                                   jtl::ptr<void> const type,
                                   identifier const &fn,
                                   jtl::immutable_string const &fn_base_name,
                                   native_vector<identifier> &&args,
                                   bool const needs_dynamic_call)
    : instruction{ instruction_kind::named_recursion, name, type }
    , fn{ fn }
    , fn_base_name{ fn_base_name }
    , args{ jtl::move(args) }
    , needs_dynamic_call{ needs_dynamic_call }
  {
  }

  recursion_reference::recursion_reference(identifier const &name, jtl::ptr<void> const type)
    : instruction{ instruction_kind::recursion_reference, name, type }
  {
  }

  truthy::truthy(identifier const &name, identifier const &value)
    : instruction{ instruction_kind::truthy, name, bool_type() }
    , value{ value }
  {
  }

  jump::jump(identifier const &name, identifier const &block)
    : instruction{ instruction_kind::jump, name, Cpp::GetVoidType() }
    , block{ block }
  {
  }

  jump::jump(identifier const &name, identifier const &block, bool const loop)
    : instruction{ instruction_kind::jump, name, Cpp::GetVoidType() }
    , block{ block }
    , loop{ loop }
  {
  }

  bool jump::is_terminator() const
  {
    return true;
  }

  branch_set::branch_set(identifier const &name, identifier const &shadow, identifier const &value)
    : instruction{ instruction_kind::branch_set, name, Cpp::GetVoidType() }
    , shadow{ shadow }
    , value{ value }
  {
  }

  branch_get::branch_get(identifier const &name, jtl::ptr<void> const type)
    : instruction{ instruction_kind::branch_get, name, type }
  {
  }

  branch::branch(identifier const &name,
                 identifier const &condition,
                 identifier const &then_block,
                 identifier const &else_block,
                 jtl::option<identifier> const &merge_block,
                 jtl::option<detail::typed_identifier> const &shadow)
    : instruction{ instruction_kind::branch, name, Cpp::GetVoidType() }
    , condition{ condition }
    , then_block{ then_block }
    , else_block{ else_block }
    , merge_block{ merge_block }
    , shadow{ shadow }
  {
  }

  bool branch::is_terminator() const
  {
    return true;
  }

  loop::loop(identifier const &name,
             identifier const &loop_block,
             jtl::option<identifier> const &merge_block,
             jtl::option<detail::typed_identifier> const &shadow,
             native_vector<loop::binding_shadow_details> &&binding_shadows)
    : instruction{ instruction_kind::loop, name, Cpp::GetVoidType() }
    , loop_block{ loop_block }
    , merge_block{ merge_block }
    , shadow{ shadow }
    , binding_shadows{ jtl::move(binding_shadows) }
  {
  }

  bool loop::is_terminator() const
  {
    return true;
  }

  case_::case_(identifier const &name,
               i64 const shift,
               i64 const mask,
               identifier const &value,
               native_unordered_map<i64, identifier> &&case_blocks,
               identifier const &default_block,
               jtl::option<identifier> const &merge_block,
               jtl::option<identifier> const &shadow)
    : instruction{ instruction_kind::case_, name, Cpp::GetVoidType() }
    , shift{ shift }
    , mask{ mask }
    , value{ value }
    , case_blocks{ jtl::move(case_blocks) }
    , default_block{ default_block }
    , merge_block{ merge_block }
    , shadow{ shadow }
  {
  }

  bool case_::is_terminator() const
  {
    return true;
  }

  try_::try_(identifier const &name,
             native_vector<std::pair<jtl::ptr<void>, identifier>> &&catches,
             identifier const &merge_block,
             identifier const &shadow,
             jtl::option<identifier> const &finally_block)
    : instruction{ instruction_kind::try_, name, Cpp::GetVoidType() }
    , catches{ jtl::move(catches) }
    , merge_block{ merge_block }
    , shadow{ shadow }
    , finally_block{ finally_block }
  {
  }

  catch_::catch_(identifier const &name,
                 jtl::ptr<void> const type,
                 jtl::option<identifier> const &merge_block,
                 jtl::option<identifier> const &shadow,
                 jtl::option<identifier> const &finally_block)
    : instruction{ instruction_kind::catch_, name, type }
    , merge_block{ merge_block }
    , shadow{ shadow }
    , finally_block{ finally_block }
  {
  }

  finally::finally(identifier const &name, identifier const &merge_block)
    : instruction{ instruction_kind::finally, name, Cpp::GetVoidType() }
    , merge_block{ merge_block }
  {
  }

  throw_::throw_(identifier const &name, identifier const &value)
    : instruction{ instruction_kind::throw_, name, Cpp::GetVoidType() }
    , value{ value }
  {
  }

  bool throw_::is_terminator() const
  {
    return true;
  }

  ret::ret(identifier const &name, jtl::ptr<void> const type, identifier const &value)
    : instruction{ instruction_kind::ret, name, type }
    , value{ value }
  {
  }

  bool ret::is_terminator() const
  {
    return true;
  }

  cpp_raw::cpp_raw(identifier const &name, analyze::expr::cpp_raw_ref const expr)
    : instruction{ instruction_kind::cpp_raw, name, expr->get_type() }
    , expr{ expr }
  {
  }

  cpp_value::cpp_value(identifier const &name, analyze::expr::cpp_value_ref const expr)
    : instruction{ instruction_kind::cpp_value, name, expression_type(expr) }
    , expr{ expr }
  {
  }

  cpp_into_object::cpp_into_object(identifier const &name,
                                   identifier const &value,
                                   analyze::expr::cpp_conversion_ref const expr)
    : instruction{ instruction_kind::cpp_into_object, name, expr->type }
    , value{ value }
    , expr{ expr }
  {
  }

  cpp_from_object::cpp_from_object(identifier const &name,
                                   identifier const &value,
                                   analyze::expr::cpp_conversion_ref const expr)
    : instruction{ instruction_kind::cpp_from_object, name, expr->type }
    , value{ value }
    , expr{ expr }
  {
  }

  cpp_unsafe_cast::cpp_unsafe_cast(identifier const &name,
                                   identifier const &value,
                                   analyze::expr::cpp_unsafe_cast_ref const expr)
    : instruction{ instruction_kind::cpp_unsafe_cast, name, expression_type(expr) }
    , value{ value }
    , expr{ expr }
  {
  }

  cpp_call::cpp_call(identifier const &name,
                     jtl::option<identifier> const &value,
                     native_vector<identifier> &&args,
                     analyze::expr::cpp_call_ref const expr)
    : instruction{ instruction_kind::cpp_call, name, expression_type(expr) }
    , value{ value }
    , args{ jtl::move(args) }
    , expr{ expr }
  {
  }

  cpp_constructor_call::cpp_constructor_call(identifier const &name,
                                             native_vector<identifier> &&args,
                                             analyze::expr::cpp_constructor_call_ref const expr)
    : instruction{ instruction_kind::cpp_constructor_call, name, expression_type(expr) }
    , args{ jtl::move(args) }
    , expr{ expr }
  {
  }

  cpp_member_call::cpp_member_call(identifier const &name,
                                   native_vector<identifier> &&args,
                                   analyze::expr::cpp_member_call_ref const expr)
    : instruction{ instruction_kind::cpp_member_call, name, expression_type(expr) }
    , args{ jtl::move(args) }
    , expr{ expr }
  {
  }

  cpp_member_access::cpp_member_access(identifier const &name,
                                       identifier const &value,
                                       analyze::expr::cpp_member_access_ref const expr)
    : instruction{ instruction_kind::cpp_member_access, name, expression_type(expr) }
    , value{ value }
    , expr{ expr }
  {
  }

  cpp_builtin_operator_call::cpp_builtin_operator_call(
    identifier const &name,
    native_vector<identifier> &&args,
    analyze::expr::cpp_builtin_operator_call_ref const expr)
    : instruction{ instruction_kind::cpp_builtin_operator_call, name, expression_type(expr) }
    , args{ jtl::move(args) }
    , expr{ expr }
  {
  }

  cpp_box::cpp_box(identifier const &name,
                   identifier const &value,
                   analyze::expr::cpp_box_ref const expr)
    : instruction{ instruction_kind::cpp_box, name, expression_type(expr) }
    , value{ value }
    , expr{ expr }
  {
  }

  cpp_unbox::cpp_unbox(identifier const &name,
                       identifier const &value,
                       identifier const &meta,
                       analyze::expr::cpp_unbox_ref const expr)
    : instruction{ instruction_kind::cpp_unbox, name, expression_type(expr) }
    , value{ value }
    , meta{ meta }
    , expr{ expr }
  {
  }

  cpp_new::cpp_new(identifier const &name,
                   identifier const &value,
                   analyze::expr::cpp_new_ref const expr)
    : instruction{ instruction_kind::cpp_new, name, expression_type(expr) }
    , value{ value }
    , expr{ expr }
  {
  }

  cpp_delete::cpp_delete(identifier const &name,
                         identifier const &value,
                         analyze::expr::cpp_delete_ref const expr)
    : instruction{ instruction_kind::cpp_delete, name, expression_type(expr) }
    , value{ value }
    , expr{ expr }
  {
  }
}
