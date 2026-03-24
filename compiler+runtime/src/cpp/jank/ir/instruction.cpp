#include <CppInterOp/CppInterOp.h>

#include <jank/ir/instruction.hpp>
#include <jank/analyze/cpp_util.hpp>

namespace jank::ir
{
  instruction::instruction(identifier const &name, jtl::ptr<void> const type)
    : name{ name }
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

  parameter::parameter(identifier const &name, jtl::ptr<void> const type, u8 const index)
    : instruction{ name, type }
    , index{ index }
  {
  }

  literal::literal(identifier const &name,
                   jtl::ptr<void> const type,
                   runtime::object_ref const value)
    : instruction{ name, type }
    , value{ value }
  {
  }

  def::def(identifier const &name,
           jtl::ptr<void> const type,
           jtl::immutable_string const &qualified_var,
           jtl::option<identifier> const &value,
           identifier const &meta)
    : instruction{ name, type }
    , qualified_var{ qualified_var }
    , value{ value }
    , meta{ meta }
  {
  }

  var_deref::var_deref(identifier const &name,
                       jtl::ptr<void> const type,
                       jtl::immutable_string const &qualified_var)
    : instruction{ name, type }
    , qualified_var{ qualified_var }
  {
  }

  var_ref::var_ref(identifier const &name,
                   jtl::ptr<void> const type,
                   jtl::immutable_string const &qualified_var)
    : instruction{ name, type }
    , qualified_var{ qualified_var }
  {
  }

  dynamic_call::dynamic_call(identifier const &name,
                             jtl::ptr<void> const type,
                             identifier const &fn,
                             native_vector<identifier> &&args)
    : instruction{ name, type }
    , fn{ fn }
    , args{ jtl::move(args) }
  {
  }

  named_recursion::named_recursion(identifier const &name,
                                   jtl::ptr<void> const type,
                                   native_vector<identifier> &&args)
    : instruction{ name, type }
    , args{ jtl::move(args) }
  {
  }

  recursion_reference::recursion_reference(identifier const &name, jtl::ptr<void> const type)
    : instruction{ name, type }
  {
  }

  truthy::truthy(identifier const &name, identifier const &value)
    : instruction{ name, bool_type() }
    , value{ value }
  {
  }

  jump::jump(identifier const &name, identifier const &block)
    : instruction{ name, Cpp::GetVoidType() }
    , block{ block }
  {
  }

  bool jump::is_terminator() const
  {
    return true;
  }

  branch_set::branch_set(identifier const &name, identifier const &shadow, identifier const &value)
    : instruction{ name, Cpp::GetVoidType() }
    , shadow{ shadow }
    , value{ value }
  {
  }

  branch_get::branch_get(identifier const &name, jtl::ptr<void> const type)
    : instruction{ name, type }
  {
  }

  branch::branch(identifier const &name,
                 identifier const &condition,
                 identifier const &then_block,
                 identifier const &else_block)
    : instruction{ name, Cpp::GetVoidType() }
    , condition{ condition }
    , then_block{ then_block }
    , else_block{ else_block }
  {
  }

  bool branch::is_terminator() const
  {
    return true;
  }

  throw_::throw_(identifier const &name, identifier const &value)
    : instruction{ name, Cpp::GetVoidType() }
    , value{ value }
  {
  }

  bool throw_::is_terminator() const
  {
    return true;
  }

  ret::ret(identifier const &name, jtl::ptr<void> const type, identifier const &value)
    : instruction{ name, type }
    , value{ value }
  {
  }

  bool ret::is_terminator() const
  {
    return true;
  }
}
