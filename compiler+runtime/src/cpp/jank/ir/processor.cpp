#include <jank/ir/processor.hpp>
#include <jank/ir/print.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/truthy.hpp>
#include <jank/runtime/core/call.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/ns.hpp>
#include <jank/runtime/module/loader.hpp>
#include <jank/analyze/cpp_util.hpp>
#include <jank/analyze/visit.hpp>
#include <jank/ir/builder.hpp>
#include <jank/codegen/llvm_processor.hpp>
#include <jank/ui/highlight.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/scope_exit.hpp>
#include <jank/error/codegen.hpp>

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

  usize function::find_block(identifier const &name) const
  {
    for(usize i{}; i < blocks.size(); ++i)
    {
      if(blocks[i].name == name)
      {
        return i;
      }
    }
    jank_panic_fmt("Unable to find IR block '{}'.", name);
  }

  bool block::has_terminator() const
  {
    return !instructions.empty() && instructions.back()->is_terminator();
  }

  jtl::option<identifier> gen(analyze::expression_ref const expr, builder &b);

  jtl::option<identifier> gen(analyze::expr::def_ref const expr, builder &b)
  {
    jtl::option<identifier> value_ident;
    if(expr->value.is_some())
    {
      value_ident = gen(expr->value.unwrap(), b);
    }

    static auto const dynamic_kw{ runtime::__rt_ctx->intern_keyword("dynamic").expect_ok() };
    return b.def(expr->position,
                 expr->name->to_code_string(),
                 value_ident,
                 b.literal(analyze::expression_position::value, expr->name->meta),
                 runtime::truthy(runtime::get(expr->name->meta, dynamic_kw)));
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

  jtl::option<identifier> gen(analyze::expr::list_ref const expr, builder &b)
  {
    native_vector<identifier> values;
    values.reserve(expr->data_exprs.size());
    for(auto const value_expr : expr->data_exprs)
    {
      values.emplace_back(gen(value_expr, b).unwrap());
    }

    jtl::option<identifier> meta;
    if(!is_empty(expr->meta))
    {
      meta = b.literal(analyze::expression_position::value, expr->meta);
    }
    return b.persistent_list(expr->position, jtl::move(values), meta);
  }

  jtl::option<identifier> gen(analyze::expr::vector_ref const expr, builder &b)
  {
    native_vector<identifier> values;
    values.reserve(expr->data_exprs.size());
    for(auto const value_expr : expr->data_exprs)
    {
      values.emplace_back(gen(value_expr, b).unwrap());
    }

    jtl::option<identifier> meta;
    if(!is_empty(expr->meta))
    {
      meta = b.literal(analyze::expression_position::value, expr->meta);
    }
    return b.persistent_vector(expr->position, jtl::move(values), meta);
  }

  jtl::option<identifier> gen(analyze::expr::map_ref const expr, builder &b)
  {
    native_vector<std::pair<identifier, identifier>> values;
    values.reserve(expr->data_exprs.size());
    for(auto const value_expr : expr->data_exprs)
    {
      values.emplace_back(gen(value_expr.first, b).unwrap(), gen(value_expr.second, b).unwrap());
    }

    jtl::option<identifier> meta;
    if(!is_empty(expr->meta))
    {
      meta = b.literal(analyze::expression_position::value, expr->meta);
    }

    if(expr->data_exprs.size() <= runtime::obj::persistent_array_map::max_size)
    {
      return b.persistent_array_map(expr->position, jtl::move(values), meta);
    }
    return b.persistent_hash_map(expr->position, jtl::move(values), meta);
  }

  jtl::option<identifier> gen(analyze::expr::set_ref const expr, builder &b)
  {
    native_vector<identifier> values;
    values.reserve(expr->data_exprs.size());
    for(auto const value_expr : expr->data_exprs)
    {
      values.emplace_back(gen(value_expr, b).unwrap());
    }

    jtl::option<identifier> meta;
    if(!is_empty(expr->meta))
    {
      meta = b.literal(analyze::expression_position::value, expr->meta);
    }
    return b.persistent_hash_set(expr->position, jtl::move(values), meta);
  }

  jtl::option<identifier> gen(analyze::expr::local_reference_ref const expr, builder &b)
  {
    auto const &local_name{ expr->name->get_name() };
    auto const local{ b.locals.find(local_name) };
    if(local != b.locals.end())
    {
      if(expr->position == analyze::expression_position::tail)
      {
        return b.ret(local->second, expression_type(expr));
      }
      return local->second;
    }

    if(b.allowed_defers.contains(local_name))
    {
      return ":defer";
    }

    return b.locals[local_name] = b.parameter(expr->position, local_name);
  }

  jtl::immutable_string gen_arity(module &mod,
                                  analyze::expr::function_ref const fn_expr,
                                  analyze::expr::function_arity const &arity)
  {
    auto &fn{ mod.functions.emplace_back(&arity) };
    fn.name = runtime::munge(util::format("{}_{}", fn_expr->unique_name, arity.params.size()));
    fn.add_block("entry");
    builder b{ &mod, mod.functions.size() - 1 };

    for(auto const &capture : arity.frame->captures)
    {
      auto const &name{ capture.first->get_name() };
      b.locals[name]
        = b.capture(analyze::expression_position::value, capture.second.binding.type, name);
    }

    if(arity.fn_ctx->is_recur_recursive)
    {
      for(auto const param : arity.params)
      {
        auto const shadow{ b.next_shadow() };
        auto const &name{ param->get_name() };
        b.locals[name] = b.parameter(analyze::expression_position::value, name);
        b.local_to_loop_shadow[name] = shadow;
        b.branch_set(shadow, b.locals[name]);
      }

      auto const recur_blk{ b.block(b.next_ident("recur")) };
      b.fn_recur_target = recur_blk;
      b.jump(recur_blk);
      b.enter_block(recur_blk);

      for(auto const param : arity.params)
      {
        auto const &name{ param->get_name() };
        b.locals[name] = b.branch_get(b.local_to_loop_shadow[name], untyped_object_ref_type());
      }
    }

    for(auto const expr : arity.body->values)
    {
      gen(expr, b);
    }

    if(arity.body->values.empty())
    {
      b.literal(analyze::expression_position::tail, runtime::jank_nil());
    }

    return fn.name;
  }

  static runtime::callable_arity_flags
  arity_flags(native_vector<analyze::expr::function_arity> const &arities)
  {
    analyze::expr::function_arity const *variadic_arity{};
    analyze::expr::function_arity const *highest_fixed_arity{};
    for(auto const &arity : arities)
    {
      if(arity.fn_ctx->is_variadic)
      {
        variadic_arity = &arity;
      }
      else if(!highest_fixed_arity
              || highest_fixed_arity->fn_ctx->param_count < arity.fn_ctx->param_count)
      {
        highest_fixed_arity = &arity;
      }
    }

    if(variadic_arity)
    {
      bool const variadic_ambiguous{ variadic_arity && highest_fixed_arity
                                     && highest_fixed_arity->fn_ctx->param_count
                                       == variadic_arity->fn_ctx->param_count - 1 };

      return runtime::build_arity_flags(variadic_arity->fn_ctx->param_count - 1,
                                        true,
                                        variadic_ambiguous);
    }
    jank_assert(highest_fixed_arity != nullptr);
    return runtime::build_arity_flags(highest_fixed_arity->fn_ctx->param_count, false, false);
  }

  jtl::option<identifier> gen(analyze::expr::function_ref const expr, builder &b)
  {
    native_unordered_map<u8, jtl::immutable_string> arities;
    auto const flags{ arity_flags(expr->arities) };
    for(auto const &arity : expr->arities)
    {
      arities[arity.params.size()] = gen_arity(*b.mod, expr, arity);
    }

    auto const captures{ expr->captures() };
    auto const is_closure{ !captures.empty() };
    if(is_closure)
    {
      native_unordered_map<jtl::immutable_string, identifier> captured_idents;
      for(auto const &capture : captures)
      {
        auto const &name{ capture.first->get_name() };
        analyze::expr::local_reference const local_ref{ analyze::expression_position::value,
                                                        expr->frame,
                                                        expr->needs_box,
                                                        capture.first,
                                                        capture.second };
        captured_idents[name] = gen(analyze::expr::local_reference_ref{ &local_ref }, b).unwrap();
      }
      return b.closure(expr->position,
                       runtime::munge(expr->unique_name + "_ctx"),
                       jtl::move(arities),
                       jtl::move(captured_idents),
                       flags);
    }

    return b.function(expr->position, jtl::move(arities), flags);
  }

  jtl::option<identifier> gen(analyze::expr::recur_ref const expr, builder &b)
  {
    native_vector<identifier> arg_idents;
    arg_idents.reserve(expr->arg_exprs.size());
    for(auto const arg_expr : expr->arg_exprs)
    {
      arg_idents.emplace_back(gen(arg_expr, b).unwrap());
    }

    if(expr->loop_target.is_some())
    {
      auto const loop{ expr->loop_target.unwrap() };
      for(usize i{}; i < loop->pairs.size(); ++i)
      {
        auto const &name{ loop->pairs[i].first->name->get_name() };
        b.branch_set(b.local_to_loop_shadow[name], arg_idents[i]);
      }
      return b.jump(b.loop_recur_target.unwrap());
    }
    else
    {
      for(usize i{}; i < b.current_function()->arity->params.size(); ++i)
      {
        auto const shadow{ b.next_shadow() };
        auto const &name{ b.current_function()->arity->params[i]->get_name() };
        b.branch_set(b.local_to_loop_shadow[name], arg_idents[i]);
      }
      return b.jump(b.fn_recur_target.unwrap());
    }
  }

  jtl::option<identifier> gen(analyze::expr::recursion_reference_ref const expr, builder &b)
  {
    if(expr->fn_ctx != b.current_function()->arity->fn_ctx)
    {
      auto name{ b.locals[expr->fn_ctx->name] };
      if(expr->position == analyze::expression_position::tail)
      {
        return b.ret(name, expression_type(expr));
      }
      return name;
    }
    return b.recursion_reference(expr->position);
  }

  jtl::option<identifier> gen(analyze::expr::named_recursion_ref const expr, builder &b)
  {
    native_vector<identifier> arg_idents;
    arg_idents.reserve(expr->arg_exprs.size());
    for(auto const arg_expr : expr->arg_exprs)
    {
      arg_idents.emplace_back(gen(arg_expr, b).unwrap());
    }

    return b.named_recursion(
      expr->position,
      gen(analyze::expr::recursion_reference_ref{ &expr->recursion_ref }, b).unwrap(),
      jtl::move(arg_idents));
  }

  jtl::option<identifier> gen(analyze::expr::let_ref const expr, builder &b)
  {
    for(auto const &pair : expr->pairs)
    {
      b.locals[pair.first->name->get_name()] = gen(pair.second, b).unwrap();
    }

    if(expr->is_loop)
    {
      for(auto const &pair : expr->pairs)
      {
        auto const shadow{ b.next_shadow() };
        auto const &name{ pair.first->name->get_name() };
        b.local_to_loop_shadow[name] = shadow;
        b.branch_set(shadow, b.locals[name]);
      }

      auto const loop_blk{ b.block(b.next_ident("loop")) };
      auto old_current_loop{ b.loop_recur_target };
      util::scope_exit const finally{ [&] { b.loop_recur_target = jtl::move(old_current_loop); } };
      b.loop_recur_target = loop_blk;
      b.jump(loop_blk);
      b.enter_block(loop_blk);

      for(auto const &pair : expr->pairs)
      {
        auto const &name{ pair.first->name->get_name() };
        b.locals[name] = b.branch_get(b.local_to_loop_shadow[name], expression_type(pair.second));
      }

      return gen(expr->body, b);
    }
    else
    {
      return gen(expr->body, b);
    }
  }

  jtl::option<identifier> gen(analyze::expr::letfn_ref const expr, builder &b)
  {
    auto old_allowed_defers{ b.allowed_defers };
    b.allowed_defers.clear();
    auto old_locals{ b.locals };
    util::scope_exit const finally{ [&] {
      b.allowed_defers = jtl::move(old_allowed_defers);
      b.locals = jtl::move(old_locals);
    } };

    native_vector<jtl::immutable_string> bindings;
    bindings.reserve(expr->pairs.size());
    for(auto const &pair : expr->pairs)
    {
      auto const &name{ pair.first->name->get_name() };
      bindings.emplace_back(name);
      b.allowed_defers.insert(name);
    }

    b.letfn(jtl::move(bindings));

    for(auto const &pair : expr->pairs)
    {
      auto const &name{ pair.first->name->get_name() };
      b.locals[name] = gen(pair.second, b).unwrap();
    }

    return gen(expr->body, b);
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
    auto const shadow{ b.next_shadow() };

    identifier bool_condition{ condition_name };
    if(is_any_object(expression_type(expr->condition)))
    {
      bool_condition = b.truthy(bool_condition);
    }
    b.branch(bool_condition,
             b.block_name(then_blk),
             b.block_name(else_blk),
             (expr->position != analyze::expression_position::tail) ? b.block_name(merge_blk)
                                                                    : jtl::option<identifier>{},
             (expr->position != analyze::expression_position::tail)
               ? inst::branch::shadow_details{ shadow, expression_type(expr->then) }
               : jtl::option<inst::branch::shadow_details>{});

    b.enter_block(then_blk);
    auto const then_name{ gen(expr->then, b) };
    if(expr->position != analyze::expression_position::tail && !b.current_block()->has_terminator())
    {
      b.branch_set(shadow, then_name.unwrap());
      b.jump(merge_blk);
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
    if(expr->position != analyze::expression_position::tail && !b.current_block()->has_terminator())
    {
      b.branch_set(shadow, else_name);
      b.jump(merge_blk);
    }

    if(expr->position != analyze::expression_position::tail)
    {
      b.enter_block(merge_blk);
      return b.branch_get(shadow, expression_type(expr));
    }

    b.remove_block(merge_blk);
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::throw_ref const expr, builder &b)
  {
    /* TODO: Any code after this shouldn't be added to the block. */
    return b.throw_(gen(expr->value, b).unwrap());
  }

  jtl::option<identifier> gen(analyze::expr::try_ref const expr, builder &b)
  {
    auto const try_blk{ b.block(b.next_ident("try")) };
    b.jump(try_blk);

    auto const merge_blk{ b.block(b.next_ident("merge")) };
    auto const shadow{ b.next_shadow() };
    auto const original_pos{ expr->position };

    auto jump_blk{ merge_blk };
    jtl::option<identifier> finally_blk;
    if(expr->finally_body.is_some())
    {
      auto const blk{ b.block(b.next_ident("finally")) };
      jump_blk = blk;
      finally_blk = b.block_name(blk);
      b.enter_block(blk);
      b.finally(b.block_name(merge_blk));

      gen(expr->finally_body.unwrap(), b);
      b.jump(merge_blk);

      expr->propagate_position(analyze::expression_position::value);
    }

    auto const needs_merge_blk{ original_pos != analyze::expression_position::tail
                                || finally_blk.is_some() };

    native_vector<std::pair<jtl::ptr<void>, identifier>> catch_blocks;
    catch_blocks.reserve(expr->catch_bodies.size());
    for(auto const catch_ : expr->catch_bodies)
    {
      auto const catch_blk{ b.block(b.next_ident("catch")) };
      b.enter_block(catch_blk);
      auto old_locals{ b.locals };
      util::scope_exit const finally{ [&] { b.locals = jtl::move(old_locals); } };
      b.locals[catch_.sym->get_name()]
        = b.catch_(catch_.type,
                   needs_merge_blk ? b.block_name(merge_blk) : jtl::option<identifier>{},
                   needs_merge_blk ? shadow : jtl::option<identifier>{},
                   finally_blk);

      auto const catch_res{ gen(catch_.body, b) };
      catch_blocks.emplace_back(catch_.type, b.block_name(catch_blk));

      if(expr->position != analyze::expression_position::tail)
      {
        b.branch_set(shadow, catch_res.unwrap());
        b.jump(jump_blk);
      }
    }

    b.enter_block(try_blk);
    b.try_(jtl::move(catch_blocks),
           needs_merge_blk ? b.block_name(merge_blk) : jtl::option<identifier>{},
           needs_merge_blk ? shadow : jtl::option<identifier>{},
           finally_blk);

    auto try_res{ gen(expr->body, b) };

    if(needs_merge_blk && !b.current_block()->has_terminator())
    {
      b.branch_set(shadow, try_res.unwrap());
      b.jump(jump_blk);
    }

    if(needs_merge_blk)
    {
      b.enter_block(merge_blk);
      auto phi{ b.branch_get(shadow, untyped_object_ref_type()) };
      if(original_pos == analyze::expression_position::tail)
      {
        return b.ret(phi, untyped_object_ref_type());
      }
      return phi;
    }

    b.remove_block(merge_blk);
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::case_ref const expr, builder &b)
  {
    auto const value_ident{ gen(expr->value_expr, b).unwrap() };
    auto const starting_block{ b.current_block()->index };
    auto const shadow{ b.next_shadow() };
    auto const merge_blk{ b.block(b.next_ident("merge")) };

    native_unordered_map<i64, identifier> cases;
    for(usize i{}; i < expr->keys.size(); ++i)
    {
      auto const block{ b.block(b.next_ident("case")) };
      cases[expr->keys[i]] = b.block_name(block);
      b.enter_block(block);
      auto const case_res{ gen(expr->exprs[i], b) };

      if(expr->position != analyze::expression_position::tail
         && !b.current_block()->has_terminator())
      {
        b.branch_set(shadow, case_res.unwrap());
        b.jump(merge_blk);
      }
    }

    auto const default_block{ b.block(b.next_ident("default")) };
    b.enter_block(default_block);
    auto const default_res{ gen(expr->default_expr, b) };

    if(expr->position != analyze::expression_position::tail && !b.current_block()->has_terminator())
    {
      b.branch_set(shadow, default_res.unwrap());
      b.jump(merge_blk);
    }

    b.enter_block(starting_block);
    b.case_(expr->shift,
            expr->mask,
            value_ident,
            jtl::move(cases),
            b.block_name(default_block),
            (expr->position != analyze::expression_position::tail) ? b.block_name(merge_blk)
                                                                   : jtl::option<identifier>{},
            (expr->position != analyze::expression_position::tail) ? shadow
                                                                   : jtl::option<identifier>{});

    if(expr->position != analyze::expression_position::tail)
    {
      b.enter_block(merge_blk);
      return b.branch_get(shadow, untyped_object_ref_type());
    }

    b.remove_block(merge_blk);
    return none;
  }

  jtl::option<identifier> gen(analyze::expr::cpp_raw_ref const expr, builder &b)
  {
    return b.cpp_raw(expr);
  }

  jtl::option<identifier> gen(analyze::expr::cpp_type_ref const, builder &)
  {
    throw error::internal_codegen_failure(
      "A cpp_type expression was found during codegen, but that should not be possible.");
  }

  jtl::option<identifier> gen(analyze::expr::cpp_value_ref const expr, builder &b)
  {
    return b.cpp_value(expr);
  }

  jtl::option<identifier> gen(analyze::expr::cpp_conversion_ref const expr, builder &b)
  {
    return b.cpp_conversion(gen(expr->value_expr, b).unwrap(), expr);
  }

  jtl::option<identifier> gen(analyze::expr::cpp_unsafe_cast_ref const expr, builder &b)
  {
    return b.cpp_unsafe_cast(gen(expr->value_expr, b).unwrap(), expr);
  }

  jtl::option<identifier> gen(analyze::expr::cpp_call_ref const expr, builder &b)
  {
    native_vector<identifier> args;
    args.reserve(expr->arg_exprs.size());
    for(auto const arg : expr->arg_exprs)
    {
      args.emplace_back(gen(arg, b).unwrap());
    }

    if(expr->source_expr->kind == analyze::expression_kind::cpp_value)
    {
      return b.cpp_call(none, jtl::move(args), expr);
    }
    return b.cpp_call(gen(expr->source_expr, b).unwrap(), jtl::move(args), expr);
  }

  jtl::option<identifier> gen(analyze::expr::cpp_constructor_call_ref const expr, builder &b)
  {
    native_vector<identifier> args;
    args.reserve(expr->arg_exprs.size());
    for(auto const arg : expr->arg_exprs)
    {
      args.emplace_back(gen(arg, b).unwrap());
    }
    return b.cpp_constructor_call(jtl::move(args), expr);
  }

  jtl::option<identifier> gen(analyze::expr::cpp_member_call_ref const expr, builder &b)
  {
    native_vector<identifier> args;
    args.reserve(expr->arg_exprs.size());
    for(auto const arg : expr->arg_exprs)
    {
      args.emplace_back(gen(arg, b).unwrap());
    }
    return b.cpp_member_call(jtl::move(args), expr);
  }

  jtl::option<identifier> gen(analyze::expr::cpp_member_access_ref const expr, builder &b)
  {
    return b.cpp_member_access(gen(expr->obj_expr, b).unwrap(), expr);
  }

  jtl::option<identifier> gen(analyze::expr::cpp_builtin_operator_call_ref const expr, builder &b)
  {
    native_vector<identifier> args;
    args.reserve(expr->arg_exprs.size());
    for(auto const arg : expr->arg_exprs)
    {
      args.emplace_back(gen(arg, b).unwrap());
    }
    return b.cpp_builtin_operator_call(jtl::move(args), expr);
  }

  jtl::option<identifier> gen(analyze::expr::cpp_box_ref const expr, builder &b)
  {
    return b.cpp_box(gen(expr->value_expr, b).unwrap(), expr);
  }

  jtl::option<identifier> gen(analyze::expr::cpp_unbox_ref const expr, builder &b)
  {
    return b.cpp_unbox(gen(expr->value_expr, b).unwrap(), expr);
  }

  jtl::option<identifier> gen(analyze::expr::cpp_new_ref const expr, builder &b)
  {
    return b.cpp_new(gen(expr->value_expr, b).unwrap(), expr);
  }

  jtl::option<identifier> gen(analyze::expr::cpp_delete_ref const expr, builder &b)
  {
    return b.cpp_delete(gen(expr->value_expr, b).unwrap(), expr);
  }

  jtl::option<identifier> gen(analyze::expression_ref const expr, builder &b)
  {
    jtl::option<identifier> name;
    visit_expr([&](auto const typed_ex) { name = gen(typed_ex, b); }, expr);
    return name;
  }

  module create(analyze::expr::function_ref const fn_expr,
                jtl::immutable_string const &module_name,
                [[maybe_unused]] codegen::compilation_target const target)
  {
    native_vector<jtl::immutable_string> entry_points;
    entry_points.reserve(fn_expr->arities.size());
    for(auto const &arity : fn_expr->arities)
    {
      entry_points.emplace_back(
        runtime::munge(util::format("{}_{}", fn_expr->unique_name, arity.params.size())));
    }
    module mod{ module_name, arity_flags(fn_expr->arities), fn_expr, jtl::move(entry_points) };

    for(auto const &arity : fn_expr->arities)
    {
      gen_arity(mod, fn_expr, arity);
    }

    jtl::immutable_string_view const print_settings{ getenv("JANK_PRINT_IR") ?: "" };
    if(print_settings == "1")
    {
      util::println("{}", ui::highlight_str(runtime::module::file_view{ "ir.jank", print(mod) }));
    }
    //util::println("{}", print(mod));

    return mod;
  }
}
