#include <set>

#include <fmt/core.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/behavior/number_like.hpp>
#include <jank/runtime/behavior/sequential.hpp>
#include <jank/runtime/behavior/map_like.hpp>
#include <jank/runtime/behavior/set_like.hpp>
#include <jank/runtime/core/truthy.hpp>
#include <jank/runtime/core.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/analyze/expr/primitive_literal.hpp>
#include <jank/analyze/step/force_boxed.hpp>
#include <jank/evaluate.hpp>
#include <jank/result.hpp>
#include <jank/util/scope_exit.hpp>
#include <jank/error/analyze.hpp>

namespace jank::analyze
{
  processor::processor(runtime::context &rt_ctx)
    : rt_ctx{ rt_ctx }
    , root_frame{ make_box<local_frame>(local_frame::frame_type::root, rt_ctx, none) }
  {
    using runtime::obj::symbol;
    auto const make_fn = [this](auto const fn) -> decltype(specials)::mapped_type {
      return [this, fn](auto const &list,
                        auto &current_frame,
                        auto const position,
                        auto const &fn_ctx,
                        auto const needs_box) {
        return (this->*fn)(list, current_frame, position, fn_ctx, needs_box);
      };
    };
    specials = {
      {   make_box<symbol>("def"),      make_fn(&processor::analyze_def) },
      {   make_box<symbol>("fn*"),       make_fn(&processor::analyze_fn) },
      { make_box<symbol>("recur"),    make_fn(&processor::analyze_recur) },
      {    make_box<symbol>("do"),       make_fn(&processor::analyze_do) },
      {  make_box<symbol>("let*"),      make_fn(&processor::analyze_let) },
      { make_box<symbol>("loop*"),     make_fn(&processor::analyze_loop) },
      {    make_box<symbol>("if"),       make_fn(&processor::analyze_if) },
      { make_box<symbol>("quote"),    make_fn(&processor::analyze_quote) },
      {   make_box<symbol>("var"), make_fn(&processor::analyze_var_call) },
      { make_box<symbol>("throw"),    make_fn(&processor::analyze_throw) },
      {   make_box<symbol>("try"),      make_fn(&processor::analyze_try) },
      { make_box<symbol>("case*"),     make_fn(&processor::analyze_case) },
    };
  }

  processor::expression_result processor::analyze(read::parse::processor::iterator parse_current,
                                                  read::parse::processor::iterator const &parse_end)
  {
    if(parse_current == parse_end)
    {
      return error::internal_analysis_failure("Invalid iterator; parse_current == parse_end");
    }

    /* We wrap all of the expressions we get in an anonymous fn so that we can call it easily.
     * This also simplifies codegen, since we only ever codegen a single fn, even if that fn
     * represents a ns, a single REPL expression, or an actual source fn. */
    runtime::detail::native_transient_vector fn;
    fn.push_back(make_box<runtime::obj::symbol>("fn*"));
    fn.push_back(make_box<runtime::obj::persistent_vector>());
    for(; parse_current != parse_end; ++parse_current)
    {
      if(parse_current->is_err())
      {
        return err(parse_current->expect_err_move());
      }
      fn.push_back(parse_current->expect_ok().unwrap().ptr);
    }
    auto fn_list(make_box<runtime::obj::persistent_list>(std::in_place, fn.rbegin(), fn.rend()));
    return analyze(fn_list, expression_position::value);
  }

  processor::expression_result
  processor::analyze_def(runtime::obj::persistent_list_ptr const &l,
                         local_frame_ptr &current_frame,
                         expression_position const position,
                         option<expr::function_context_ptr> const &fn_ctx,
                         native_bool const)
  {
    auto const length(l->count());
    if(length < 2)
    {
      return error::analysis_invalid_def("Too few arguments provided to 'def'",
                                         meta_source(l->meta));
    }
    else if(length > 4)
    {
      return error::analysis_invalid_def("Too many argument provided to 'def'",
                                         meta_source(l->meta));
    }

    auto const sym_obj(l->data.rest().first().unwrap());
    if(sym_obj->type != runtime::object_type::symbol)
    {
      return error::analysis_invalid_def(
        fmt::format("The var name in a 'def' must be a symbol, but you provided a '{}'",
                    object_type_str(sym_obj->type)),
        meta_source(sym_obj));
    }

    auto const sym(runtime::expect_object<runtime::obj::symbol>(sym_obj));
    if(!sym->ns.empty())
    {
      return error::analysis_invalid_def("The provided var name for a 'def' must not be qualified",
                                         meta_source(sym_obj));
    }

    auto qualified_sym(current_frame->lift_var(sym));
    qualified_sym->meta = sym->meta;
    auto const var(rt_ctx.intern_var(qualified_sym));
    if(var.is_err())
    {
      return error::internal_analysis_failure(var.expect_err(), meta_source(sym));
    }

    option<native_box<expression>> value_expr;
    native_bool const has_value{ 3 <= length };
    native_bool const has_docstring{ 4 <= length };

    if(has_value)
    {
      auto const value_opt(has_docstring ? l->data.rest().rest().rest().first()
                                         : l->data.rest().rest().first());
      auto value_result(
        analyze(value_opt.unwrap(), current_frame, expression_position::value, fn_ctx, true));
      if(value_result.is_err())
      {
        return value_result;
      }
      value_expr = some(value_result.expect_ok());

      vars.insert_or_assign(var.expect_ok(), value_expr.unwrap());
    }

    if(has_docstring)
    {
      auto const docstring_obj(l->data.rest().rest().first().unwrap());
      if(docstring_obj->type != runtime::object_type::persistent_string)
      {
        return error::analysis_invalid_def("The doc string for a 'def' must be a string",
                                           meta_source(docstring_obj));
      }
      auto const meta_with_doc(
        runtime::assoc(qualified_sym->meta.unwrap_or(runtime::obj::nil::nil_const()),
                       rt_ctx.intern_keyword("doc").expect_ok(),
                       docstring_obj));
      qualified_sym = qualified_sym->with_meta(meta_with_doc);
    }

    /* Lift this so it can be used during codegen. */
    if(qualified_sym->meta.is_some())
    {
      current_frame->lift_constant(qualified_sym->meta.unwrap());
    }

    return make_box<expression>(expr::def<expression>{
      expression_base{ {}, position, current_frame, true },
      qualified_sym,
      value_expr
    });
  }

