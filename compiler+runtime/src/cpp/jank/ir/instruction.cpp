#include <jank/ir/instruction.hpp>

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
