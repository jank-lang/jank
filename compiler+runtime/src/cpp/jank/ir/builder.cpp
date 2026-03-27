#include <jank/ir/builder.hpp>
#include <jank/util/fmt.hpp>
#include <jank/analyze/cpp_util.hpp>

namespace jank::ir
{
  using namespace analyze::cpp_util;

  identifier builder::next_ident()
  {
    return next_ident("v");
  }

  identifier builder::next_ident(jtl::immutable_string const &prefix)
  {
    return util::format("{}{}", prefix, ident_count++);
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

  void builder::remove_block(usize const block_index) const
  {
    current_function()->remove_block(block_index);
  }

  void builder::enter_block(usize const blk_index)
  {
    block_index = blk_index;
  }

  identifier
  builder::parameter(analyze::expression_position const pos, jtl::immutable_string const &value)
  {
    auto name{ next_ident() };
    auto const type{ untyped_object_ref_type() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::parameter>(name, type, value));
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
      jtl::make_ref<inst::capture>(name, type, value));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, type);
    }
    return name;
  }

  identifier
  builder::literal(analyze::expression_position const pos, runtime::object_ref const value)
  {
    auto name{ next_ident() };
    auto const type{ literal_type(value) };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::literal>(name, type, value));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, type);
    }
    return name;
  }

  identifier builder::persistent_list(analyze::expression_position const pos,
                                      native_vector<identifier> &&values)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::persistent_list>(name, jtl::move(values)));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, current_function()->blocks[block_index].instructions.back()->type);
    }
    return name;
  }

  identifier builder::persistent_vector(analyze::expression_position const pos,
                                        native_vector<identifier> &&values)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::persistent_vector>(name, jtl::move(values)));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, current_function()->blocks[block_index].instructions.back()->type);
    }
    return name;
  }

  identifier
  builder::persistent_array_map(analyze::expression_position const pos,
                                native_vector<std::pair<identifier, identifier>> &&values)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::persistent_array_map>(name, jtl::move(values)));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, current_function()->blocks[block_index].instructions.back()->type);
    }
    return name;
  }

  identifier builder::persistent_hash_map(analyze::expression_position const pos,
                                          native_vector<std::pair<identifier, identifier>> &&values)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::persistent_hash_map>(name, jtl::move(values)));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, current_function()->blocks[block_index].instructions.back()->type);
    }
    return name;
  }

  identifier builder::persistent_hash_set(analyze::expression_position const pos,
                                          native_vector<identifier> &&values)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::persistent_hash_set>(name, jtl::move(values)));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, current_function()->blocks[block_index].instructions.back()->type);
    }
    return name;
  }

  identifier builder::function(analyze::expression_position const pos,
                               native_unordered_map<u8, jtl::immutable_string> &&arities)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::function>(name, jtl::move(arities)));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, current_function()->blocks[block_index].instructions.back()->type);
    }
    return name;
  }

  identifier builder::closure(analyze::expression_position const pos,
                              native_unordered_map<u8, jtl::immutable_string> &&arities,
                              native_unordered_map<jtl::immutable_string, identifier> &&captures)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::closure>(name, jtl::move(arities), jtl::move(captures)));
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
      jtl::make_ref<inst::letfn>(name, jtl::move(bindings)));
    return name;
  }

  identifier builder::def(analyze::expression_position const pos,
                          jtl::immutable_string const &qualified_var,
                          jtl::option<identifier> const &value,
                          identifier const &meta)
  {
    auto name{ next_ident() };
    auto const type{ var_type() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::def>(name, type, qualified_var, value, meta));
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
    auto const type{ untyped_object_ref_type() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::var_deref>(name, type, qualified_var));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, type);
    }
    return name;
  }

  identifier builder::var_ref(analyze::expression_position const pos,
                              jtl::immutable_string const &qualified_var)
  {
    auto name{ next_ident() };
    auto const type{ var_type() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::var_ref>(name, type, qualified_var));
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
      jtl::make_ref<inst::dynamic_call>(name, type, fn, jtl::move(args)));
    if(pos == analyze::expression_position::tail)
    {
      return ret(name, type);
    }
    return name;
  }

  identifier
  builder::named_recursion(analyze::expression_position const pos, native_vector<identifier> &&args)
  {
    auto name{ next_ident() };
    auto const type{ untyped_object_ref_type() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::named_recursion>(name, type, jtl::move(args)));
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
      jtl::make_ref<inst::recursion_reference>(name, type));
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
      jtl::make_ref<inst::truthy>(name, value));
    return name;
  }

  identifier builder::jump(usize const index)
  {
    auto name{ next_ident() };
    auto const &block{ current_function()->blocks[index].name };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::jump>(name, block));
    return name;
  }

  identifier builder::branch_set(identifier const &shadow, identifier const &value)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::branch_set>(name, shadow, value));
    return name;
  }

  identifier builder::branch_get(identifier const &name, jtl::ptr<void> const type) const
  {
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::branch_get>(name, type));
    return name;
  }

  identifier builder::branch(identifier const &condition,
                             identifier const &then_blk,
                             identifier const &else_blk)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::branch>(name, condition, then_blk, else_blk));
    return name;
  }

  identifier builder::case_(identifier const &value,
                            native_unordered_map<i64, identifier> &&cases,
                            identifier const &default_block)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::case_>(name, value, jtl::move(cases), default_block));
    return name;
  }

  identifier builder::try_(native_vector<std::pair<jtl::ptr<void>, identifier>> &&catches)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::try_>(name, jtl::move(catches)));
    return name;
  }

  identifier builder::catch_(jtl::ptr<void> const type)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::catch_>(name, type));
    return name;
  }

  identifier builder::throw_(identifier const &value)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::throw_>(name, value));
    return name;
  }

  identifier builder::ret(identifier const &value, jtl::ptr<void> const type)
  {
    auto name{ next_ident() };
    current_function()->blocks[block_index].instructions.emplace_back(
      jtl::make_ref<inst::ret>(name, type, value));
    return name;
  }
}
