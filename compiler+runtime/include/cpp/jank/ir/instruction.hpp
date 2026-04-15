#pragma once

#include <jtl/ref.hpp>
#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::analyze
{
  using expression_ref = jtl::ref<struct expression>;

  namespace expr
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
}

namespace jank::ir
{
  using identifier = jtl::immutable_string;

  enum class instruction_kind : u8
  {
    parameter,
    capture,
    literal,
    persistent_list,
    persistent_vector,
    persistent_array_map,
    persistent_hash_map,
    persistent_hash_set,
    function,
    closure,
    letfn,
    def,
    var_deref,
    var_ref,
    dynamic_call,
    named_recursion,
    recursion_reference,
    truthy,
    jump,
    branch_set,
    branch_get,
    branch,
    loop,
    case_,
    try_,
    catch_,
    finally,
    throw_,
    ret,
    cpp_raw,
    cpp_value,
    cpp_into_object,
    cpp_from_object,
    cpp_unsafe_cast,
    cpp_call,
    cpp_constructor_call,
    cpp_member_call,
    cpp_member_access,
    cpp_builtin_operator_call,
    cpp_box,
    cpp_unbox,
    cpp_new,
    cpp_delete
  };

  struct instruction
  {
    instruction(instruction_kind const kind, identifier const &name, jtl::ptr<void> const type);
    virtual ~instruction() = default;

    virtual bool is_terminator() const;
    virtual void print(jtl::string_builder &sb, usize indent) const = 0;

    instruction_kind kind;
    identifier name;
    jtl::ptr<void> type;
  };

  using instruction_ref = jtl::ref<instruction>;

  namespace detail
  {
    struct typed_identifier
    {
      identifier name;
      jtl::ptr<void> type;
    };
  }

  namespace inst
  {
    struct parameter : instruction
    {
      parameter(identifier const &name, jtl::ptr<void> const type);

      void print(jtl::string_builder &sb, usize indent) const override;
    };

    using parameter_ref = jtl::ref<parameter>;

    struct capture : instruction
    {
      capture(identifier const &name,
              jtl::ptr<void> const type,
              jtl::immutable_string const &value);

      void print(jtl::string_builder &sb, usize indent) const override;

      jtl::immutable_string value;
    };

    using capture_ref = jtl::ref<capture>;

    struct literal : instruction
    {
      literal(identifier const &name,
              jtl::ptr<void> const type,
              runtime::object_ref const obj,
              identifier const &value);

      void print(jtl::string_builder &sb, usize indent) const override;

      runtime::object_ref obj;
      identifier value;
    };

    using literal_ref = jtl::ref<literal>;

    struct persistent_list : instruction
    {
      persistent_list(identifier const &name,
                      native_vector<identifier> &&values,
                      jtl::option<identifier> const &meta);

      void print(jtl::string_builder &sb, usize indent) const override;

      native_vector<identifier> values;
      jtl::option<identifier> meta;
    };

    using persistent_list_ref = jtl::ref<persistent_list>;

    struct persistent_vector : instruction
    {
      persistent_vector(identifier const &name,
                        native_vector<identifier> &&values,
                        jtl::option<identifier> const &meta);

      void print(jtl::string_builder &sb, usize indent) const override;

      native_vector<identifier> values;
      jtl::option<identifier> meta;
    };

    using persistent_vector_ref = jtl::ref<persistent_vector>;

    struct persistent_array_map : instruction
    {
      persistent_array_map(identifier const &name,
                           native_vector<std::pair<identifier, identifier>> &&values,
                           jtl::option<identifier> const &meta);

      void print(jtl::string_builder &sb, usize indent) const override;

      native_vector<std::pair<identifier, identifier>> values;
      jtl::option<identifier> meta;
    };

    using persistent_array_map_ref = jtl::ref<persistent_array_map>;

