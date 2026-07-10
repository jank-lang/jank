#include <jank/ir/builder.hpp>
#include <jank/util/fmt.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/analyze/cpp_util.hpp>
#include <jank/analyze/visit.hpp>

namespace jank::ir
{
  using namespace analyze::cpp_util;

  identifier builder::next_ident()
  {
    return next_ident("v");
  }

  identifier builder::next_ident(jtl::immutable_string const &prefix)
  {
    identifier ret;
    /* NOLINTNEXTLINE(cppcoreguidelines-avoid-do-while) */
    do
    {
      ret = util::format("{}{}", prefix, ident_count++);
    } while(used_identifiers.contains(ret));
    used_identifiers.emplace(ret);
    return ret;
  }

  identifier builder::next_shadow()
  {
    return next_ident("s");
  }

  jtl::ref<function> builder::current_function() const
  {
    return &mod->functions[fn_index];
  }

  jtl::ref<block> builder::current_block() const
  {
    return &current_function()->blocks[block_index];
  }

  usize builder::block(identifier const &name) const
  {
    return current_function()->add_block(name);
  }

  identifier builder::block_name(usize const block_index) const
  {
    return current_function()->blocks[block_index].name;
  }

  void builder::remove_block(usize const block_index)
  {
    jank_debug_assert(this->block_index != block_index);
    current_function()->remove_block(block_index);

    if(this->block_index >= block_index)
    {
      --this->block_index;
    }
  }

  void builder::enter_block(usize const blk_index)
  {
    block_index = blk_index;
  }

