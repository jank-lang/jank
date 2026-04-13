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

namespace jank::analyze::expr
{
  using cpp_raw_ref = jtl::ref<struct cpp_raw>;
  using cpp_value_ref = jtl::ref<struct cpp_value>;
  using cpp_conversion_ref = jtl::ref<struct cpp_conversion>;
  using cpp_unsafe_cast_ref = jtl::ref<struct cpp_unsafe_cast>;
  using cpp_call_ref = jtl::ref<struct cpp_call>;
  using cpp_constructor_call_ref = jtl::ref<struct cpp_constructor_call>;
  using cpp_member_call_ref = jtl::ref<struct cpp_member_call>;
  using cpp_member_access_ref = jtl::ref<struct cpp_member_access>;
  using cpp_builtin_operator_call_ref = jtl::ref<struct cpp_builtin_operator_call>;
  using cpp_box_ref = jtl::ref<struct cpp_box>;
  using cpp_unbox_ref = jtl::ref<struct cpp_unbox>;
  using cpp_new_ref = jtl::ref<struct cpp_new>;
  using cpp_delete_ref = jtl::ref<struct cpp_delete>;
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
    void remove_block(usize const block_index);
    void enter_block(usize const blk_index);

    identifier
    parameter(analyze::expression_position const pos, jtl::immutable_string const &value);
    identifier capture(analyze::expression_position const pos,
                       jtl::ptr<void> const type,
                       jtl::immutable_string const &value);
    identifier literal(analyze::expression_position const pos, runtime::object_ref const value);
    identifier persistent_list(analyze::expression_position const pos,
                               native_vector<identifier> &&values,
                               jtl::option<identifier> const &meta);
    identifier persistent_vector(analyze::expression_position const pos,
                                 native_vector<identifier> &&values,
                                 jtl::option<identifier> const &meta);
    identifier persistent_array_map(analyze::expression_position const pos,
                                    native_vector<std::pair<identifier, identifier>> &&values,
                                    jtl::option<identifier> const &meta);
    identifier persistent_hash_map(analyze::expression_position const pos,
                                   native_vector<std::pair<identifier, identifier>> &&values,
                                   jtl::option<identifier> const &meta);
    identifier persistent_hash_set(analyze::expression_position const pos,
                                   native_vector<identifier> &&values,
                                   jtl::option<identifier> const &meta);
    identifier function(analyze::expression_position const pos,
                        native_unordered_map<u8, jtl::immutable_string> &&arities,
                        runtime::callable_arity_flags const arity_flags);
    identifier closure(analyze::expression_position const pos,
                       jtl::immutable_string const &context,
                       native_unordered_map<u8, jtl::immutable_string> &&arities,
                       native_unordered_map<jtl::immutable_string, identifier> &&captures,
                       runtime::callable_arity_flags const arity_flags);
    identifier letfn(native_vector<jtl::immutable_string> &&bindings);
    identifier def(analyze::expression_position const pos,
                   jtl::immutable_string const &qualified_var,
                   jtl::option<identifier> const &value,
                   identifier const &meta,
                   bool const is_dynamic);
    identifier
    var_deref(analyze::expression_position const pos, jtl::immutable_string const &qualified_var);
    identifier
    var_ref(analyze::expression_position const pos, jtl::immutable_string const &qualified_var);
    identifier dynamic_call(analyze::expression_position const pos,
                            identifier const &fn,
                            native_vector<identifier> &&args);
    identifier named_recursion(analyze::expression_position const pos,
                               identifier const &fn,
                               native_vector<identifier> &&args);
    identifier recursion_reference(analyze::expression_position const pos);
    identifier truthy(identifier const &value);
    identifier jump(usize const index);
    identifier jump(usize const index, bool const loop);
    identifier branch_set(identifier const &shadow, identifier const &value);
    identifier branch_get(identifier const &name, jtl::ptr<void> const type) const;
    identifier branch(identifier const &condition,
                      identifier const &then_blk,
                      identifier const &else_blk,
                      jtl::option<identifier> const &merge_blk,
                      jtl::option<detail::typed_shadow> const &shadow);
    identifier loop(identifier const &loop_blk,
                    jtl::option<identifier> const &merge_blk,
                    jtl::option<detail::typed_shadow> const &shadow,
                    native_vector<inst::loop::binding_shadow_details> &&shadows);
    identifier case_(i64 const shift,
                     i64 const mask,
                     identifier const &value,
                     native_unordered_map<i64, identifier> &&cases,
                     identifier const &default_block,
                     jtl::option<identifier> const &merge_block,
                     jtl::option<identifier> const &shadow);
    identifier try_(native_vector<std::pair<jtl::ptr<void>, identifier>> &&catches,
                    jtl::option<identifier> const &merge_block,
                    jtl::option<identifier> const &shadow,
                    jtl::option<identifier> const &finally_block);
    identifier catch_(jtl::ptr<void> const type,
                      jtl::option<identifier> const &merge_block,
                      jtl::option<identifier> const &shadow,
                      jtl::option<identifier> const &finally_block);
    identifier finally(identifier const &merge_block);
    identifier throw_(identifier const &value);
    identifier ret(identifier const &value, jtl::ptr<void> const type);

    identifier cpp_raw(analyze::expr::cpp_raw_ref const expr);
    identifier cpp_value(analyze::expr::cpp_value_ref const expr);
    identifier
    cpp_conversion(identifier const &value, analyze::expr::cpp_conversion_ref const expr);
    identifier
    cpp_unsafe_cast(identifier const &value, analyze::expr::cpp_unsafe_cast_ref const expr);
    identifier cpp_call(jtl::option<identifier> const &value,
                        native_vector<identifier> &&args,
                        analyze::expr::cpp_call_ref const expr);
    identifier cpp_constructor_call(native_vector<identifier> &&args,
                                    analyze::expr::cpp_constructor_call_ref const expr);
    identifier cpp_member_call(native_vector<identifier> &&args,
                               analyze::expr::cpp_member_call_ref const expr);
    identifier
    cpp_member_access(identifier const &value, analyze::expr::cpp_member_access_ref const expr);
    identifier cpp_builtin_operator_call(native_vector<identifier> &&args,
                                         analyze::expr::cpp_builtin_operator_call_ref const expr);
    identifier cpp_box(identifier const &value, analyze::expr::cpp_box_ref const expr);
    identifier cpp_unbox(identifier const &value,
                         identifier const &meta,
                         analyze::expr::cpp_unbox_ref const expr);
    identifier cpp_new(identifier const &value, analyze::expr::cpp_new_ref const expr);
    identifier cpp_delete(identifier const &value, analyze::expr::cpp_delete_ref const expr);

    jtl::ref<module> mod;
    usize fn_index{};
    usize block_index{};
    usize ident_count{};
    native_unordered_map<jtl::immutable_string, identifier> locals{};
    native_unordered_map<jtl::immutable_string, identifier> local_to_loop_shadow{};
    native_set<jtl::immutable_string> allowed_defers{};
    jtl::option<usize> loop_recur_target{};
    jtl::option<usize> fn_recur_target{};
    native_unordered_map<jtl::immutable_string, identifier> lifted_vars{};
    native_unordered_map<runtime::object_ref,
                         identifier,
                         std::hash<runtime::object_ref>,
                         runtime::very_equal_to>
      lifted_constants{};
  };
}