    struct persistent_hash_map : instruction
    {
      persistent_hash_map(identifier const &name,
                          native_vector<std::pair<identifier, identifier>> &&values,
                          jtl::option<identifier> const &meta);

      void print(jtl::string_builder &sb, usize indent) const override;

      native_vector<std::pair<identifier, identifier>> values;
      jtl::option<identifier> meta;
    };

    using persistent_hash_map_ref = jtl::ref<persistent_hash_map>;

    struct persistent_hash_set : instruction
    {
      persistent_hash_set(identifier const &name,
                          native_vector<identifier> &&values,
                          jtl::option<identifier> const &meta);

      void print(jtl::string_builder &sb, usize indent) const override;

      native_vector<identifier> values;
      jtl::option<identifier> meta;
    };

    using persistent_hash_set_ref = jtl::ref<persistent_hash_set>;

    struct function : instruction
    {
      function(identifier const &name,
               native_unordered_map<u8, jtl::immutable_string> &&arities,
               runtime::callable_arity_flags const arity_flags);

      void print(jtl::string_builder &sb, usize indent) const override;

      native_unordered_map<u8, jtl::immutable_string> arities;
      runtime::callable_arity_flags arity_flags{};
    };

    using function_ref = jtl::ref<function>;

    struct closure : instruction
    {
      closure(identifier const &name,
              jtl::immutable_string const &context,
              native_unordered_map<u8, jtl::immutable_string> &&arities,
              native_unordered_map<jtl::immutable_string, detail::typed_identifier> &&captures,
              runtime::callable_arity_flags const arity_flags);

      void print(jtl::string_builder &sb, usize indent) const override;

      jtl::immutable_string context;
      native_unordered_map<u8, jtl::immutable_string> arities;
      native_unordered_map<jtl::immutable_string, detail::typed_identifier> captures;
      runtime::callable_arity_flags arity_flags{};
    };

    using closure_ref = jtl::ref<closure>;

    struct letfn : instruction
    {
      letfn(identifier const &name, native_vector<jtl::immutable_string> &&bindings);

      void print(jtl::string_builder &sb, usize indent) const override;

      native_vector<jtl::immutable_string> bindings;
    };

    using letfn_ref = jtl::ref<letfn>;

    struct def : instruction
    {
      def(identifier const &name,
          jtl::ptr<void> const type,
          jtl::immutable_string const &qualified_var,
          jtl::option<identifier> const &value,
          identifier const &meta,
          bool const is_dynamic);

      void print(jtl::string_builder &sb, usize indent) const override;

      jtl::immutable_string qualified_var;
      jtl::option<identifier> value;
      identifier meta;
      bool is_dynamic{};
    };

    using def_ref = jtl::ref<def>;

    struct var_deref : instruction
    {
      var_deref(identifier const &name, identifier const &var);

      void print(jtl::string_builder &sb, usize indent) const override;

      identifier var;
    };

    using var_deref_ref = jtl::ref<var_deref>;

    struct var_ref : instruction
    {
      var_ref(identifier const &name, jtl::ptr<void> const type, identifier const &var);

      void print(jtl::string_builder &sb, usize indent) const override;

      identifier var;
    };

    using var_ref_ref = jtl::ref<var_ref>;

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

    using dynamic_call_ref = jtl::ref<dynamic_call>;

    struct named_recursion : instruction
    {
      named_recursion(identifier const &name,
                      jtl::ptr<void> const type,
                      identifier const &fn,
                      native_vector<identifier> &&args);

      void print(jtl::string_builder &sb, usize indent) const override;

      identifier fn;
      native_vector<identifier> args;
    };

    using named_recursion_ref = jtl::ref<named_recursion>;

    struct recursion_reference : instruction
    {
      recursion_reference(identifier const &name, jtl::ptr<void> const type);

      void print(jtl::string_builder &sb, usize indent) const override;
    };

    using recursion_reference_ref = jtl::ref<recursion_reference>;

