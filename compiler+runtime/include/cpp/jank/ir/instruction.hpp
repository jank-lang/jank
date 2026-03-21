#pragma once

#include <jtl/ref.hpp>
#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::ir
{
  using identifier = jtl::immutable_string;

  struct instruction
  {
    instruction(identifier const &name, jtl::ptr<void> const type);
    virtual ~instruction() = default;

    virtual bool is_terminator() const;
    virtual void print(jtl::string_builder &sb, usize indent) const = 0;

    identifier name;
    jtl::ptr<void> type;
  };

  namespace inst
  {
    struct literal : instruction
    {
      literal(identifier const &name, jtl::ptr<void> const type, runtime::object_ref const value);

      void print(jtl::string_builder &sb, usize indent) const override;

      runtime::object_ref value;
    };

    struct def : instruction
    {
      def(identifier const &name,
          jtl::ptr<void> const type,
          jtl::immutable_string const &qualified_var,
          jtl::option<identifier> const &value,
          identifier const &meta);

      void print(jtl::string_builder &sb, usize indent) const override;

      jtl::immutable_string qualified_var;
      jtl::option<identifier> value;
      identifier meta;
    };

    struct var_deref : instruction
    {
      var_deref(identifier const &name,
                jtl::ptr<void> const type,
                jtl::immutable_string const &qualified_var);

      void print(jtl::string_builder &sb, usize indent) const override;

      jtl::immutable_string qualified_var;
    };

    struct var_ref : instruction
    {
      var_ref(identifier const &name,
              jtl::ptr<void> const type,
              jtl::immutable_string const &qualified_var);

      void print(jtl::string_builder &sb, usize indent) const override;

      jtl::immutable_string qualified_var;
    };

    struct dynamic_call : instruction
    {
      dynamic_call(identifier const &name,
                   jtl::ptr<void> const type,
                   identifier const &fn,
                   native_vector<identifier> &&args);

      void print(jtl::string_builder &sb, usize indent) const override;

      identifier fn;
      native_vector<identifier> args;
    };

    struct ret : instruction
    {
      ret(identifier const &name, jtl::ptr<void> const type, identifier const &value);

      bool is_terminator() const override;
      void print(jtl::string_builder &sb, usize indent) const override;

      identifier value;
    };
  }
}
