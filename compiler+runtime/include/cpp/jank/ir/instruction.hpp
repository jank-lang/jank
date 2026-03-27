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
    struct parameter : instruction
    {
      parameter(identifier const &name,
                jtl::ptr<void> const type,
                jtl::immutable_string const &value);

      void print(jtl::string_builder &sb, usize indent) const override;

      jtl::immutable_string value;
    };

    struct capture : instruction
    {
      capture(identifier const &name,
              jtl::ptr<void> const type,
              jtl::immutable_string const &value);

      void print(jtl::string_builder &sb, usize indent) const override;

      jtl::immutable_string const &value;
    };

    struct literal : instruction
    {
      literal(identifier const &name, jtl::ptr<void> const type, runtime::object_ref const value);

      void print(jtl::string_builder &sb, usize indent) const override;

      runtime::object_ref value;
    };

    struct persistent_list : instruction
    {
      persistent_list(identifier const &name, native_vector<identifier> &&values);

      void print(jtl::string_builder &sb, usize indent) const override;

      native_vector<identifier> values;
    };

    struct persistent_vector : instruction
    {
      persistent_vector(identifier const &name, native_vector<identifier> &&values);

      void print(jtl::string_builder &sb, usize indent) const override;

      native_vector<identifier> values;
    };

    struct persistent_array_map : instruction
    {
      persistent_array_map(identifier const &name,
                           native_vector<std::pair<identifier, identifier>> &&values);

      void print(jtl::string_builder &sb, usize indent) const override;

      native_vector<std::pair<identifier, identifier>> values;
    };

    struct persistent_hash_map : instruction
    {
      persistent_hash_map(identifier const &name,
                          native_vector<std::pair<identifier, identifier>> &&values);

      void print(jtl::string_builder &sb, usize indent) const override;

      native_vector<std::pair<identifier, identifier>> values;
    };

    struct persistent_hash_set : instruction
    {
      persistent_hash_set(identifier const &name, native_vector<identifier> &&values);

      void print(jtl::string_builder &sb, usize indent) const override;

      native_vector<identifier> values;
    };

    struct function : instruction
    {
      function(identifier const &name, native_unordered_map<u8, jtl::immutable_string> &&arities);

      void print(jtl::string_builder &sb, usize indent) const override;

      native_unordered_map<u8, jtl::immutable_string> arities;
    };

    struct closure : instruction
    {
      closure(identifier const &name,
              native_unordered_map<u8, jtl::immutable_string> &&arities,
              native_unordered_map<jtl::immutable_string, identifier> &&captures);

      void print(jtl::string_builder &sb, usize indent) const override;

      native_unordered_map<u8, jtl::immutable_string> arities;
      native_unordered_map<jtl::immutable_string, identifier> captures;
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

    struct named_recursion : instruction
    {
      named_recursion(identifier const &name,
                      jtl::ptr<void> const type,
                      native_vector<identifier> &&args);

      void print(jtl::string_builder &sb, usize indent) const override;

      native_vector<identifier> args;
    };

    struct recursion_reference : instruction
    {
      recursion_reference(identifier const &name, jtl::ptr<void> const type);

      void print(jtl::string_builder &sb, usize indent) const override;
    };

    struct truthy : instruction
    {
      truthy(identifier const &name, identifier const &value);

      void print(jtl::string_builder &sb, usize indent) const override;

      identifier value;
    };

    struct jump : instruction
    {
      jump(identifier const &name, identifier const &block);

      bool is_terminator() const override;
      void print(jtl::string_builder &sb, usize indent) const override;

      identifier block;
    };

    struct branch_set : instruction
    {
      branch_set(identifier const &name, identifier const &shadow, identifier const &value);

      void print(jtl::string_builder &sb, usize indent) const override;

      identifier shadow;
      identifier value;
    };

    struct branch_get : instruction
    {
      branch_get(identifier const &name, jtl::ptr<void> const type);

      void print(jtl::string_builder &sb, usize indent) const override;
    };

    struct branch : instruction
    {
      branch(identifier const &name,
             identifier const &condition,
             identifier const &then_block,
             identifier const &else_block);

      bool is_terminator() const override;
      void print(jtl::string_builder &sb, usize indent) const override;

      identifier condition;
      identifier then_block;
      identifier else_block;
    };

    struct case_ : instruction
    {
      case_(identifier const &name,
            identifier const &value,
            native_unordered_map<i64, identifier> &&case_blocks,
            identifier const &default_block);

      bool is_terminator() const override;
      void print(jtl::string_builder &sb, usize indent) const override;

      identifier value;
      native_unordered_map<i64, identifier> case_blocks;
      identifier default_block;
    };

    struct try_ : instruction
    {
      try_(identifier const &name, native_vector<std::pair<jtl::ptr<void>, identifier>> &&catches);

      void print(jtl::string_builder &sb, usize indent) const override;

      native_vector<std::pair<jtl::ptr<void>, identifier>> catches;
    };

    struct catch_ : instruction
    {
      catch_(identifier const &name, jtl::ptr<void> const type);

      void print(jtl::string_builder &sb, usize indent) const override;
    };

    struct throw_ : instruction
    {
      throw_(identifier const &name, identifier const &value);

      bool is_terminator() const override;
      void print(jtl::string_builder &sb, usize indent) const override;

      identifier value;
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