  identifier
  builder::parameter(analyze::expression_position const pos, jtl::immutable_string const &value)
  {
    auto name{ runtime::munge(value) };
    auto const type{ untyped_object_ref_type() };
    used_identifiers.emplace(name);
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::parameter>(name, type, location));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, type);
    }
    return name;
  }

  identifier builder::capture(analyze::expression_position const pos,
                              jtl::ptr<void> const type,
                              jtl::immutable_string const &value)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::capture>(name, type, location, value));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, type);
    }
    return name;
  }

  identifier
  builder::literal(analyze::expression_position const pos, runtime::object_ref const value)
  {
    auto const type{ literal_type(value) };
    jtl::immutable_string lifted_literal_name;
    auto const found{ mod->lifted_constants.find(value) };
    if(found == mod->lifted_constants.end())
    {
      lifted_literal_name = runtime::munge(runtime::__rt_ctx->unique_string("const"));
      mod->lifted_constants.emplace(value, lifted_literal_name);
    }
    else
    {
      lifted_literal_name = found->second;
    }

    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::literal>(name, type, location, value, lifted_literal_name));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, type);
    }
    return name;
  }

  identifier builder::persistent_list(analyze::expression_position const pos,
                                      native_vector<identifier> &&values,
                                      runtime::object_ref const meta)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::persistent_list>(name, location, jtl::move(values), meta));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, current_function()->blocks[block_index].instructions.back()->type);
    }
    return name;
  }

  identifier builder::persistent_vector(analyze::expression_position const pos,
                                        native_vector<identifier> &&values,
                                        runtime::object_ref const meta)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::persistent_vector>(name, location, jtl::move(values), meta));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, current_function()->blocks[block_index].instructions.back()->type);
    }
    return name;
  }

  identifier
  builder::persistent_array_map(analyze::expression_position const pos,
                                native_vector<std::pair<identifier, identifier>> &&values,
                                runtime::object_ref const meta)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::persistent_array_map>(name, location, jtl::move(values), meta));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, current_function()->blocks[block_index].instructions.back()->type);
    }
    return name;
  }

  identifier builder::persistent_hash_map(analyze::expression_position const pos,
                                          native_vector<std::pair<identifier, identifier>> &&values,
                                          runtime::object_ref const meta)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::persistent_hash_map>(name, location, jtl::move(values), meta));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, current_function()->blocks[block_index].instructions.back()->type);
    }
    return name;
  }

  identifier builder::persistent_hash_set(analyze::expression_position const pos,
                                          native_vector<identifier> &&values,
                                          runtime::object_ref const meta)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::persistent_hash_set>(name, location, jtl::move(values), meta));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, current_function()->blocks[block_index].instructions.back()->type);
    }
    return name;
  }

  identifier builder::function(analyze::expression_position const pos,
                               native_unordered_map<u8, jtl::immutable_string> &&arities,
                               runtime::callable_arity_flags const arity_flags,
                               bool const is_variadic)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::function>(name, location, jtl::move(arities), arity_flags, is_variadic));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, current_function()->blocks[block_index].instructions.back()->type);
    }
    return name;
  }

  identifier
  builder::closure(analyze::expression_position const pos,
                   jtl::immutable_string const &context,
                   native_unordered_map<u8, jtl::immutable_string> &&arities,
                   native_unordered_map<jtl::immutable_string, detail::typed_identifier> &&captures,
                   runtime::callable_arity_flags const arity_flags,
                   bool const is_variadic)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::closure>(name,
                                   location,
                                   context,
                                   jtl::move(arities),
                                   jtl::move(captures),
                                   arity_flags,
                                   is_variadic));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, current_function()->blocks[block_index].instructions.back()->type);
    }
    return name;
  }

  identifier builder::letfn(native_vector<jtl::immutable_string> &&bindings)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::letfn>(name, location, jtl::move(bindings)));
    return name;
  }

  identifier builder::def(analyze::expression_position const pos,
                          jtl::immutable_string const &qualified_var,
                          jtl::option<identifier> const &value,
                          runtime::object_ref const meta,
                          bool const is_dynamic)
  {
    auto name{ next_ident() };
    auto const type{ var_type() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::def>(name, type, location, qualified_var, value, meta, is_dynamic));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, type);
    }
    return name;
  }

  identifier builder::var_deref(analyze::expression_position const pos,
                                jtl::immutable_string const &qualified_var)
  {
    auto name{ next_ident() };
    static jtl::immutable_string const dot{ "\\." };
    auto const us{ runtime::__rt_ctx->unique_string(qualified_var) };
    jtl::immutable_string lifted_var_name;
    auto const found{ mod->lifted_vars.find(qualified_var) };
    if(found == mod->lifted_vars.end())
    {
      lifted_var_name = runtime::munge_and_replace(us, dot, "_");
      mod->lifted_vars.emplace(qualified_var, module::lifted_var{ lifted_var_name, false });
    }
    else
    {
      lifted_var_name = found->second.name;
    }

    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::var_deref>(name, location, qualified_var));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, untyped_object_ref_type());
    }
    return name;
  }

  identifier builder::var_ref(analyze::expression_position const pos,
                              jtl::immutable_string const &qualified_var)
  {
    static jtl::immutable_string const dot{ "\\." };
    auto const us{ runtime::__rt_ctx->unique_string(qualified_var) };
    auto var_name{ runtime::munge_and_replace(us, dot, "_") };
    mod->lifted_vars.emplace(qualified_var, module::lifted_var{ var_name, false });
    auto const type{ var_type() };
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::var_ref>(name, type, location, qualified_var));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, type);
    }
    return name;
  }

  identifier builder::type_erase(analyze::expression_position const pos, identifier const &value)
  {
    auto name{ next_ident() };
    auto const type{ untyped_object_ref_type() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::type_erase>(name, location, value));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, type);
    }
    return name;
  }

  identifier builder::dynamic_call(analyze::expression_position const pos,
                                   identifier const &fn,
                                   native_vector<identifier> &&args)
  {
    auto name{ next_ident() };
    auto const type{ untyped_object_ref_type() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::dynamic_call>(name, type, location, fn, jtl::move(args)));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, type);
    }
    return name;
  }

  identifier builder::named_recursion(analyze::expression_position const pos,
                                      identifier const &fn,
                                      jtl::immutable_string const &fn_base_name,
                                      native_vector<identifier> &&args,
                                      bool const needs_dynamic_call)
  {
    auto name{ next_ident() };
    auto const type{ untyped_object_ref_type() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::named_recursion>(name,
                                           type,
                                           location,
                                           fn,
                                           fn_base_name,
                                           jtl::move(args),
                                           needs_dynamic_call));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, type);
    }
    return name;
  }

  identifier builder::recursion_reference(analyze::expression_position const pos)
  {
    auto name{ next_ident() };
    auto const type{ untyped_object_ref_type() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::recursion_reference>(name, type, location));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, type);
    }
    return name;
  }

  identifier builder::truthy(identifier const &value)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::truthy>(name, location, value));
    return name;
  }

  identifier builder::jump(usize const index)
  {
    auto name{ next_ident() };
    auto const &block{ current_function()->blocks[index].name };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::jump>(name, location, block));
    return name;
  }

  identifier builder::jump(usize const index, bool const loop)
  {
    auto name{ next_ident() };
    auto const &block{ current_function()->blocks[index].name };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::jump>(name, location, block, loop));
    return name;
  }

  identifier builder::branch_set(identifier const &shadow, identifier const &value)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::branch_set>(name, location, shadow, value));
    return name;
  }

  identifier builder::branch_get(identifier const &name, jtl::ptr<void> const type) const
  {
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::branch_get>(name, type, location));
    return name;
  }

  identifier builder::branch(identifier const &condition,
                             identifier const &then_blk,
                             identifier const &else_blk,
                             jtl::option<identifier> const &merge_blk,
                             jtl::option<detail::typed_identifier> const &shadow)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::branch>(name,
                                  location,
                                  condition,
                                  then_blk,
                                  else_blk,
                                  merge_blk,
                                  shadow));
    return name;
  }

  identifier builder::loop(identifier const &loop_blk,
                           jtl::option<identifier> const &merge_blk,
                           jtl::option<detail::typed_identifier> const &shadow,
                           native_vector<inst::loop::binding_shadow_details> &&shadows)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::loop>(name, location, loop_blk, merge_blk, shadow, jtl::move(shadows)));
    return name;
  }

  identifier builder::case_(i64 const shift,
                            i64 const mask,
                            identifier const &value,
                            native_unordered_map<i64, identifier> &&cases,
                            identifier const &default_block,
                            jtl::option<identifier> const &merge_block,
                            jtl::option<identifier> const &shadow)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::case_>(name,
                                 location,
                                 shift,
                                 mask,
                                 value,
                                 jtl::move(cases),
                                 default_block,
                                 merge_block,
                                 shadow));
    return name;
  }

  identifier builder::try_(native_vector<std::pair<jtl::ptr<void>, identifier>> &&catches,
                           identifier const &merge_block,
                           identifier const &shadow,
                           jtl::option<identifier> const &finally_block)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::try_>(name,
                                location,
                                jtl::move(catches),
                                merge_block,
                                shadow,
                                finally_block));
    return name;
  }

  identifier builder::catch_(jtl::ptr<void> const type,
                             jtl::option<identifier> const &merge_block,
                             jtl::option<identifier> const &shadow,
                             jtl::option<identifier> const &finally_block)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::catch_>(name, type, location, merge_block, shadow, finally_block));
    return name;
  }

  identifier builder::finally(identifier const &merge_block)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::finally>(name, location, merge_block));
    return name;
  }

  identifier builder::throw_(identifier const &value)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::throw_>(name, location, value));
    return name;
  }

  identifier builder::ret(identifier const &value, jtl::ptr<void> const type)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::ret>(name, type, location, value));
    return name;
  }

  identifier builder::cpp_scope_open()
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::cpp_scope_open>(name, location));
    return name;
  }

  identifier builder::cpp_scope_close(identifier const &scope)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::cpp_scope_close>(name, location, scope));
    return name;
  }

  identifier builder::cpp_raw(analyze::expr::cpp_raw_ref const expr)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::cpp_raw>(name, location, expr));
    if(expr->position == analyze::expression_position::tail)
    {
      return ret(name, expression_type(expr));
    }
    return name;
  }

  identifier builder::cpp_value(analyze::expr::cpp_value_ref const expr)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::cpp_value>(name, location, expr));
    if(expr->position == analyze::expression_position::tail)
    {
      return ret(name, expression_type(expr));
    }
    return name;
  }

  identifier
  builder::cpp_conversion(identifier const &value, analyze::expr::cpp_conversion_ref const expr)
  {
    auto name{ next_ident() };
    if(expr->policy == analyze::conversion_policy::into_object)
    {
      current_function()->blocks[block_index].instructions.emplace_back(
        jtl::make_ref<inst::cpp_into_object>(name, location, value, expr));
    }
    else
    {
      current_function()->blocks[block_index].instructions.emplace_back(
        jtl::make_ref<inst::cpp_from_object>(name, location, value, expr));
    }
    if(expr->position == analyze::expression_position::tail)
    {
      return ret(name, expression_type(expr));
    }
    return name;
  }

  identifier
  builder::cpp_unsafe_cast(identifier const &value, analyze::expr::cpp_unsafe_cast_ref const expr)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::cpp_unsafe_cast>(name, location, value, expr));
    if(expr->position == analyze::expression_position::tail)
    {
      return ret(name, expression_type(expr));
    }
    return name;
  }

  identifier builder::cpp_call(jtl::option<identifier> const &value,
                               native_vector<identifier> &&args,
                               analyze::expr::cpp_call_ref const expr)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::cpp_call>(name, location, value, jtl::move(args), expr));
    if(expr->position == analyze::expression_position::tail)
    {
      return ret(name, expression_type(expr));
    }
    return name;
  }

  identifier builder::cpp_constructor_call(native_vector<identifier> &&args,
                                           analyze::expr::cpp_constructor_call_ref const expr)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::cpp_constructor_call>(name, location, jtl::move(args), expr));
    if(expr->position == analyze::expression_position::tail)
    {
      return ret(name, expression_type(expr));
    }
    return name;
  }

  identifier builder::cpp_member_call(native_vector<identifier> &&args,
                                      analyze::expr::cpp_member_call_ref const expr)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::cpp_member_call>(name, location, jtl::move(args), expr));
    if(expr->position == analyze::expression_position::tail)
    {
      return ret(name, expression_type(expr));
    }
    return name;
  }

  identifier builder::cpp_member_access(identifier const &value,
                                        analyze::expr::cpp_member_access_ref const expr)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::cpp_member_access>(name, location, value, expr));
    if(expr->position == analyze::expression_position::tail)
    {
      return ret(name, expression_type(expr));
    }
    return name;
  }

  identifier
  builder::cpp_builtin_operator_call(native_vector<identifier> &&args,
                                     analyze::expr::cpp_builtin_operator_call_ref const expr)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::cpp_builtin_operator_call>(name, location, jtl::move(args), expr));
    if(expr->position == analyze::expression_position::tail)
    {
      return ret(name, expression_type(expr));
    }
    return name;
  }

  identifier builder::cpp_box(identifier const &value, analyze::expr::cpp_box_ref const expr)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::cpp_box>(name, location, value, expr));
    if(expr->position == analyze::expression_position::tail)
    {
      return ret(name, expression_type(expr));
    }
    return name;
  }

  identifier builder::cpp_unbox(identifier const &value,
                                identifier const &meta,
                                analyze::expr::cpp_unbox_ref const expr)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::cpp_unbox>(name, location, value, meta, expr));
    if(expr->position == analyze::expression_position::tail)
    {
      return ret(name, expression_type(expr));
    }
    return name;
  }

  identifier builder::cpp_new(identifier const &value, analyze::expr::cpp_new_ref const expr)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::cpp_new>(name, location, value, expr));
    if(expr->position == analyze::expression_position::tail)
    {
      return ret(name, expression_type(expr));
    }
    return name;
  }

  identifier builder::cpp_delete(identifier const &value, analyze::expr::cpp_delete_ref const expr)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::cpp_delete>(name, location, value, expr));
    if(expr->position == analyze::expression_position::tail)
    {
      return ret(name, expression_type(expr));
    }
    return name;
  }
}