    /* Truthy converts a jank object into a bool by calling `jank::runtime::truthy`. */
    struct truthy : instruction
    {
      truthy(identifier const &name, identifier const &value);

      void print(jtl::string_builder &sb, usize indent) const override;

      identifier value;
    };

    using truthy_ref = jtl::ref<truthy>;

    /* Jump is a terminator. Nothing may follow it, in the same block. */
    struct jump : instruction
    {
      jump(identifier const &name, identifier const &block);
      jump(identifier const &name, identifier const &block, bool const loop);

      bool is_terminator() const override;
      void print(jtl::string_builder &sb, usize indent) const override;

      identifier block;
      bool loop{};
    };

    using jump_ref = jtl::ref<jump>;

    /* Branch set/get follow the pizlo-style upsilon/phi practice.
     * https://gist.github.com/pizlonator/cf1e72b8600b1437dda8153ea3fdb963 */
    struct branch_set : instruction
    {
      branch_set(identifier const &name, identifier const &shadow, identifier const &value);

      void print(jtl::string_builder &sb, usize indent) const override;

      identifier shadow;
      identifier value;
    };

    using branch_set_ref = jtl::ref<branch_set>;

    struct branch_get : instruction
    {
      branch_get(identifier const &name, jtl::ptr<void> const type);

      void print(jtl::string_builder &sb, usize indent) const override;
    };

    using branch_get_ref = jtl::ref<branch_get>;

    /* Branch is a terminator. Nothing may follow it, in the same block. */
    struct branch : instruction
    {
      branch(identifier const &name,
             identifier const &condition,
             identifier const &then_block,
             identifier const &else_block,
             jtl::option<identifier> const &merge_block,
             jtl::option<detail::typed_identifier> const &shadow);

      bool is_terminator() const override;
      void print(jtl::string_builder &sb, usize indent) const override;

      identifier condition;
      identifier then_block;
      identifier else_block;
      jtl::option<identifier> merge_block;
      jtl::option<detail::typed_identifier> shadow;
    };

    using branch_ref = jtl::ref<branch>;

    /* Loop is a terminator. Nothing may follow it, in the same block. */
    struct loop : instruction
    {
      struct binding_shadow_details
      {
        identifier name;
        identifier value;
        jtl::ptr<void> type;
      };

      loop(identifier const &name,
           identifier const &loop_block,
           jtl::option<identifier> const &merge_block,
           jtl::option<detail::typed_identifier> const &shadow,
           native_vector<binding_shadow_details> &&binding_shadows);

      bool is_terminator() const override;
      void print(jtl::string_builder &sb, usize indent) const override;

      identifier loop_block;
      jtl::option<identifier> merge_block;
      jtl::option<detail::typed_identifier> shadow;
      native_vector<binding_shadow_details> binding_shadows;
    };

    using loop_ref = jtl::ref<loop>;

    /* Case is a terminator. Nothing may follow it, in the same block. The map of case blocks
     * shows all of the different cases which need to be covered and their corresponding starting
     * blocks. */
    struct case_ : instruction
    {
      case_(identifier const &name,
            i64 const shift,
            i64 const mask,
            identifier const &value,
            native_unordered_map<i64, identifier> &&case_blocks,
            identifier const &default_block,
            jtl::option<identifier> const &merge_block,
            jtl::option<identifier> const &shadow);

      bool is_terminator() const override;
      void print(jtl::string_builder &sb, usize indent) const override;

      i64 shift{};
      i64 mask{};
      identifier value;
      native_unordered_map<i64, identifier> case_blocks;
      identifier default_block;
      jtl::option<identifier> merge_block;
      jtl::option<identifier> shadow;
    };

    using case_ref = jtl::ref<case_>;

