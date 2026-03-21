#include <jank/ir/processor.hpp>
#include <jank/ir/print.hpp>
#include <jank/analyze/expr/function.hpp>
#include <jank/codegen/llvm_processor.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/ns.hpp>
#include <jank/analyze/visit.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::ir
{
  struct builder
  {
    identifier next_ident()
    {
      return util::format("v{}", ident_count++);
    }

    jtl::option<identifier>
    literal(analyze::expression_position const pos, runtime::object_ref const value)
    {
      auto name{ next_ident() };
      block->instructions.emplace_back(jtl::make_ref<inst::literal>(name, value));
      if(pos == analyze::expression_position::tail)
      {
        return ret(name);
      }
      return name;
    }

    jtl::option<identifier> def(analyze::expression_position const pos,
                                jtl::immutable_string const &qualified_var,
                                jtl::option<identifier> const &value,
                                identifier const &meta)
    {
      auto name{ next_ident() };
      block->instructions.emplace_back(jtl::make_ref<inst::def>(name, qualified_var, value, meta));
      if(pos == analyze::expression_position::tail)
      {
        return ret(name);
      }
      return name;
    }

    jtl::option<identifier>
    var_deref(analyze::expression_position const pos, jtl::immutable_string const &qualified_var)
    {
      auto name{ next_ident() };
      block->instructions.emplace_back(jtl::make_ref<inst::var_deref>(name, qualified_var));
      if(pos == analyze::expression_position::tail)
      {
        return ret(name);
      }
      return name;
    }

    jtl::option<identifier>
    var_ref(analyze::expression_position const pos, jtl::immutable_string const &qualified_var)
    {
      auto name{ next_ident() };
      block->instructions.emplace_back(jtl::make_ref<inst::var_ref>(name, qualified_var));
      if(pos == analyze::expression_position::tail)
      {
        return ret(name);
      }
      return name;
    }

    jtl::option<identifier> dynamic_call(analyze::expression_position const pos,
                                         identifier const &fn,
                                         native_vector<identifier> &&args)
    {
      auto name{ next_ident() };
      block->instructions.emplace_back(
        jtl::make_ref<inst::dynamic_call>(name, fn, jtl::move(args)));
      if(pos == analyze::expression_position::tail)
      {
        return ret(name);
      }
      return name;
    }

    jtl::option<identifier> ret(identifier const &name) const
    {
      block->instructions.emplace_back(jtl::make_ref<inst::ret>(name));
      return none;
    }

    jtl::ref<block> block;
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
                 b.literal(analyze::expression_position::value, expr->name->meta).unwrap());
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
    b.dynamic_call(expr->position, fn_ident, jtl::move(arg_idents));
    return none;
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
    if(b.block->has_terminator())
    {
      return none;
    }

    return b.literal(expr->position, runtime::jank_nil());
  }

  jtl::option<identifier> gen(analyze::expr::if_ref const, builder &)
  {
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
      fn.blocks.emplace_back("entry");
      builder b{ &fn.blocks.back() };
      for(auto const expr : arity.body->values)
      {
        gen(expr, b);
      }

      util::println("{}", print(fn));
      fns.emplace_back(jtl::move(fn));
    }

    return fns;
  }
}