  processor::expression_result processor::analyze_case(obj::persistent_list_ptr const &o,
                                                       local_frame_ptr &f,
                                                       expression_position const position,
                                                       option<expr::function_context_ptr> const &fc,
                                                       native_bool const needs_box)
  {
    if(auto const length(o->count()); length != 6)
    {
      return error::analysis_invalid_case("Invalid case*: exactly 6 parameters are needed.",
                                          meta_source(o->meta));
    }

    auto it{ o->data.rest() };
    if(it.first().is_none())
    {
      return error::analysis_invalid_case("Value expression is missing.", meta_source(o->meta));
    }
    auto const value_expr_obj{ it.first().unwrap() };
    auto const value_expr{ analyze(value_expr_obj, f, expression_position::value, fc, needs_box) };
    if(value_expr.is_err())
    {
      return error::analysis_invalid_case(value_expr.expect_err()->message, meta_source(o->meta));
    }

    it = it.rest();
    if(it.first().is_none())
    {
      return error::analysis_invalid_case("Shift value is missing.", meta_source(o->meta));
    }
    auto const shift_obj{ it.first().unwrap() };
    if(shift_obj.data->type != object_type::integer)
    {
      return error::analysis_invalid_case("Shift value should be an integer.",
                                          meta_source(o->meta));
    }
    auto const shift{ runtime::expect_object<runtime::obj::integer>(shift_obj) };

    it = it.rest();
    if(it.first().is_none())
    {
      return error::analysis_invalid_case("Mask value is missing.", meta_source(o->meta));
    }
    auto const mask_obj{ it.first().unwrap() };
    if(mask_obj.data->type != object_type::integer)
    {
      return error::analysis_invalid_case("Mask value should be an integer.", meta_source(o->meta));
    }
    auto const mask{ runtime::expect_object<runtime::obj::integer>(mask_obj) };

    it = it.rest();
    if(it.first().is_none())
    {
      return error::analysis_invalid_case("Default expression is missing.", meta_source(o->meta));
    }
    auto const default_expr_obj{ it.first().unwrap() };
    auto const default_expr{ analyze(default_expr_obj, f, position, fc, needs_box) };

    it = it.rest();
    if(it.first().is_none())
    {
      return error::analysis_invalid_case("Keys and expressions are missing.",
                                          meta_source(o->meta));
    }
    auto const imap_obj{ it.first().unwrap() };

    struct keys_and_exprs
    {
      native_vector<native_integer> keys{};
      native_vector<expression_ptr> exprs{};
    };

    auto const keys_exprs{ visit_map_like(
      [&](auto const typed_imap_obj) -> string_result<keys_and_exprs> {
        keys_and_exprs ret{};
        for(auto seq{ typed_imap_obj->fresh_seq() }; seq != nullptr; seq = seq->next_in_place())
        {
          auto const e{ seq->first() };
          auto const k_obj{ e->data[0] };
          auto const v_obj{ e->data[1] };
          if(k_obj.data->type != object_type::integer)
          {
            return err("Map key for case* is expected to be an integer.");
          }
          auto const key{ runtime::expect_object<obj::integer>(k_obj) };
          auto const expr{ analyze(v_obj, f, position, fc, needs_box) };
          if(expr.is_err())
          {
            return err(expr.expect_err()->message);
          }
          ret.keys.push_back(key->data);
          ret.exprs.push_back(expr.expect_ok());
        }
        return ret;
      },
      [&]() -> string_result<keys_and_exprs> {
        return err("Case keys and expressions should be a map-like.");
      },
      imap_obj) };

    if(keys_exprs.is_err())
    {
      return error::analysis_invalid_case(keys_exprs.expect_err(), meta_source(o->meta));
    }

    auto case_expr{
      make_box<expression>(expr::case_<expression>{
                                                   expression_base{ {}, position, f, needs_box },
                                                   value_expr.expect_ok(),
                                                   shift->data,
                                                   mask->data,
                                                   default_expr.expect_ok(),
                                                   keys_exprs.expect_ok().keys,
                                                   keys_exprs.expect_ok().exprs,
                                                   }
      )
    };
    return case_expr;
  }

  processor::expression_result processor::analyze_symbol(runtime::obj::symbol_ptr const &sym,
                                                         local_frame_ptr &current_frame,
                                                         expression_position const position,
                                                         option<expr::function_context_ptr> const &,
                                                         native_bool needs_box)
  {
    assert(!sym->to_string().empty());

    /* TODO: Assert it doesn't start with __. */
    auto found_local(current_frame->find_local_or_capture(sym));
    if(found_local.is_some())
    {
      auto &unwrapped_local(found_local.unwrap());
      local_frame::register_captures(unwrapped_local);

      /* Since we're referring to a local, we're boxed if it is boxed. */
      needs_box |= unwrapped_local.binding.needs_box;

      /* Captured locals are always boxed, even if the originating local is not. */
      if(!unwrapped_local.crossed_fns.empty())
      {
        needs_box = true;

        /* Capturing counts as a boxed usage for the originating local. */
        unwrapped_local.binding.has_boxed_usage = true;

        /* The first time we reference a captured local from within a function, we get here.
         * We determine that we had to cross one or more function scopes to find the relevant
         * local, so it's a new capture. We register the capture above, but we need to search
         * again to get the binding within our current function, since the one we have now
         * is the originating binding.
         *
         * All future lookups for this capatured local, in this function, will skip this branch. */
        found_local = current_frame->find_local_or_capture(sym);
      }

      if(needs_box)
      {
        unwrapped_local.binding.has_boxed_usage = true;
      }
      else
      {
        unwrapped_local.binding.has_unboxed_usage = true;
      }

      return make_box<expression>(expr::local_reference{
        expression_base{ {}, position, current_frame, needs_box },
        sym,
        unwrapped_local.binding
      });
    }

    /* If it's not a local and it matches a fn's name, we're dealing with a
     * reference to a fn. We don't want to go to var lookup now. */
    auto const found_named_recursion(current_frame->find_named_recursion(sym));
    if(found_named_recursion.is_some())
    {
      return make_box<expression>(expr::recursion_reference<expression>{
        expression_base{ {}, position, current_frame, needs_box },
        found_named_recursion.unwrap()
      });
    }

    auto const qualified_sym(rt_ctx.qualify_symbol(sym));
    auto const var(rt_ctx.find_var(qualified_sym));
    if(var.is_none())
    {
      return error::analysis_unresolved_symbol(
        fmt::format("Unable to resolve symbol '{}'", sym->to_string()),
        meta_source(sym));
    }

    /* Macros aren't lifted, since they're not used during runtime. */
    auto const unwrapped_var(var.unwrap());
    auto const macro_kw(rt_ctx.intern_keyword("", "macro", true).expect_ok());
    if(unwrapped_var->meta.is_none()
       || get(unwrapped_var->meta.unwrap(), macro_kw) == runtime::obj::nil::nil_const())
    {
      current_frame->lift_var(qualified_sym);
    }
    return make_box<expression>(expr::var_deref<expression>{
      expression_base{ {}, position, current_frame },
      qualified_sym,
      unwrapped_var
    });
  }

