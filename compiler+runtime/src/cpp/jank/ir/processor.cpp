#include <jank/ir/processor.hpp>
#include <jank/ir/print.hpp>
#include <jank/analyze/expr/function.hpp>
#include <jank/analyze/cpp_util.hpp>
#include <jank/codegen/llvm_processor.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/ns.hpp>
#include <jank/analyze/visit.hpp>
#include <jank/runtime/module/loader.hpp>
#include <jank/ui/highlight.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::ir
{
  using namespace analyze::cpp_util;

  usize function::add_block(identifier const &name)
  {
    auto const index{ blocks.size() };
    blocks.emplace_back(index, name);
    return index;
  }

  void function::remove_block(usize const index)
  {
    blocks.erase(blocks.begin() + index);
    for(auto i{ index }; i != blocks.size(); ++i)
    {
      --blocks[i].index;
    }
  }

  struct builder
  {
    identifier next_ident()
    {
      return next_ident("v");
    }

    identifier next_ident(jtl::immutable_string const &prefix)
    {
      return util::format("{}{}", prefix, ident_count++);
    }

    jtl::ref<block> current_block() const
    {
      return &fn->blocks[block_index];
    }

    usize block(identifier const &name) const
    {
      return fn->add_block(name);
    }

    void remove_block(usize const block_index) const
    {
      fn->remove_block(block_index);
    }

    void enter_block(usize const blk_index)
    {
      block_index = blk_index;
    }

    identifier literal(analyze::expression_position const pos, runtime::object_ref const value)
    {
      auto name{ next_ident() };
      auto const type{ literal_type(value) };
      fn->blocks[block_index].instructions.emplace_back(
        jtl::make_ref<inst::literal>(name, type, value));
      if(pos == analyze::expression_position::tail)
      {
        return ret(name, type);
      }
      return name;
    }

    identifier def(analyze::expression_position const pos,
                   jtl::immutable_string const &qualified_var,
                   jtl::option<identifier> const &value,
                   identifier const &meta)
    {
      auto name{ next_ident() };
      auto const type{ var_type() };
      fn->blocks[block_index].instructions.emplace_back(
        jtl::make_ref<inst::def>(name, type, qualified_var, value, meta));
      if(pos == analyze::expression_position::tail)
      {
        return ret(name, type);
      }
      return name;
    }

    identifier
    var_deref(analyze::expression_position const pos, jtl::immutable_string const &qualified_var)
    {
      auto name{ next_ident() };
      auto const type{ untyped_object_ref_type() };
      fn->blocks[block_index].instructions.emplace_back(
        jtl::make_ref<inst::var_deref>(name, type, qualified_var));
      if(pos == analyze::expression_position::tail)
      {
        return ret(name, type);
      }
      return name;
    }

    identifier
    var_ref(analyze::expression_position const pos, jtl::immutable_string const &qualified_var)
    {
      auto name{ next_ident() };
      auto const type{ var_type() };
      fn->blocks[block_index].instructions.emplace_back(
        jtl::make_ref<inst::var_ref>(name, type, qualified_var));
      if(pos == analyze::expression_position::tail)
      {
        return ret(name, type);
      }
      return name;
    }

    identifier dynamic_call(analyze::expression_position const pos,
                            identifier const &fn,
                            native_vector<identifier> &&args)
    {
      auto name{ next_ident() };
      auto const type{ untyped_object_ref_type() };
      this->fn->blocks[block_index].instructions.emplace_back(
        jtl::make_ref<inst::dynamic_call>(name, type, fn, jtl::move(args)));
      if(pos == analyze::expression_position::tail)
      {
        return ret(name, type);
      }
      return name;
    }

    identifier truthy(identifier const &value)
    {
      auto name{ next_ident() };
      this->fn->blocks[block_index].instructions.emplace_back(
        jtl::make_ref<inst::truthy>(name, value));
      return name;
    }

    identifier jump(identifier const &block)
    {
      auto name{ next_ident() };
      fn->blocks[block_index].instructions.emplace_back(jtl::make_ref<inst::jump>(name, block));
      return name;
    }

    identifier branch_set(identifier const &shadow, identifier const &value)
    {
      auto name{ next_ident() };
      fn->blocks[block_index].instructions.emplace_back(
        jtl::make_ref<inst::branch_set>(name, shadow, value));
      return name;
    }

    identifier branch_get(identifier const &name, jtl::ptr<void> const type) const
    {
      fn->blocks[block_index].instructions.emplace_back(
        jtl::make_ref<inst::branch_get>(name, type));
      return name;
    }

    identifier
    branch(identifier const &condition, identifier const &then_blk, identifier const &else_blk)
    {
      auto name{ next_ident() };
      fn->blocks[block_index].instructions.emplace_back(
        jtl::make_ref<inst::branch>(name, condition, then_blk, else_blk));
      return name;
    }

    identifier ret(identifier const &value, jtl::ptr<void> const type)
    {
      auto name{ next_ident() };
      fn->blocks[block_index].instructions.emplace_back(
        jtl::make_ref<inst::ret>(name, type, value));
      return name;
    }

    jtl::ref<function> fn;
    usize block_index{};
    native_unordered_map<jtl::immutable_string, identifier> locals{};
    usize ident_count{};
  };

  bool block::has_terminator() const
  {
    return !instructions.empty() && instructions.back()->is_terminator();
  }

  jtl::option<identifier> gen(analyze::expr::def_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::var_deref_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::var_ref_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::call_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::primitive_literal_ref const expr, builder &b);
  jtl::option<identifier> gen(analyze::expr::list_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::vector_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::map_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::set_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::local_reference_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::function_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::recur_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::recursion_reference_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::named_recursion_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::let_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::letfn_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::do_ref const expr, builder &b);
  jtl::option<identifier> gen(analyze::expr::if_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::throw_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::try_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::case_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::cpp_raw_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::cpp_type_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::cpp_value_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::cpp_cast_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::cpp_unsafe_cast_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::cpp_call_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::cpp_constructor_call_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::cpp_member_call_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::cpp_member_access_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::cpp_builtin_operator_call_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::cpp_box_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::cpp_unbox_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::cpp_new_ref const, builder &);
  jtl::option<identifier> gen(analyze::expr::cpp_delete_ref const, builder &);
  jtl::option<identifier> gen(analyze::expression_ref const expr, builder &b);

  jtl::option<identifier> gen(analyze::expr::def_ref const expr, builder &b)
  {
    jtl::option<identifier> value_ident;
    if(expr->value.is_some())
    {
      value_ident = gen(expr->value.unwrap(), b);
    }
    return b.def(expr->position,
                 expr->name->to_code_string(),
                 value_ident,
                 b.literal(analyze::expression_position::value, expr->name->meta));
  }

  jtl::option<identifier> gen(analyze::expr::var_deref_ref const expr, builder &b)
  {
    return b.var_deref(expr->position,
                       util::format("{}/{}", expr->var->n->name->name, expr->var->name->name));
  }

  jtl::option<identifier> gen(analyze::expr::var_ref_ref const expr, builder &b)
  {
    return b.var_ref(expr->position,
                     util::format("{}/{}", expr->var->n->name->name, expr->var->name->name));
  }

  jtl::option<identifier> gen(analyze::expr::call_ref const expr, builder &b)
  {
    native_vector<identifier> arg_idents;
    arg_idents.reserve(expr->arg_exprs.size());
    for(auto const arg_expr : expr->arg_exprs)
    {
      arg_idents.emplace_back(gen(arg_expr, b).unwrap());
    }

    auto const fn_ident{ gen(expr->source_expr, b).unwrap() };
    return b.dynamic_call(expr->position, fn_ident, jtl::move(arg_idents));
  }

  jtl::option<identifier> gen(analyze::expr::primitive_literal_ref const expr, builder &b)
  {
    return b.literal(expr->position, expr->data);
  }

  jtl::option<identifier> gen(analyze::expr::list_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::vector_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::map_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::set_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::local_reference_ref const expr, builder &b)
  {
    return b.locals[expr->name->to_code_string()];
  }

  jtl::option<identifier> gen(analyze::expr::function_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::recur_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::recursion_reference_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::named_recursion_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::let_ref const expr, builder &b)
  {
    for(auto const &pair : expr->pairs)
    {
      b.locals[pair.first->name->to_code_string()] = gen(pair.second, b).unwrap();
    }

    return gen(expr->body, b);
  }

  jtl::option<identifier> gen(analyze::expr::letfn_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::do_ref const expr, builder &b)
  {
    jtl::option<identifier> name;
    for(auto const ex : expr->values)
    {
      name = gen(ex, b);
    }

    if(name.is_some())
    {
      return name;
    }
    if(b.current_block()->has_terminator())
    {
      return none;
    }

    return b.literal(expr->position, runtime::jank_nil());
  }

  jtl::option<identifier> gen(analyze::expr::if_ref const expr, builder &b)
  {
    auto const then_blk{ b.block(b.next_ident("if")) };
    auto const else_blk{ b.block(b.next_ident("else")) };
    auto const merge_blk{ b.block(b.next_ident("merge")) };
    auto const condition_name{ gen(expr->condition, b).unwrap() };
    auto const shadow{ b.next_ident("shadow") };

    identifier bool_condition{ condition_name };
    if(is_any_object(expression_type(expr->condition)))
    {
      bool_condition = b.truthy(bool_condition);
    }
    b.branch(bool_condition, b.fn->blocks[then_blk].name, b.fn->blocks[else_blk].name);

    b.enter_block(then_blk);
    auto const then_name{ gen(expr->then, b).unwrap() };
    if(expr->position != analyze::expression_position::tail)
    {
      b.branch_set(shadow, then_name);
      b.jump(b.fn->blocks[merge_blk].name);
    }

    b.enter_block(else_blk);
    identifier else_name;
    if(expr->else_.is_some())
    {
      else_name = gen(expr->else_.unwrap(), b).unwrap();
    }
    else
    {
      else_name = b.literal(expr->position, runtime::jank_nil());
    }
    if(expr->position != analyze::expression_position::tail)
    {
      b.branch_set(shadow, else_name);
      b.jump(b.fn->blocks[merge_blk].name);
    }

    if(expr->position != analyze::expression_position::tail)
    {
      b.enter_block(merge_blk);
      return b.branch_get(shadow, expression_type(expr));
    }

    b.remove_block(merge_blk);

    return none;
  }

  jtl::option<identifier> gen(analyze::expr::throw_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::try_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::case_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::cpp_raw_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::cpp_type_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::cpp_value_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::cpp_cast_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::cpp_unsafe_cast_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::cpp_call_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::cpp_constructor_call_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::cpp_member_call_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::cpp_member_access_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::cpp_builtin_operator_call_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::cpp_box_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::cpp_unbox_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::cpp_new_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::cpp_delete_ref const, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(analyze::expression_ref const expr, builder &b)
  {
    jtl::option<identifier> name;
    visit_expr([&](auto const typed_ex) { name = gen(typed_ex, b); }, expr);
    return name;
  }

  native_vector<function> create(analyze::expr::function_ref const fn_expr,
                                 [[maybe_unused]] jtl::immutable_string const &module,
                                 [[maybe_unused]] codegen::compilation_target const target)
  {
    native_vector<function> fns;

    for(auto const &arity : fn_expr->arities)
    {
      function fn;
      fn.name = fn_expr->unique_name;
      fn.add_block("entry");
      builder b{ &fn };
      for(auto const expr : arity.body->values)
      {
        gen(expr, b);
      }

      if(arity.body->values.empty())
      {
        b.literal(analyze::expression_position::tail, runtime::jank_nil());
      }

      util::println("{}", ui::highlight_str(runtime::module::file_view{ "ir.jank", print(fn) }));
      fns.emplace_back(jtl::move(fn));
    }

    return fns;
  }
}
