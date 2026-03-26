#pragma once

#include <jtl/ref.hpp>
#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>
#include <jank/analyze/expression.hpp>
#include <jank/ir/processor.hpp>

namespace jtl
{
  struct immutable_string;
  struct string_builder;
}

namespace jank::ir
{
  struct builder
  {
    identifier next_ident();
    identifier next_ident(jtl::immutable_string const &prefix);
    identifier next_shadow();

    jtl::ref<function> current_function() const;

    jtl::ref<block> current_block() const;
    usize block(identifier const &name) const;
    identifier block_name(usize const block_index) const;
    void remove_block(usize const block_index) const;
    void enter_block(usize const blk_index);

    identifier
    parameter(analyze::expression_position const pos, jtl::immutable_string const &value);
    identifier capture(analyze::expression_position const pos,
                       jtl::ptr<void> const type,
                       jtl::immutable_string const &value);
    identifier literal(analyze::expression_position const pos, runtime::object_ref const value);
    identifier
    persistent_list(analyze::expression_position const pos, native_vector<identifier> &&values);
    identifier
    persistent_vector(analyze::expression_position const pos, native_vector<identifier> &&values);
    identifier persistent_array_map(analyze::expression_position const pos,
                                    native_vector<std::pair<identifier, identifier>> &&values);
    identifier persistent_hash_map(analyze::expression_position const pos,
                                   native_vector<std::pair<identifier, identifier>> &&values);
    identifier
    persistent_hash_set(analyze::expression_position const pos, native_vector<identifier> &&values);
    identifier def(analyze::expression_position const pos,
                   jtl::immutable_string const &qualified_var,
                   jtl::option<identifier> const &value,
                   identifier const &meta);
    identifier
    var_deref(analyze::expression_position const pos, jtl::immutable_string const &qualified_var);
    identifier
    var_ref(analyze::expression_position const pos, jtl::immutable_string const &qualified_var);
    identifier dynamic_call(analyze::expression_position const pos,
                            identifier const &fn,
                            native_vector<identifier> &&args);
    identifier
    named_recursion(analyze::expression_position const pos, native_vector<identifier> &&args);
    identifier recursion_reference(analyze::expression_position const pos);
    identifier truthy(identifier const &value);
    identifier jump(usize const index);
    identifier branch_set(identifier const &shadow, identifier const &value);
    identifier branch_get(identifier const &name, jtl::ptr<void> const type) const;
    identifier
    branch(identifier const &condition, identifier const &then_blk, identifier const &else_blk);
    identifier case_(identifier const &value,
                     native_unordered_map<i64, identifier> &&cases,
                     identifier const &default_block);
    identifier try_(native_vector<std::pair<jtl::ptr<void>, identifier>> &&catches);
    identifier catch_(jtl::ptr<void> const type);
    identifier throw_(identifier const &value);
    identifier ret(identifier const &value, jtl::ptr<void> const type);

    jtl::ref<module> mod;
    usize fn_index{};
    usize block_index{};
    usize ident_count{};
    native_unordered_map<jtl::immutable_string, identifier> locals{};
    native_unordered_map<jtl::immutable_string, identifier> local_to_loop_shadow{};
    jtl::option<usize> loop_recur_target{};
    jtl::option<usize> fn_recur_target{};
  };
}