  result<expr::function_arity<expression>, error_ptr>
  processor::analyze_fn_arity(runtime::obj::persistent_list_ptr const &list,
                              native_persistent_string const &name,
                              local_frame_ptr &current_frame)
  {
    auto const params_obj(list->data.first().unwrap());
    if(params_obj->type != runtime::object_type::persistent_vector)
    {
      return error::analysis_invalid_fn_parameters("A function parameter vector must be a vector",
                                                   meta_source(params_obj));
    }

    auto const params(runtime::expect_object<runtime::obj::persistent_vector>(params_obj));

    auto frame{
      make_box<local_frame>(local_frame::frame_type::fn, current_frame->rt_ctx, current_frame)
    };

    native_vector<runtime::obj::symbol_ptr> param_symbols;
    param_symbols.reserve(params->data.size());
    std::set<runtime::obj::symbol> unique_param_symbols;

    native_bool is_variadic{};
    for(auto it(params->data.begin()); it != params->data.end(); ++it)
    {
      auto const p(*it);
      if(p->type != runtime::object_type::symbol)
      {
        return error::analysis_invalid_fn_parameters(
          fmt::format("A function parameter must be a symbol, not a '{}'",
                      object_type_str(p->type)),
          meta_source(p));
      }

      auto const sym(runtime::expect_object<runtime::obj::symbol>(p));
      if(!sym->ns.empty())
      {
        return error::analysis_invalid_fn_parameters("A function parameter must be unqualified",
                                                     meta_source(p));
      }
      else if(sym->name == "&")
      {
        if(is_variadic)
        {
          return error::analysis_invalid_fn_parameters(
            "Multiple '&' paramters found, but only one may be present",
            meta_source(sym));
        }
        else if(it + 1 == params->data.end())
        {
          return error::analysis_invalid_fn_parameters(
            "A symbol must be present after '&' to name the variadic parameter",
            meta_source(*(it + 1)));
        }
        else if(it + 2 != params->data.end())
        {
          return error::analysis_invalid_fn_parameters(
            "There may be no additional parameters after the variadic parameter",
            meta_source(*(it + 1)));
        }

        is_variadic = true;
        continue;
      }

      auto const unique_res(unique_param_symbols.emplace(*sym));
      if(!unique_res.second)
      {
        /* TODO: Output a warning here. */
        for(auto &param : param_symbols)
        {
          if(param->equal(*sym))
          {
            /* C++ doesn't allow multiple params with the same name, so we generate a unique
             * name for shared params. */
            param = make_box<runtime::obj::symbol>(__rt_ctx->unique_string("shadowed"));
            break;
          }
        }
      }

      frame->locals.emplace(sym, local_binding{ sym, none, current_frame });
      param_symbols.emplace_back(sym);
    }

    /* We do this after building the symbols vector, since the & symbol isn't a param
     * and would cause an off-by-one error. */
    if(param_symbols.size() > runtime::max_params)
    {
      /* TODO: Suggestion: use & args to capture the rest */
      return error::analysis_invalid_fn_parameters(
        fmt::format("This function has too many parameters. The max is {}", runtime::max_params),
        meta_source(params_obj));
    }

    auto fn_ctx(make_box<expr::function_context>());
    fn_ctx->name = name;
    fn_ctx->is_variadic = is_variadic;
    fn_ctx->param_count = param_symbols.size();
    frame->fn_ctx = fn_ctx;
    expr::do_<expression> body_do{
      expression_base{ {}, expression_position::tail, frame }
    };
    size_t const form_count{ list->count() - 1 };
    size_t i{};
    for(auto const &item : list->data.rest())
    {
      auto const position((++i == form_count) ? expression_position::tail
                                              : expression_position::statement);
      auto form(analyze(item, frame, position, fn_ctx, position != expression_position::statement));
      if(form.is_err())
      {
        return form.expect_err_move();
      }
      body_do.values.emplace_back(form.expect_ok());
    }

    /* If it turns out this function uses recur, we need to ensure that its tail expression
     * is boxed. This is because unboxed values may use IIFE for initialization, which will
     * not work with the generated while/continue we use for recursion. */
    if(fn_ctx->is_tail_recursive)
    {
      body_do = step::force_boxed(std::move(body_do));
    }

    return ok(expr::function_arity<expression>{ std::move(param_symbols),
                                                std::move(body_do),
                                                std::move(frame),
                                                std::move(fn_ctx) });
  }

  processor::expression_result
  processor::analyze_fn(runtime::obj::persistent_list_ptr const &full_list,
                        local_frame_ptr &current_frame,
                        expression_position const position,
                        option<expr::function_context_ptr> const &,
                        native_bool const)
  {
    auto const length(full_list->count());
    if(length < 2)
    {
      return error::analysis_invalid_fn("This function is missing its parameter vector",
                                        meta_source(full_list));
    }
    auto list(full_list);

    native_persistent_string name, unique_name;
    auto first_elem(list->data.rest().first().unwrap());
    if(first_elem->type == runtime::object_type::symbol)
    {
      auto const s(runtime::expect_object<runtime::obj::symbol>(first_elem));
      name = s->name;
      unique_name = __rt_ctx->unique_string(name);
      if(length < 3)
      {
        return error::analysis_invalid_fn("This function is missing its parameter vector",
                                          meta_source(first_elem));
      }
      first_elem = list->data.rest().rest().first().unwrap();
      list = make_box(list->data.rest());
    }
    else
    {
      name = __rt_ctx->unique_string("fn");
      unique_name = name;
    }

    native_vector<expr::function_arity<expression>> arities;

    if(first_elem->type == runtime::object_type::persistent_vector)
    {
      auto result(analyze_fn_arity(make_box<runtime::obj::persistent_list>(list->data.rest()),
                                   name,
                                   current_frame));
      if(result.is_err())
      {
        return result.expect_err_move();
      }
      arities.emplace_back(result.expect_ok_move());
    }
    /* TODO: Sequence? */
    else
    {
      for(auto it(list->data.rest()); !it.empty(); it = it.rest())
      {
        auto arity_list_obj(it.first().unwrap());

        auto const err(runtime::visit_object(
          [&](auto const typed_arity_list) -> result<void, error_ptr> {
            using T = typename decltype(typed_arity_list)::value_type;

            if constexpr(runtime::behavior::sequenceable<T>)
            {
              auto arity_list(runtime::obj::persistent_list::create(typed_arity_list));

              auto result(analyze_fn_arity(arity_list.data, name, current_frame));
              if(result.is_err())
              {
                return result.expect_err_move();
              }
              arities.emplace_back(result.expect_ok_move());
              return ok();
            }
            else
            {
              return error::analysis_invalid_fn(
                "Invalid 'fn' syntax. Please provide either a list of arities or a "
                "parameter vector",
                meta_source(list));
            }
          },
          arity_list_obj));

        if(err.is_err())
        {
          return err.expect_err();
        }
      }
    }

    /* There can only be one variadic arity. Clojure requires this. */
    size_t found_variadic{};
    size_t variadic_arity{};
    for(auto const &arity : arities)
    {
      found_variadic += static_cast<int>(arity.fn_ctx->is_variadic);
      variadic_arity = arity.params.size();
    }
    if(found_variadic > 1)
    {
      return error::analysis_invalid_fn("A function may only have one variadic arity",
                                        meta_source(full_list));
    }

    /* The variadic arity, if present, must have at least as many fixed params as the
     * highest non-variadic arity. Clojure requires this. */
    if(found_variadic > 0)
    {
      for(auto const &arity : arities)
      {
        if(!arity.fn_ctx->is_variadic && arity.params.size() >= variadic_arity)
        {
          return error::analysis_invalid_fn(
            "The variadic arity of this function has fewer parameters than one of "
            "its fixed arities, which would lead to ambiguities when it's called",
            meta_source(full_list));
        }
      }
    }

    auto const meta(runtime::obj::persistent_hash_map::create_unique(
      std::make_pair(rt_ctx.intern_keyword("source").expect_ok(),
                     make_box(full_list->to_code_string())),
      std::make_pair(
        rt_ctx.intern_keyword("name").expect_ok(),
        make_box(runtime::obj::symbol{ runtime::__rt_ctx->current_ns()->to_string(), name }
                   .to_string()))));

    auto ret(make_box<expression>(expr::function<expression>{
      expression_base{ {}, position, current_frame },
      name,
      unique_name,
      {},
      meta
    }));

    /* Assert that arities are unique. Lazy implementation, but N is small anyway. */
    for(auto base(arities.begin()); base != arities.end(); ++base)
    {
      base->fn_ctx->fn = ret;
      /* TODO: remove these, since we have a ptr to the fn now. */
      base->fn_ctx->name = name;
      base->fn_ctx->unique_name = unique_name;
      if(base + 1 == arities.end())
      {
        break;
      }

      for(auto other(base + 1); other != arities.end(); ++other)
      {
        if(base->params.size() == other->params.size()
           && base->fn_ctx->is_variadic == other->fn_ctx->is_variadic)
        {
          return error::analysis_invalid_fn(
            "There are multiple overloads with the same number of parameters. Each "
            "one must be unique",
            meta_source(full_list));
        }
      }
    }

    boost::get<expr::function<expression>>(ret->data).arities = std::move(arities);

    return ret;
  }