    /* Try indicates that the rest of the block should be in a try body. A try must either
     * have some catches or a finally. The merge block is always present. */
    struct try_ : instruction
    {
      try_(identifier const &name,
           native_vector<std::pair<jtl::ptr<void>, identifier>> &&catches,
           identifier const &merge_block,
           identifier const &shadow,
           jtl::option<identifier> const &finally_block);

      void print(jtl::string_builder &sb, usize indent) const override;

      native_vector<std::pair<jtl::ptr<void>, identifier>> catches;
      identifier merge_block;
      identifier shadow;
      jtl::option<identifier> finally_block;
    };

    using try_ref = jtl::ref<try_>;

    /* Catch instructions are positioned at the start of a block and they indicate that the
     * block is dedicated to the catch body. */
    struct catch_ : instruction
    {
      catch_(identifier const &name,
             jtl::ptr<void> const type,
             jtl::option<identifier> const &merge_block,
             jtl::option<identifier> const &shadow,
             jtl::option<identifier> const &finally_block);

      void print(jtl::string_builder &sb, usize indent) const override;

      jtl::option<identifier> merge_block;
      jtl::option<identifier> shadow;
      jtl::option<identifier> finally_block;
    };

    using catch_ref = jtl::ref<catch_>;

    struct finally : instruction
    {
      finally(identifier const &name, identifier const &merge_block);

      void print(jtl::string_builder &sb, usize indent) const override;

      identifier merge_block;
    };

    using finally_ref = jtl::ref<finally>;

    /* Throw is a terminator, but we currently allow it in the middle of a block as well. This
     * is mainly just a convenience. We can have a pass which strips out all instructions in
     * a block after a throw. */
    struct throw_ : instruction
    {
      throw_(identifier const &name, identifier const &value);

      bool is_terminator() const override;
      void print(jtl::string_builder &sb, usize indent) const override;

      identifier value;
    };

    using throw_ref = jtl::ref<throw_>;

    /* Ret is a terminator. Nothing may follow it, in the same block. */
    struct ret : instruction
    {
      ret(identifier const &name, jtl::ptr<void> const type, identifier const &value);

      bool is_terminator() const override;
      void print(jtl::string_builder &sb, usize indent) const override;

      identifier value;
    };

    using ret_ref = jtl::ref<ret>;

    struct cpp_raw : instruction
    {
      cpp_raw(identifier const &name, analyze::expr::cpp_raw_ref const expr);

      void print(jtl::string_builder &sb, usize indent) const override;

      analyze::expr::cpp_raw_ref const expr;
    };

    using cpp_raw_ref = jtl::ref<cpp_raw>;

    struct cpp_value : instruction
    {
      cpp_value(identifier const &name, analyze::expr::cpp_value_ref const expr);

      void print(jtl::string_builder &sb, usize) const override;

      analyze::expr::cpp_value_ref expr;
    };

    using cpp_value_ref = jtl::ref<cpp_value>;

    struct cpp_into_object : instruction
    {
      cpp_into_object(identifier const &name,
                      identifier const &value,
                      analyze::expr::cpp_conversion_ref const expr);

      void print(jtl::string_builder &sb, usize) const override;

      identifier value;
      analyze::expr::cpp_conversion_ref expr;
    };

    using cpp_into_object_ref = jtl::ref<cpp_into_object>;

    struct cpp_from_object : instruction
    {
      cpp_from_object(identifier const &name,
                      identifier const &value,
                      analyze::expr::cpp_conversion_ref const expr);

      void print(jtl::string_builder &sb, usize) const override;

      identifier value;
      analyze::expr::cpp_conversion_ref expr;
    };

    using cpp_from_object_ref = jtl::ref<cpp_from_object>;

    struct cpp_unsafe_cast : instruction
    {
      cpp_unsafe_cast(identifier const &name,
                      identifier const &value,
                      analyze::expr::cpp_unsafe_cast_ref const expr);

      void print(jtl::string_builder &sb, usize) const override;

      identifier value;
      analyze::expr::cpp_unsafe_cast_ref expr;
    };

