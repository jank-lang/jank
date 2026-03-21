#include <jank/ir/instruction.hpp>

namespace jank::ir
{
  bool instruction::is_terminator() const
  {
    return false;
  }

}

namespace jank::ir::inst
{
  literal::literal(identifier const &name, runtime::object_ref const value)
    : name{ name }
    , value{ value }
  {
  }

  def::def(identifier const &name,
           jtl::immutable_string const &qualified_var,
           jtl::option<identifier> const &value,
           identifier const &meta)
    : name{ name }
    , qualified_var{ qualified_var }
    , value{ value }
    , meta{ meta }
  {
  }

  var_deref::var_deref(identifier const &name, jtl::immutable_string const &qualified_var)
    : name{ name }
    , qualified_var{ qualified_var }
  {
  }

  var_ref::var_ref(identifier const &name, jtl::immutable_string const &qualified_var)
    : name{ name }
    , qualified_var{ qualified_var }
  {
  }

  dynamic_call::dynamic_call(identifier const &name,
                             identifier const &fn,
                             native_vector<identifier> &&args)
    : name{ name }
    , fn{ fn }
    , args{ jtl::move(args) }
  {
  }

  ret::ret(identifier const &value)
    : value{ value }
  {
  }

  bool ret::is_terminator() const
  {
    return true;
  }
}