  processor::expression_result
  processor::analyze_recur(runtime::obj::persistent_list_ptr const &list,
                           local_frame_ptr &current_frame,
                           expression_position const position,
                           option<expr::function_context_ptr> const &fn_ctx,
                           native_bool const)
  {
    if(fn_ctx.is_none())
    {
      return error::analysis_invalid_recur_position(
        "Unable to use recur outside of a function or loop",
        meta_source(list));
    }
    else if(rt_ctx.no_recur_var->is_bound() && runtime::truthy(rt_ctx.no_recur_var->deref()))
    {
      return error::analysis_invalid_recur_from_try(
        "It's not permitted to use recur through a try/catch",
        meta_source(list));
    }
    else if(position != expression_position::tail)
    {
      return error::analysis_invalid_recur_position("'recur' must be used in tail position",
                                                    meta_source(list));
    }

    /* Minus one to remove recur symbol. */
    auto const arg_count(list->count() - 1);
    if(fn_ctx.unwrap()->param_count != arg_count)
    {
      return error::analysis_invalid_recur_args(
        fmt::format("Invalid number of args passed to recur; expected {}, found {}",
                    fn_ctx.unwrap()->param_count,
                    arg_count),
        meta_source(list));
    }


    native_vector<expression_ptr> arg_exprs;
    arg_exprs.reserve(arg_count);
    for(auto const &form : list->data.rest())
    {
      auto arg_expr(analyze(form, current_frame, expression_position::value, fn_ctx, true));
      if(arg_expr.is_err())
      {
        return arg_expr;
      }
      arg_exprs.emplace_back(arg_expr.expect_ok());
    }

    fn_ctx.unwrap()->is_tail_recursive = true;

    return make_box<expression>(expr::recur<expression>{
      expression_base{ {}, position, current_frame },
      make_box<runtime::obj::persistent_list>(list->data.rest()),
      arg_exprs
    });
  }

  processor::expression_result
  processor::analyze_do(runtime::obj::persistent_list_ptr const &list,
                        local_frame_ptr &current_frame,
                        expression_position const position,
                        option<expr::function_context_ptr> const &fn_ctx,
                        native_bool const needs_box)
  {
    expr::do_<expression> ret{
      expression_base{ {}, position, current_frame },
      {}
    };
    size_t const form_count{ list->count() - 1 };
    size_t i{};
    for(auto const &item : list->data.rest())
    {
      auto const is_last(++i == form_count);
      auto const form_type(is_last ? position : expression_position::statement);
      auto form(analyze(item,
                        current_frame,
                        form_type,
                        fn_ctx,
                        form_type == expression_position::statement ? false : needs_box));
      if(form.is_err())
      {
        return form.expect_err_move();
      }

      if(is_last)
      {
        ret.needs_box = form.expect_ok()->get_base()->needs_box;
      }

      ret.values.emplace_back(form.expect_ok());
    }

    return make_box<expression>(std::move(ret));
  }

  processor::expression_result
  processor::analyze_let(runtime::obj::persistent_list_ptr const &o,
                         local_frame_ptr &current_frame,
                         expression_position const position,
                         option<expr::function_context_ptr> const &fn_ctx,
                         native_bool const needs_box)
  {
    if(o->count() < 2)
    {
      return error::analysis_invalid_let("A bindings vector must be provided to 'let'",
                                         meta_source(o));
    }

    auto const bindings_obj(o->data.rest().first().unwrap());
    if(bindings_obj->type != runtime::object_type::persistent_vector)
    {
      return error::analysis_invalid_let("The bindings of a 'let' must be in a vector",
                                         meta_source(bindings_obj));
    }

    auto const bindings(runtime::expect_object<runtime::obj::persistent_vector>(bindings_obj));

    auto const binding_parts(bindings->data.size());
    if(binding_parts % 2 == 1)
    {
      return error::analysis_invalid_let("There must be an even number of bindings for a 'let'",
                                         meta_source(bindings_obj));
    }

    expr::let<expression> ret{
      position,
      needs_box,
      make_box<local_frame>(local_frame::frame_type::let, current_frame->rt_ctx, current_frame)
    };
    ret.body.position = position;
    for(size_t i{}; i < binding_parts; i += 2)
    {
      auto const &sym_obj(bindings->data[i]);
      auto const &val(bindings->data[i + 1]);

      if(sym_obj->type != runtime::object_type::symbol)
      {
        return error::analysis_invalid_let("The left hand side of a 'let' binding must be a symbol",
                                           meta_source(sym_obj));
      }
      auto const &sym(runtime::expect_object<runtime::obj::symbol>(sym_obj));
      if(!sym->ns.empty())
      {
        return error::analysis_invalid_let("'let' binding symbols must be unqualified",
                                           meta_source(sym_obj));
      }

      auto res(analyze(val, ret.frame, expression_position::value, fn_ctx, false));
      if(res.is_err())
      {
        return res.expect_err_move();
      }
      auto it(ret.pairs.emplace_back(sym, res.expect_ok_move()));
      ret.frame->locals.emplace(
        sym,
        local_binding{ sym, some(it.second), current_frame, it.second->get_base()->needs_box });
    }

    size_t const form_count{ o->count() - 2 };
    size_t i{};
    for(auto const &item : o->data.rest().rest())
    {
      auto const is_last(++i == form_count);
      auto const form_type(is_last ? position : expression_position::statement);
      auto res(analyze(item, ret.frame, form_type, fn_ctx, needs_box));
      if(res.is_err())
      {
        return res.expect_err_move();
      }

      /* Ultimately, whether or not this let is boxed is up to the last form. */
      if(is_last)
      {
        ret.needs_box = res.expect_ok()->get_base()->needs_box;
      }

      ret.body.values.emplace_back(res.expect_ok_move());
    }

    return make_box<expression>(std::move(ret));
  }