    using cpp_unsafe_cast_ref = jtl::ref<cpp_unsafe_cast>;

    struct cpp_call : instruction
    {
      cpp_call(identifier const &name,
               jtl::option<identifier> const &value,
               native_vector<identifier> &&args,
               analyze::expr::cpp_call_ref const expr);

      void print(jtl::string_builder &sb, usize) const override;

      jtl::option<identifier> value;
      native_vector<identifier> args;
      analyze::expr::cpp_call_ref expr;
    };

    using cpp_call_ref = jtl::ref<cpp_call>;

    struct cpp_constructor_call : instruction
    {
      cpp_constructor_call(identifier const &name,
                           native_vector<identifier> &&args,
                           analyze::expr::cpp_constructor_call_ref const expr);

      void print(jtl::string_builder &sb, usize) const override;

      native_vector<identifier> args;
      analyze::expr::cpp_constructor_call_ref expr;
    };

    using cpp_constructor_call_ref = jtl::ref<cpp_constructor_call>;

    struct cpp_member_call : instruction
    {
      cpp_member_call(identifier const &name,
                      native_vector<identifier> &&args,
                      analyze::expr::cpp_member_call_ref const expr);

      void print(jtl::string_builder &sb, usize) const override;

      identifier value;
      native_vector<identifier> args;
      analyze::expr::cpp_member_call_ref expr;
    };

    using cpp_member_call_ref = jtl::ref<cpp_member_call>;

    struct cpp_member_access : instruction
    {
      cpp_member_access(identifier const &name,
                        identifier const &value,
                        analyze::expr::cpp_member_access_ref const expr);

      void print(jtl::string_builder &sb, usize) const override;

      identifier value;
      analyze::expr::cpp_member_access_ref expr;
    };

    using cpp_member_access_ref = jtl::ref<cpp_member_access>;

    struct cpp_builtin_operator_call : instruction
    {
      cpp_builtin_operator_call(identifier const &name,
                                native_vector<identifier> &&args,
                                analyze::expr::cpp_builtin_operator_call_ref const expr);

      void print(jtl::string_builder &sb, usize) const override;

      native_vector<identifier> args;
      analyze::expr::cpp_builtin_operator_call_ref expr;
    };

    using cpp_builtin_operator_call_ref = jtl::ref<cpp_builtin_operator_call>;

    struct cpp_box : instruction
    {
      cpp_box(identifier const &name,
              identifier const &value,
              analyze::expr::cpp_box_ref const expr);

      void print(jtl::string_builder &sb, usize) const override;

      identifier value;
      analyze::expr::cpp_box_ref expr;
    };

    using cpp_box_ref = jtl::ref<cpp_box>;

    struct cpp_unbox : instruction
    {
      cpp_unbox(identifier const &name,
                identifier const &value,
                identifier const &meta,
                analyze::expr::cpp_unbox_ref const expr);

      void print(jtl::string_builder &sb, usize) const override;

      identifier value;
      identifier meta;
      analyze::expr::cpp_unbox_ref expr;
    };

    using cpp_unbox_ref = jtl::ref<cpp_unbox>;

    struct cpp_new : instruction
    {
      cpp_new(identifier const &name,
              identifier const &value,
              analyze::expr::cpp_new_ref const expr);

      void print(jtl::string_builder &sb, usize) const override;

      identifier value;
      analyze::expr::cpp_new_ref expr;
    };

    using cpp_new_ref = jtl::ref<cpp_new>;

    struct cpp_delete : instruction
    {
      cpp_delete(identifier const &name,
                 identifier const &value,
                 analyze::expr::cpp_delete_ref const expr);

      void print(jtl::string_builder &sb, usize) const override;

      identifier value;
      analyze::expr::cpp_delete_ref expr;
    };

    using cpp_delete_ref = jtl::ref<cpp_delete>;
  }
}