  processor::expression_result
  processor::analyze_loop(runtime::obj::persistent_list_ptr const &o,
                          local_frame_ptr &current_frame,
                          expression_position const position,
                          option<expr::function_context_ptr> const &fn_ctx,
                          native_bool const)
  {
    if(o->count() < 2)
    {
      return error::analysis_invalid_loop("A 'loop' form requires a binding vector",
                                          meta_source(o));
    }

    auto const bindings_obj(o->data.rest().first().unwrap());
    if(bindings_obj->type != runtime::object_type::persistent_vector)
    {
      return error::analysis_invalid_loop("The bindings for a 'loop' must be a vector",
                                          meta_source(bindings_obj));
    }

    auto const bindings(runtime::expect_object<runtime::obj::persistent_vector>(bindings_obj));

    auto const binding_parts(bindings->data.size());
    if(binding_parts % 2 == 1)
    {
      return error::analysis_invalid_loop("There must be an even number of bindings for a 'loop'",
                                          meta_source(bindings_obj));
    }

    runtime::detail::native_transient_vector binding_syms, binding_vals;
    for(size_t i{}; i < binding_parts; i += 2)
    {
      auto const &sym_obj(bindings->data[i]);
      auto const &val(bindings->data[i + 1]);

      if(sym_obj->type != runtime::object_type::symbol)
      {
        return error::analysis_invalid_loop(
          "The left hand side of a 'loop' binding must be a symbol",
          meta_source(sym_obj));
      }
      auto const &sym(runtime::expect_object<runtime::obj::symbol>(sym_obj));
      if(!sym->ns.empty())
      {
        return error::analysis_invalid_loop("'loop' binding symbols must be unqualified",
                                            meta_source(sym_obj));
      }

      binding_syms.push_back(sym_obj);
      binding_vals.push_back(val);
    }

    /* We take the lazy way out here. Clojure JVM handles loop* with two cases:
     *
     * 1. Statements, which expand the loop inline and use labels, gotos, and mutation
     * 2. Expressions, which wrap the loop in a fn which does the same
     *
     * We do something similar to the second, but we transform the loop into just function
     * recursion and call the function on the spot. It works for both cases, though it's
     * marginally less efficient.
     *
     * However, there's an additional snag. If we just transform the loop into a fn to
     * call immediately, we get something like this:
     *
     * ```
     * (loop* [a 1
     *         b 2]
     *   (println a b))
     * ```
     *
     * Becoming this:
     *
     * ```
     * ((fn* [a b]
     *   (println a b)) 1 2)
     * ```
     *
     * This works great, but loop* can actually be used as a let*. That means we can do something
     * like this:
     *
     * ```
     * (loop* [a 1
     *         b (* 2 a)]
     *   (println a b))
     * ```
     *
     * But we can't translate that like the one above, since we'd be referring to `a` before it
     * was bound. So we get around this by actually just lifting all of this into a let*:
     *
     * ```
     * (let* [a 1
     *        b (* 2 a)]
     *   ((fn* [a b]
     *     (println a b)) a b))
     * ```
     */
    runtime::detail::native_persistent_list const args{ binding_syms.rbegin(),
                                                        binding_syms.rend() };
    auto const params(make_box<runtime::obj::persistent_vector>(binding_syms.persistent()));
    auto const fn(make_box<runtime::obj::persistent_list>(
      o->data.rest().rest().conj(params).conj(make_box<runtime::obj::symbol>("fn*"))));
    auto const call(make_box<runtime::obj::persistent_list>(args.conj(fn)));
    auto const let(make_box<runtime::obj::persistent_list>(std::in_place,
                                                           make_box<runtime::obj::symbol>("let*"),
                                                           bindings_obj,
                                                           call));

    return analyze_let(let, current_frame, position, fn_ctx, true);
  }

  processor::expression_result
  processor::analyze_if(runtime::obj::persistent_list_ptr const &o,
                        local_frame_ptr &current_frame,
                        expression_position const position,
                        option<expr::function_context_ptr> const &fn_ctx,
                        native_bool needs_box)
  {
    /* We can't (yet) guarantee that each branch of an if returns the same unboxed type,
     * so we're unable to unbox them. */
    needs_box = true;

    auto const form_count(o->count());
    if(form_count < 3)
    {
      return error::analysis_invalid_if("'if' needs at least a condition and a 'then' form",
                                        meta_source(o));
    }
    else if(form_count > 4)
    {
      return error::analysis_invalid_if(
        "Extra form specified in 'if'",
        meta_source(o->data.rest().rest().rest().rest().first().unwrap()));
    }

    auto const condition(o->data.rest().first().unwrap());
    auto condition_expr(
      analyze(condition, current_frame, expression_position::value, fn_ctx, false));
    if(condition_expr.is_err())
    {
      return condition_expr.expect_err_move();
    }

    auto const then(o->data.rest().rest().first().unwrap());
    auto then_expr(analyze(then, current_frame, position, fn_ctx, needs_box));
    if(then_expr.is_err())
    {
      return then_expr.expect_err_move();
    }

    option<expression_ptr> else_expr_opt;
    if(form_count == 4)
    {
      auto const else_(o->data.rest().rest().rest().first().unwrap());
      auto else_expr(analyze(else_, current_frame, position, fn_ctx, needs_box));
      if(else_expr.is_err())
      {
        return else_expr.expect_err_move();
      }

      else_expr_opt = else_expr.expect_ok();
    }

    return make_box<expression>(expr::if_<expression>{
      expression_base{ {}, position, current_frame, needs_box },
      condition_expr.expect_ok(),
      then_expr.expect_ok(),
      else_expr_opt
    });
  }

  processor::expression_result
  processor::analyze_quote(runtime::obj::persistent_list_ptr const &o,
                           local_frame_ptr &current_frame,
                           expression_position const position,
                           option<expr::function_context_ptr> const &fn_ctx,
                           native_bool const needs_box)
  {
    if(o->count() != 2)
    {
      return error::analysis_invalid_quote("'quote' requires exactly one form to quote",
                                           meta_source(o));
    }

    return analyze_primitive_literal(o->data.rest().first().unwrap(),
                                     current_frame,
                                     position,
                                     fn_ctx,
                                     needs_box);
  }

  processor::expression_result
  processor::analyze_var_call(runtime::obj::persistent_list_ptr const &o,
                              local_frame_ptr &current_frame,
                              expression_position const position,
                              option<expr::function_context_ptr> const &,
                              native_bool const)
  {
    if(o->count() != 2)
    {
      return error::analysis_invalid_var_reference("'var' expects exactly one form to resolve",
                                                   meta_source(o));
    }

    auto const arg(o->data.rest().first().unwrap());
    if(arg->type != runtime::object_type::symbol)
    {
      return error::analysis_invalid_var_reference("The argument to 'var' must be a symbol",
                                                   meta_source(arg));
    }

    auto const arg_sym(runtime::expect_object<runtime::obj::symbol>(arg));

    auto const qualified_sym(current_frame->lift_var(arg_sym));
    auto const found_var(rt_ctx.find_var(qualified_sym));
    if(found_var.is_none())
    {
      return error::analysis_unresolved_var(
        fmt::format("Unable to resolve var '{}'", qualified_sym->to_string()),
        meta_source(o));
    }

    return make_box<expression>(expr::var_ref<expression>{
      expression_base{ {}, position, current_frame, true },
      qualified_sym,
      found_var.unwrap()
    });
  }

  processor::expression_result
  processor::analyze_var_val(runtime::var_ptr const &o,
                             local_frame_ptr &current_frame,
                             expression_position const position,
                             option<expr::function_context_ptr> const &,
                             native_bool const)
  {
    auto const qualified_sym(
      current_frame->lift_var(make_box<runtime::obj::symbol>(o->n->name->name, o->name->name)));
    return make_box<expression>(expr::var_ref<expression>{
      expression_base{ {}, position, current_frame, true },
      qualified_sym,
      o
    });
  }

  processor::expression_result
  processor::analyze_throw(runtime::obj::persistent_list_ptr const &o,
                           local_frame_ptr &current_frame,
                           expression_position const position,
                           option<expr::function_context_ptr> const &fn_ctx,
                           native_bool const)
  {
    if(o->count() != 2)
    {
      return error::analysis_invalid_throw("'throw' requires exactly one argument", meta_source(o));
    }

    auto const arg(o->data.rest().first().unwrap());
    auto arg_expr(analyze(arg, current_frame, expression_position::value, fn_ctx, true));
    if(arg_expr.is_err())
    {
      return arg_expr.expect_err_move();
    }

    return make_box<expression>(expr::throw_<expression>{
      expression_base{ {}, position, current_frame, true },
      arg_expr.unwrap_move()
    });
  }

  processor::expression_result
  processor::analyze_try(runtime::obj::persistent_list_ptr const &list,
                         local_frame_ptr &current_frame,
                         expression_position const position,
                         option<expr::function_context_ptr> const &fn_ctx,
                         native_bool const)
  {
    auto try_frame(
      make_box<local_frame>(local_frame::frame_type::try_, current_frame->rt_ctx, current_frame));
    /* We introduce a new frame so that we can register the sym as a local.
     * It holds the exception value which was caught. */
    auto catch_frame(
      make_box<local_frame>(local_frame::frame_type::catch_, current_frame->rt_ctx, current_frame));
    auto finally_frame(make_box<local_frame>(local_frame::frame_type::finally,
                                             current_frame->rt_ctx,
                                             current_frame));
    expr::try_<expression> ret{
      expression_base{ {}, position, try_frame }
    };

    /* Clojure JVM doesn't support recur across try/catch/finally, so we don't either. */
    rt_ctx
      .push_thread_bindings(runtime::obj::persistent_hash_map::create_unique(
        std::make_pair(rt_ctx.no_recur_var, runtime::obj::boolean::true_const())))
      .expect_ok();
    util::scope_exit const finally{ [&]() { rt_ctx.pop_thread_bindings().expect_ok(); } };

    enum class try_expression_type : uint8_t
    {
      other,
      catch_,
      finally_
    };

    static runtime::obj::symbol catch_{ "catch" }, finally_{ "finally" };
    native_bool has_catch{}, has_finally{};

    for(auto it(next_in_place(list->fresh_seq())); it != nullptr; it = next_in_place(it))
    {
      auto const item(it->first());
      auto const type(runtime::visit_seqable(
        [](auto const typed_item) {
          using T = typename decltype(typed_item)::value_type;

          if constexpr(std::same_as<T, obj::nil>)
          {
            return try_expression_type::other;
          }
          else
          {
            auto const first(typed_item->seq()->first());
            if(runtime::equal(first, &catch_))
            {
              return try_expression_type::catch_;
            }
            else if(runtime::equal(first, &finally_))
            {
              return try_expression_type::finally_;
            }
            else
            {
              return try_expression_type::other;
            }
          }
        },
        []() { return try_expression_type::other; },
        item));

      switch(type)
      {
        case try_expression_type::other:
          {
            if(has_catch || has_finally)
            {
              return error::analysis_invalid_try(
                "No extra forms may appear after 'catch' or 'finally",
                meta_source(item));
            }

            auto const is_last(it->next() == nullptr);
            auto const form_type(is_last ? position : expression_position::statement);
            auto form(analyze(item, try_frame, form_type, fn_ctx, is_last));
            if(form.is_err())
            {
              return form.expect_err_move();
            }

            ret.body.values.emplace_back(form.expect_ok());
          }
          break;
        case try_expression_type::catch_:
          {
            if(has_finally)
            {
              /* TODO: Note where the finally is. */
              return error::analysis_invalid_try(
                "No 'catch' forms are permitted after a 'finally' form has been been provided",
                meta_source(item));
            }
            if(has_catch)
            {
              /* TODO: Note where the other catch is. */
              return error::analysis_invalid_try("Only one 'catch' form may be supplied",
                                                 meta_source(item));
            }
            has_catch = true;

            /* Verify we have (catch <sym> ...) */
            auto const catch_list(runtime::list(item));
            auto const catch_body_size(catch_list->count());
            if(catch_body_size == 1)
            {
              return error::analysis_invalid_try(
                "Symbol required after 'catch', which is used as the binding to "
                "hold the exception value",
                meta_source(item));
            }

            auto const sym_obj(catch_list->data.rest().first().unwrap());
            if(sym_obj->type != runtime::object_type::symbol)
            {
              return error::analysis_invalid_try(
                "Symbol required after 'catch', which is used as the binding to "
                "hold the exception value",
                meta_source(sym_obj));
            }

            auto const sym(runtime::expect_object<runtime::obj::symbol>(sym_obj));
            if(!sym->get_namespace().empty())
            {
              return error::analysis_invalid_try("The symbol after 'catch' must be unqualified",
                                                 meta_source(sym_obj));
            }

            catch_frame->locals.emplace(sym, local_binding{ sym, none, catch_frame });

            /* Now we just turn the body into a do block and have the do analyzer handle the rest. */
            auto const do_list(
              catch_list->data.rest().rest().conj(make_box<runtime::obj::symbol>("do")));
            auto do_res(analyze(make_box(do_list), catch_frame, position, fn_ctx, true));
            if(do_res.is_err())
            {
              return do_res.expect_err_move();
            }

            ret.catch_body = expr::catch_<expression>{ sym,
                                                       std::move(boost::get<expr::do_<expression>>(
                                                         do_res.expect_ok()->data)) };
          }
          break;
        case try_expression_type::finally_:
          {
            if(has_finally)
            {
              /* TODO: Note the other finally */
              return error::analysis_invalid_try("Only one finally may be supplied",
                                                 meta_source(item));
            }
            has_finally = true;

            auto const finally_list(runtime::list(item));
            auto const do_list(
              finally_list->data.rest().conj(make_box<runtime::obj::symbol>("do")));
            auto do_res(analyze(make_box(do_list),
                                finally_frame,
                                expression_position::statement,
                                fn_ctx,
                                false));
            if(do_res.is_err())
            {
              return do_res.expect_err_move();
            }
            ret.finally_body
              = std::move(boost::get<expr::do_<expression>>(do_res.expect_ok()->data));
          }
          break;
      }
    }

    ret.body.frame = try_frame;
    ret.body.propagate_position(position);
    if(ret.catch_body.is_some())
    {
      ret.catch_body.unwrap().body.frame = catch_frame;
    }
    if(ret.finally_body.is_some())
    {
      ret.finally_body.unwrap().frame = finally_frame;
    }

    return make_box<expression>(std::move(ret));
  }

  processor::expression_result
  processor::analyze_primitive_literal(runtime::object_ptr const o,
                                       local_frame_ptr &current_frame,
                                       expression_position const position,
                                       option<expr::function_context_ptr> const &,
                                       native_bool const needs_box)
  {
    current_frame->lift_constant(o);
    return make_box<expression>(expr::primitive_literal<expression>{
      expression_base{ {}, position, current_frame, needs_box },
      o
    });
  }

  /* TODO: Test for this. */
  processor::expression_result
  processor::analyze_vector(runtime::obj::persistent_vector_ptr const &o,
                            local_frame_ptr &current_frame,
                            expression_position const position,
                            option<expr::function_context_ptr> const &fn_ctx,
                            native_bool const)
  {
    native_vector<expression_ptr> exprs;
    exprs.reserve(o->count());
    native_bool literal{ true };
    for(auto d = o->fresh_seq(); d != nullptr; d = next_in_place(d))
    {
      auto res(analyze(d->first(), current_frame, expression_position::value, fn_ctx, true));
      if(res.is_err())
      {
        return res.expect_err_move();
      }
      exprs.emplace_back(res.expect_ok_move());
      if(!boost::get<expr::primitive_literal<expression>>(&exprs.back()->data))
      {
        literal = false;
      }
    }

    if(literal)
    {
      /* Eval the literal to resolve exprs such as quotes. */
      auto const pre_eval_expr(make_box<expression>(expr::vector<expression>{
        expression_base{ {}, position, current_frame, true },
        std::move(exprs),
        o->meta
      }));
      auto const o(evaluate::eval(pre_eval_expr));

      /* TODO: Order lifted constants. Use sub constants during codegen. */
      current_frame->lift_constant(o);

      return make_box<expression>(expr::primitive_literal<expression>{
        expression_base{ {}, position, current_frame, true },
        o
      });
    }

    return make_box<expression>(expr::vector<expression>{
      expression_base{ {}, position, current_frame, true },
      std::move(exprs),
      o->meta,
    });
  }

  processor::expression_result
  processor::analyze_map(object_ptr const &o,
                         local_frame_ptr &current_frame,
                         expression_position const position,
                         option<expr::function_context_ptr> const &fn_ctx,
                         native_bool const)
  {
    /* TODO: Detect literal and act accordingly. */
    return visit_map_like(
      [&](auto const typed_o) -> processor::expression_result {
        using T = typename decltype(typed_o)::value_type;

        native_vector<std::pair<expression_ptr, expression_ptr>> exprs;
        exprs.reserve(typed_o->data.size());

        for(auto const &kv : typed_o->data)
        {
          /* The two maps (hash and sorted) have slightly different iterators, so we need to
           * pull out the entries differently. */
          object_ptr first{}, second{};
          if constexpr(std::same_as<T, obj::persistent_sorted_map>)
          {
            auto const &entry(kv.get());
            first = entry.first;
            second = entry.second;
          }
          else
          {
            first = kv.first;
            second = kv.second;
          }

          auto k_expr(analyze(first, current_frame, expression_position::value, fn_ctx, true));
          if(k_expr.is_err())
          {
            return k_expr.expect_err_move();
          }
          auto v_expr(analyze(second, current_frame, expression_position::value, fn_ctx, true));
          if(v_expr.is_err())
          {
            return v_expr.expect_err_move();
          }
          exprs.emplace_back(k_expr.expect_ok_move(), v_expr.expect_ok_move());
        }

        /* TODO: Uniqueness check. */
        return make_box<expression>(expr::map<expression>{
          expression_base{ {}, position, current_frame, true },
          std::move(exprs),
          typed_o->meta,
        });
      },
      o);
  }

  processor::expression_result
  processor::analyze_set(object_ptr const &o,
                         local_frame_ptr &current_frame,
                         expression_position const position,
                         option<expr::function_context_ptr> const &fn_ctx,
                         native_bool const)
  {
    return visit_set_like(
      [&](auto const typed_o) -> processor::expression_result {
        native_vector<expression_ptr> exprs;
        exprs.reserve(typed_o->count());
        native_bool literal{ true };
        for(auto d = typed_o->fresh_seq(); d != nullptr; d = next_in_place(d))
        {
          auto res(analyze(d->first(), current_frame, expression_position::value, fn_ctx, true));
          if(res.is_err())
          {
            return res.expect_err_move();
          }
          exprs.emplace_back(res.expect_ok_move());
          if(!boost::get<expr::primitive_literal<expression>>(&exprs.back()->data))
          {
            literal = false;
          }
        }

        if(literal)
        {
          /* Eval the literal to resolve exprs such as quotes. */
          auto const pre_eval_expr(make_box<expression>(expr::set<expression>{
            expression_base{ {}, position, current_frame, true },
            std::move(exprs),
            typed_o->meta
          }));
          auto const constant(evaluate::eval(pre_eval_expr));

          /* TODO: Order lifted constants. Use sub constants during codegen. */
          current_frame->lift_constant(constant);

          return make_box<expression>(expr::primitive_literal<expression>{
            expression_base{ {}, position, current_frame, true },
            constant
          });
        }

        return make_box<expression>(expr::set<expression>{
          expression_base{ {}, position, current_frame, true },
          std::move(exprs),
          typed_o->meta,
        });
      },
      o);
  }

  processor::expression_result
  processor::analyze_call(runtime::obj::persistent_list_ptr const &o,
                          local_frame_ptr &current_frame,
                          expression_position const position,
                          option<expr::function_context_ptr> const &fn_ctx,
                          native_bool const needs_box)
  {
    /* An empty list evaluates to a list, not a call. */
    auto const count(o->count());
    if(count == 0)
    {
      return analyze_primitive_literal(o, current_frame, position, fn_ctx, needs_box);
    }

    auto const arg_count(count - 1);

    auto const first(o->data.first().unwrap());
    expression_ptr source{};
    native_bool needs_ret_box{ true };
    native_bool needs_arg_box{ true };

    /* TODO: If this is a recursive call, note that and skip the var lookup. */
    if(first->type == runtime::object_type::symbol)
    {
      auto const sym(runtime::expect_object<runtime::obj::symbol>(first));
      auto const found_special(specials.find(sym));
      if(found_special != specials.end())
      {
        return found_special->second(o, current_frame, position, fn_ctx, needs_box);
      }

      auto sym_result(analyze_symbol(sym, current_frame, expression_position::value, fn_ctx, true));
      if(sym_result.is_err())
      {
        return sym_result;
      }

      /* If this is a macro, recur so we can start over. */
      auto const expanded(rt_ctx.macroexpand(o));
      if(expanded != o)
      {
        return analyze(expanded, current_frame, position, fn_ctx, needs_box);
      }

      source = sym_result.expect_ok();
      auto const var_deref(boost::get<expr::var_deref<expression>>(&source->data));

      /* If this expression doesn't need to be boxed, based on where it's called, we can dig
       * into the call details itself to see if the function supports unboxed returns. Most don't. */
      if(var_deref && var_deref->var->meta.is_some())
      {
        auto const arity_meta(
          runtime::get_in(var_deref->var->meta.unwrap(),
                          make_box<runtime::obj::persistent_vector>(
                            std::in_place,
                            rt_ctx.intern_keyword("", "arities", true).expect_ok(),
                            /* NOTE: We don't support unboxed meta on variadic arities. */
                            make_box(arg_count))));

        native_bool const supports_unboxed_input(runtime::truthy(
          get(arity_meta, rt_ctx.intern_keyword("", "supports-unboxed-input?", true).expect_ok())));
        native_bool const supports_unboxed_output(
          runtime::truthy
          /* TODO: Rename key. */
          (get(arity_meta, rt_ctx.intern_keyword("", "unboxed-output?", true).expect_ok())));

        if(supports_unboxed_input || supports_unboxed_output)
        {
          auto const fn_res(vars.find(var_deref->var));
          /* If we don't have a valid var_deref, we know the var exists, but we
           * don't have an AST node for it. This means the var came in through
           * a pre-compiled module. In that case, we can only rely on meta to
           * tell us what we need. */
          if(fn_res != vars.end())
          {
            auto const fn(boost::get<expr::function<expression>>(&fn_res->second->data));
            if(!fn)
            {
              return error::internal_analysis_failure("unsupported arity meta on non-function var",
                                                      meta_source(first));
            }
          }

          needs_arg_box = !supports_unboxed_input;
          needs_ret_box = needs_box | !supports_unboxed_output;
        }
      }
    }
    else
    {
      auto callable_expr(
        analyze(first, current_frame, expression_position::value, fn_ctx, needs_box));
      if(callable_expr.is_err())
      {
        return callable_expr;
      }
      source = callable_expr.expect_ok_move();
    }

    native_vector<expression_ptr> arg_exprs;
    arg_exprs.reserve(std::min(arg_count, runtime::max_params + 1));

    auto it(o->data.rest());
    for(size_t i{}; i < runtime::max_params && i < arg_count; ++i, it = it.rest())
    {
      auto arg_expr(analyze(it.first().unwrap(),
                            current_frame,
                            expression_position::value,
                            fn_ctx,
                            needs_arg_box));
      if(arg_expr.is_err())
      {
        return arg_expr;
      }
      arg_exprs.emplace_back(arg_expr.expect_ok());
    }

    /* If we have more args than a fn allows, we need to pack all of the extras
     * into a single list and tack that on at the end. So, if max_params is 10, and
     * we pass 15 args, we'll pass 10 normally and then we'll have a special 11th
     * arg which is a list containing the 5 remaining params. We rely on dynamic_call
     * to do the hard work of packing that in the shape the function actually wants,
     * based on its highest fixed arity flag. */
    if(runtime::max_params < arg_count)
    {
      native_vector<expression_ptr> packed_arg_exprs;
      for(size_t i{ runtime::max_params }; i < arg_count; ++i, it = it.rest())
      {
        auto arg_expr(analyze(it.first().unwrap(),
                              current_frame,
                              expression_position::value,
                              fn_ctx,
                              needs_arg_box));
        if(arg_expr.is_err())
        {
          return arg_expr;
        }
        packed_arg_exprs.emplace_back(arg_expr.expect_ok());
      }
      expr::list<expression> list{
        expression_base{ {}, expression_position::value, current_frame, needs_arg_box },
        std::move(packed_arg_exprs),
        none,
      };
      arg_exprs.emplace_back(make_box<expression>(std::move(list)));
    }

    auto const recursion_ref(boost::get<expr::recursion_reference<expression>>(&source->data));
    if(recursion_ref)
    {
      return make_box<expression>(expr::named_recursion<expression>{
        expression_base{ {}, position, current_frame, needs_ret_box },
        *recursion_ref,
        make_box<runtime::obj::persistent_list>(o->data.rest()),
        arg_exprs
      });
    }
    else
    {
      return make_box<expression>(expr::call<expression>{
        expression_base{ {}, position, current_frame, needs_ret_box },
        source,
        make_box<runtime::obj::persistent_list>(o->data.rest()),
        arg_exprs
      });
    }
  }

  processor::expression_result
  processor::analyze(object_ptr const o, expression_position const position)
  {
    return analyze(o, root_frame, position, none, true);
  }

  processor::expression_result processor::analyze(object_ptr const o,
                                                  local_frame_ptr &current_frame,
                                                  expression_position const position,
                                                  option<expr::function_context_ptr> const &fn_ctx,
                                                  native_bool const needs_box)
  {
    if(o == nullptr)
    {
      return error::internal_analysis_failure("Unexpected nullptr in processor::analyze",
                                              read::source::unknown);
    }

    return runtime::visit_object(
      [&](auto const typed_o) -> processor::expression_result {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(std::same_as<T, runtime::obj::persistent_list>)
        {
          return analyze_call(typed_o, current_frame, position, fn_ctx, needs_box);
        }
        else if constexpr(std::same_as<T, runtime::obj::persistent_vector>)
        {
          return analyze_vector(typed_o, current_frame, position, fn_ctx, needs_box);
        }
        else if constexpr(runtime::behavior::map_like<T>)
        {
          return analyze_map(typed_o, current_frame, position, fn_ctx, needs_box);
        }
        else if constexpr(runtime::behavior::set_like<T>)
        {
          return analyze_set(typed_o, current_frame, position, fn_ctx, needs_box);
        }
        else if constexpr(runtime::behavior::number_like<T>
                          || std::same_as<T, runtime::obj::boolean>
                          || std::same_as<T, runtime::obj::keyword>
                          || std::same_as<T, runtime::obj::nil>
                          || std::same_as<T, runtime::obj::persistent_string>
                          || std::same_as<T, runtime::obj::character>)
        {
          return analyze_primitive_literal(o, current_frame, position, fn_ctx, needs_box);
        }
        else if constexpr(std::same_as<T, runtime::obj::symbol>)
        {
          return analyze_symbol(typed_o, current_frame, position, fn_ctx, needs_box);
        }
        /* This is used when building code from macros; they may end up being other forms of
         * sequences and not just lists. */
        if constexpr(runtime::behavior::sequential<T>)
        {
          return analyze_call(runtime::obj::persistent_list::create(typed_o->seq()),
                              current_frame,
                              position,
                              fn_ctx,
                              needs_box);
        }
        else if constexpr(std::same_as<T, runtime::var>)
        {
          return analyze_var_val(typed_o, current_frame, position, fn_ctx, needs_box);
        }
        else
        {
          return error::internal_analysis_failure(
            fmt::format("Unimplemented analysis for object type '{}'",
                        object_type_str(typed_o->base.type)),
            meta_source(o));
        }
      },
      o);
  }

  native_bool processor::is_special(runtime::object_ptr const form)
  {
    if(form->type != runtime::object_type::symbol)
    {
      return false;
    }

    auto const sym(runtime::expect_object<runtime::obj::symbol>(form));
    if(sym->ns.empty() && (sym->name == "finally" || sym->name == "catch"))
    {
      return true;
    }

    auto const found_special(specials.find(sym));
    return found_special != specials.end();
  }
}
