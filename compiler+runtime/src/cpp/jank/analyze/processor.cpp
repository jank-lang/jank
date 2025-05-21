#include <ranges>
#include <set>

#include <Interpreter/Compatibility.h>

#include <cpptrace/from_current.hpp>

#include <jank/read/reparse.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/behavior/number_like.hpp>
#include <jank/runtime/behavior/sequential.hpp>
#include <jank/runtime/behavior/map_like.hpp>
#include <jank/runtime/behavior/set_like.hpp>
#include <jank/runtime/core/truthy.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/analyze/step/force_boxed.hpp>
#include <jank/evaluate.hpp>
#include <jtl/result.hpp>
#include <jank/util/scope_exit.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/try.hpp>
#include <jank/error/analyze.hpp>
#include <jank/analyze/expr/def.hpp>
#include <jank/analyze/expr/var_deref.hpp>
#include <jank/analyze/expr/var_ref.hpp>
#include <jank/analyze/expr/call.hpp>
#include <jank/analyze/expr/primitive_literal.hpp>
#include <jank/analyze/expr/list.hpp>
#include <jank/analyze/expr/vector.hpp>
#include <jank/analyze/expr/map.hpp>
#include <jank/analyze/expr/set.hpp>
#include <jank/analyze/expr/function.hpp>
#include <jank/analyze/expr/recur.hpp>
#include <jank/analyze/expr/recursion_reference.hpp>
#include <jank/analyze/expr/named_recursion.hpp>
#include <jank/analyze/expr/local_reference.hpp>
#include <jank/analyze/expr/let.hpp>
#include <jank/analyze/expr/letfn.hpp>
#include <jank/analyze/expr/do.hpp>
#include <jank/analyze/expr/if.hpp>
#include <jank/analyze/expr/throw.hpp>
#include <jank/analyze/expr/try.hpp>
#include <jank/analyze/expr/case.hpp>
#include <jank/analyze/expr/cpp_type.hpp>
#include <jank/analyze/expr/cpp_value.hpp>
#include <jank/analyze/expr/cpp_cast.hpp>
#include <jank/analyze/expr/cpp_call.hpp>
#include <jank/analyze/expr/cpp_constructor_call.hpp>
#include <jank/analyze/expr/cpp_member_call.hpp>
#include <jank/analyze/expr/cpp_member_access.hpp>
#include <jank/analyze/rtti.hpp>
#include <jank/analyze/cpp_util.hpp>

namespace jank::analyze
{
  using namespace jank::runtime;

  /* For every form we analyze, we keep track of its macro-expansion meta. This allows
   * us to keep a stack of macro expansions, which we can then use for error reporting.
   * However, we need to pop the stack when we're done with that form, so we return
   * a scope_exit which will do so. Since we don't always push something, we lift this
   * up into a nullable pointer. */
  static std::unique_ptr<util::scope_exit>
  push_macro_expansions(processor &proc, object_ref const o)
  {
    auto const meta(runtime::meta(o));
    auto const expansion(
      runtime::get(meta, __rt_ctx->intern_keyword("jank/macro-expansion").expect_ok()));

    if(expansion == jank_nil)
    {
      return nullptr;
    }

    proc.macro_expansions.push_back(expansion);

    return std::make_unique<util::scope_exit>([&]() { proc.macro_expansions.pop_back(); });
  }

  static object_ref latest_expansion(native_vector<runtime::object_ref> const &expansions)
  {
    if(expansions.empty())
    {
      return jank_nil;
    }

    /* Try to find an expansion which specifically has the `jank/macro-expansion` key
     * set in the meta. This is the root of our most recent expansion. */
    for(auto const latest : std::ranges::reverse_view(expansions))
    {
      auto const latest_meta{ meta(latest) };
      auto const expansion(
        runtime::get(latest_meta, __rt_ctx->intern_keyword("jank/macro-expansion").expect_ok()));

      if(expansion != jank_nil)
      {
        return expansion;
      }
    }

    /* If we don't find a good match, just return the last macro expansion. We only
     * wouldn't find a match in the case of synthetic macro data. In which case, the
     * latest macro we have is the closest to the synthetic macro. */
    return expansions.back();
  }

  static jtl::result<void, error_ref>
  apply_implicit_conversions(jtl::ptr<void> const fn,
                             /* Out param. */
                             native_vector<expression_ref> &arg_exprs,
                             std::vector<Cpp::TemplateArgInfo> const &arg_types,
                             local_frame_ptr const current_frame,
                             expression_position const position,
                             bool const needs_box,
                             native_vector<runtime::object_ref> const &macro_expansions);

  static processor::expression_result
  build_cpp_call(expr::cpp_value_ref const val,
                 native_vector<expression_ref> arg_exprs,
                 std::vector<Cpp::TemplateArgInfo> arg_types,
                 local_frame_ptr const current_frame,
                 expression_position const position,
                 bool const needs_box,
                 native_vector<runtime::object_ref> const &macro_expansions)
  {
    std::vector<void *> fns;

    auto const is_ctor{ val->val_kind == expr::cpp_value::value_kind::constructor };
    if(is_ctor && cpp_util::is_primitive(val->type))
    {
      if(arg_types.size() > 1)
      {
        return error::internal_analyze_failure(
          util::format("'{}' can only be constructed with one argument.",
                       Cpp::GetTypeAsString(val->type)),
          object_source(val->form),
          latest_expansion(macro_expansions));
      }
      if(arg_types.size() == 1 && !Cpp::IsConstructible(val->type, arg_types[0].m_Type)
         && !cpp_util::is_convertible(val->type))
      {
        return error::internal_analyze_failure(
          util::format("'{}' cannot be constructed from a '{}'.",
                       Cpp::GetTypeAsString(val->type),
                       Cpp::GetTypeAsString(arg_types[0].m_Type)),
          object_source(val->form),
          latest_expansion(macro_expansions));
      }

      return jtl::make_ref<expr::cpp_constructor_call>(position,
                                                       current_frame,
                                                       needs_box,
                                                       val->type,
                                                       nullptr,
                                                       false,
                                                       jtl::move(arg_exprs));
    }

    auto const is_member_call{ val->val_kind == expr::cpp_value::value_kind::member_call };
    if(is_member_call)
    {
      if(arg_exprs.empty())
      {
        return error::internal_analyze_failure("Member function calls need an invoking object.",
                                               object_source(val->form),
                                               latest_expansion(macro_expansions));
      }

      /* TODO: Switch std::vector to native_vector where possible. */
      auto const obj_type{ arg_types[0].m_Type };
      auto const obj_scope{ Cpp::GetScopeFromType(obj_type) };
      auto const name{ Cpp::GetScopeName(obj_scope) };
      if(!obj_scope)
      {
        return error::internal_analyze_failure(util::format("There is no '{}' member within '{}'.",
                                                            name,
                                                            Cpp::GetTypeAsString(obj_type)),
                                               object_source(val->form),
                                               latest_expansion(macro_expansions));
      }

      /* TODO: Check const of method and invoking object. */

      arg_types.erase(arg_types.begin());

      fns = Cpp::LookupMethods(name, obj_scope);
      if(fns.empty())
      {
        return error::internal_analyze_failure(util::format("There is no '{}' member within '{}'.",
                                                            name,
                                                            Cpp::GetTypeAsString(obj_type)),
                                               object_source(val->form),
                                               latest_expansion(macro_expansions));
      }
    }

    std::string scope_name{};
    if(is_ctor)
    {
      scope_name = Cpp::GetScopeName(val->scope);
      Cpp::LookupConstructors("", val->scope, fns);
    }
    else if(is_member_call)
    {
      scope_name = try_object<obj::symbol>(val->form)->name.substr(1);
    }
    else
    {
      scope_name = Cpp::GetScopeName(val->scope);
      fns = Cpp::GetFunctionsUsingName(Cpp::GetParentScope(val->scope), scope_name);
      if(fns.empty())
      {
        return error::internal_analyze_failure(
          util::format("There is no '{}' function.", scope_name),
          object_source(val->form),
          latest_expansion(macro_expansions));
      }
    }

    auto const match_res{ cpp_util::find_best_overload(fns, arg_types) };
    if(match_res.is_err())
    {
      return error::internal_analyze_failure(util::format("{}", match_res.expect_err()),
                                             object_source(val->form),
                                             latest_expansion(macro_expansions));
    }
    jtl::ptr<void> match{ match_res.expect_ok() };
    if(match)
    {
      auto const conversion_res{ apply_implicit_conversions(match,
                                                            arg_exprs,
                                                            arg_types,
                                                            current_frame,
                                                            position,
                                                            needs_box,
                                                            macro_expansions) };
      if(conversion_res.is_err())
      {
        return conversion_res.expect_err();
      }
      if(is_ctor)
      {
        return jtl::make_ref<expr::cpp_constructor_call>(position,
                                                         current_frame,
                                                         needs_box,
                                                         val->type,
                                                         match,
                                                         false,
                                                         jtl::move(arg_exprs));
      }
      else if(is_member_call)
      {
        return jtl::make_ref<expr::cpp_member_call>(position,
                                                    current_frame,
                                                    needs_box,
                                                    Cpp::GetFunctionReturnType(match),
                                                    match,
                                                    jtl::move(arg_exprs));
      }
      else
      {
        auto const return_type{ Cpp::GetFunctionReturnType(match) };
        return jtl::make_ref<expr::cpp_call>(position,
                                             current_frame,
                                             needs_box,
                                             return_type,
                                             match,
                                             jtl::move(arg_exprs));
      }
    }

    auto const new_types{ cpp_util::find_best_arg_types_with_conversions(fns, arg_types) };
    if(new_types.is_err())
    {
      return error::internal_analyze_failure(util::format("{}", new_types.expect_err()),
                                             object_source(val->form),
                                             latest_expansion(macro_expansions));
    }

    auto const conversion_match_res{ cpp_util::find_best_overload(fns, new_types.expect_ok()) };
    if(conversion_match_res.is_err())
    {
      return error::internal_analyze_failure(util::format("{}", conversion_match_res.expect_err()),
                                             object_source(val->form),
                                             latest_expansion(macro_expansions));
    }
    match = conversion_match_res.expect_ok();
    if(match)
    {
      auto const conversion_res{ apply_implicit_conversions(match,
                                                            arg_exprs,
                                                            arg_types,
                                                            current_frame,
                                                            position,
                                                            needs_box,
                                                            macro_expansions) };
      if(conversion_res.is_err())
      {
        return conversion_res.expect_err();
      }
      if(is_ctor)
      {
        return jtl::make_ref<expr::cpp_constructor_call>(position,
                                                         current_frame,
                                                         needs_box,
                                                         val->type,
                                                         match,
                                                         false,
                                                         jtl::move(arg_exprs));
      }
      else if(is_member_call)
      {
        return jtl::make_ref<expr::cpp_member_call>(position,
                                                    current_frame,
                                                    needs_box,
                                                    Cpp::GetFunctionReturnType(match),
                                                    match,
                                                    jtl::move(arg_exprs));
      }
      else
      {
        auto const return_type{ Cpp::GetFunctionReturnType(match) };
        return jtl::make_ref<expr::cpp_call>(position,
                                             current_frame,
                                             needs_box,
                                             return_type,
                                             match,
                                             jtl::move(arg_exprs));
      }
    }

    if(is_ctor && Cpp::IsAggregateConstructible(val->type, arg_types))
    {
      return jtl::make_ref<expr::cpp_constructor_call>(position,
                                                       current_frame,
                                                       needs_box,
                                                       val->type,
                                                       nullptr,
                                                       true,
                                                       jtl::move(arg_exprs));
    }

    /* TODO: Find a better way to render this. */
    util::string_builder sb;
    for(usize i{}; i != arg_types.size(); ++i)
    {
      util::format_to(sb,
                      " With argument {} having type '{}'.",
                      i,
                      Cpp::GetTypeAsString(arg_types[i].m_Type));
    }

    return error::internal_analyze_failure(util::format("No matching call to '{}' {}.{}",
                                                        scope_name,
                                                        is_ctor ? "constructor" : "function",
                                                        sb.release()),
                                           object_source(val->form),
                                           latest_expansion(macro_expansions));
  }

  static jtl::result<expression_ref, error_ref>
  apply_implicit_conversion(expression_ref const expr,
                            jtl::ptr<void> const expr_type,
                            jtl::ptr<void> const expected_type,
                            local_frame_ptr const current_frame,
                            expression_position const position,
                            bool const needs_box,
                            native_vector<runtime::object_ref> const &macro_expansions)
  {
    if(Cpp::GetUnderlyingType(expr_type) == Cpp::GetUnderlyingType(expected_type))
    {
      return ok(expr);
    }
    else if(cpp_util::is_any_object(expected_type) && cpp_util::is_convertible(expr_type))
    {
      return jtl::make_ref<expr::cpp_cast>(position,
                                           current_frame,
                                           needs_box,
                                           expected_type,
                                           expr_type,
                                           conversion_policy::into_object,
                                           expr);
    }
    else if(cpp_util::is_any_object(expr_type) && cpp_util::is_convertible(expected_type))
    {
      return jtl::make_ref<expr::cpp_cast>(position,
                                           current_frame,
                                           needs_box,
                                           expected_type,
                                           expected_type,
                                           conversion_policy::from_object,
                                           expr);
    }
    else if(Cpp::IsConstructible(expected_type, expr_type))
    {
      auto const bare_param_type{ Cpp::GetNonReferenceType(Cpp::GetTypeWithoutCv(expected_type)) };
      auto const cpp_value{ jtl::make_ref<expr::cpp_value>(
        expression_position::value,
        current_frame,
        false,
        /* TODO: Can we do better here? */
        make_box<obj::symbol>(Cpp::GetTypeAsString(bare_param_type)),
        bare_param_type,
        Cpp::GetScopeFromType(bare_param_type),
        expr::cpp_value::value_kind::constructor) };
      auto const new_expr{ build_cpp_call(cpp_value,
                                          { expr },
                                          { { expr_type } },
                                          current_frame,
                                          position,
                                          needs_box,
                                          macro_expansions) };
      if(new_expr.is_err())
      {
        return new_expr.expect_err();
      }
      return new_expr.expect_ok();
    }
    else
    {
      return error::internal_analyze_failure(
        util::format("Unknown implicit conversion from {} to {}.",
                     Cpp::GetTypeAsString(expr_type),
                     Cpp::GetTypeAsString(expected_type)),
        latest_expansion(macro_expansions));
    }
  }

  [[maybe_unused]]
  static jtl::result<expression_ref, error_ref>
  apply_implicit_conversion(expression_ref const expr,
                            jtl::ptr<void> const expected_type,
                            local_frame_ptr const current_frame,
                            expression_position const position,
                            bool const needs_box,
                            native_vector<runtime::object_ref> const &macro_expansions)
  {
    auto const expr_type{ cpp_util::expression_type(expr) };
    return apply_implicit_conversion(expr,
                                     expr_type,
                                     expected_type,
                                     current_frame,
                                     position,
                                     needs_box,
                                     macro_expansions);
  }

  static jtl::result<void, error_ref>
  apply_implicit_conversions(jtl::ptr<void> const fn,
                             /* Out param. */
                             native_vector<expression_ref> &arg_exprs,
                             std::vector<Cpp::TemplateArgInfo> const &arg_types,
                             local_frame_ptr const current_frame,
                             expression_position const position,
                             bool const needs_box,
                             native_vector<runtime::object_ref> const &macro_expansions)
  {
    auto const fn_param_count{ Cpp::GetFunctionNumArgs(fn) };
    for(usize i{}; i < fn_param_count; ++i)
    {
      auto const param_type{ Cpp::GetFunctionArgType(fn, i) };
      auto const res{ apply_implicit_conversion(arg_exprs[i],
                                                arg_types[i].m_Type,
                                                param_type,
                                                current_frame,
                                                position,
                                                needs_box,
                                                macro_expansions) };
      if(res.is_err())
      {
        return res.expect_err();
      }
      arg_exprs[i] = res.expect_ok();
    }
    return ok();
  }

  processor::processor(runtime::context &rt_ctx)
    : rt_ctx{ rt_ctx }
    , root_frame{ jtl::make_ref<local_frame>(local_frame::frame_type::root, rt_ctx, none) }
  {
    using runtime::obj::symbol;
    auto const make_fn = [this](auto const fn) -> decltype(specials)::mapped_type {
      return [this, fn](auto const list,
                        auto const current_frame,
                        auto const position,
                        auto const &fn_ctx,
                        auto const needs_box) {
        return (this->*fn)(list, current_frame, position, fn_ctx, needs_box);
      };
    };
    specials = {
      {      make_box<symbol>("def"),      make_fn(&processor::analyze_def) },
      {      make_box<symbol>("fn*"),       make_fn(&processor::analyze_fn) },
      {    make_box<symbol>("recur"),    make_fn(&processor::analyze_recur) },
      {       make_box<symbol>("do"),       make_fn(&processor::analyze_do) },
      {     make_box<symbol>("let*"),      make_fn(&processor::analyze_let) },
      {   make_box<symbol>("letfn*"),    make_fn(&processor::analyze_letfn) },
      {    make_box<symbol>("loop*"),     make_fn(&processor::analyze_loop) },
      {       make_box<symbol>("if"),       make_fn(&processor::analyze_if) },
      {    make_box<symbol>("quote"),    make_fn(&processor::analyze_quote) },
      {      make_box<symbol>("var"), make_fn(&processor::analyze_var_call) },
      {    make_box<symbol>("throw"),    make_fn(&processor::analyze_throw) },
      {      make_box<symbol>("try"),      make_fn(&processor::analyze_try) },
      {    make_box<symbol>("case*"),     make_fn(&processor::analyze_case) },
      { make_box<symbol>("cpp/cast"), make_fn(&processor::analyze_cpp_cast) },
    };
  }

  processor::expression_result processor::analyze(read::parse::processor::iterator parse_current,
                                                  read::parse::processor::iterator const &parse_end)
  {
    if(parse_current == parse_end)
    {
      return error::internal_analyze_failure("Invalid iterator; parse_current == parse_end.",
                                             latest_expansion(macro_expansions));
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
  processor::analyze_def(runtime::obj::persistent_list_ref const l,
                         local_frame_ptr const current_frame,
                         expression_position const position,
                         jtl::option<expr::function_context_ref> const &fn_ctx,
                         bool const)
  {
    auto const pop_macro_expansions{ push_macro_expansions(*this, l) };

    auto const length(l->count());
    if(length < 2)
    {
      return error::analyze_invalid_def("Missing var name in 'def'.",
                                        meta_source(l->meta),
                                        latest_expansion(macro_expansions));
    }
    else if(length > 4)
    {
      return error::analyze_invalid_def("Too many arguments provided to 'def'.",
                                        meta_source(l->meta),
                                        latest_expansion(macro_expansions));
    }

    auto const sym_obj(l->data.rest().first().unwrap());
    if(sym_obj->type != runtime::object_type::symbol)
    {
      return error::analyze_invalid_def("The var name in a 'def' must be a symbol.",
                                        object_source(sym_obj),
                                        "A symbol is needed for the name here.",
                                        latest_expansion(macro_expansions))
        ->add_usage(read::parse::reparse_nth(l, 1));
    }

    auto const sym(runtime::expect_object<runtime::obj::symbol>(sym_obj));
    if(!sym->ns.empty())
    {
      return error::analyze_invalid_def("The provided var name for a 'def' must not be qualified.",
                                        meta_source(sym->meta),
                                        latest_expansion(macro_expansions))
        ->add_usage(read::parse::reparse_nth(l, 1));
    }

    auto qualified_sym(current_frame->lift_var(sym));
    qualified_sym->meta = sym->meta;
    auto const var(rt_ctx.intern_var(qualified_sym));
    if(var.is_err())
    {
      return error::internal_analyze_failure(var.expect_err(),
                                             meta_source(sym),
                                             latest_expansion(macro_expansions));
    }

    jtl::option<jtl::ref<expression>> value_expr;
    bool const has_value{ 3 <= length };
    bool const has_docstring{ 4 <= length };

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
        return error::analyze_invalid_def("The doc string for a 'def' must be a string.",
                                          object_source(docstring_obj),
                                          latest_expansion(macro_expansions))
          ->add_usage(read::parse::reparse_nth(l, 2));
      }
      auto const meta_with_doc(runtime::assoc(qualified_sym->meta.unwrap_or(runtime::jank_nil),
                                              rt_ctx.intern_keyword("doc").expect_ok(),
                                              docstring_obj));
      qualified_sym = qualified_sym->with_meta(meta_with_doc);
    }

    /* Lift this so it can be used during codegen. */
    /* TODO: I don't think lifting meta is actually needed anymore. Verify. */
    if(qualified_sym->meta.is_some())
    {
      current_frame->lift_constant(qualified_sym->meta.unwrap());
    }

    return jtl::make_ref<expr::def>(position, current_frame, true, qualified_sym, value_expr);
  }

  processor::expression_result
  processor::analyze_case(obj::persistent_list_ref const o,
                          local_frame_ptr const f,
                          expression_position const position,
                          jtl::option<expr::function_context_ref> const &fc,
                          bool const needs_box)
  {
    auto const pop_macro_expansions{ push_macro_expansions(*this, o) };

    if(auto const length(o->count()); length != 6)
    {
      return error::analyze_invalid_case("Invalid case*: exactly 6 parameters are needed.",
                                         meta_source(o->meta),
                                         latest_expansion(macro_expansions));
    }

    auto it{ o->data.rest() };
    if(it.first().is_none())
    {
      return error::analyze_invalid_case("Value expression is missing.",
                                         meta_source(o->meta),
                                         latest_expansion(macro_expansions));
    }
    auto const value_expr_obj{ it.first().unwrap() };
    auto const value_expr{ analyze(value_expr_obj, f, expression_position::value, fc, needs_box) };
    if(value_expr.is_err())
    {
      return error::analyze_invalid_case(value_expr.expect_err()->message,
                                         meta_source(o->meta),
                                         latest_expansion(macro_expansions));
    }

    it = it.rest();
    if(it.first().is_none())
    {
      return error::analyze_invalid_case("Shift value is missing.",
                                         meta_source(o->meta),
                                         latest_expansion(macro_expansions));
    }
    auto const shift_obj{ it.first().unwrap() };
    if(shift_obj.data->type != object_type::integer)
    {
      return error::analyze_invalid_case("Shift value should be an integer.",
                                         meta_source(o->meta),
                                         latest_expansion(macro_expansions));
    }
    auto const shift{ runtime::expect_object<runtime::obj::integer>(shift_obj) };

    it = it.rest();
    if(it.first().is_none())
    {
      return error::analyze_invalid_case("Mask value is missing.",
                                         meta_source(o->meta),
                                         latest_expansion(macro_expansions));
    }
    auto const mask_obj{ it.first().unwrap() };
    if(mask_obj.data->type != object_type::integer)
    {
      return error::analyze_invalid_case("Mask value should be an integer.",
                                         meta_source(o->meta),
                                         latest_expansion(macro_expansions));
    }
    auto const mask{ runtime::expect_object<runtime::obj::integer>(mask_obj) };

    it = it.rest();
    if(it.first().is_none())
    {
      return error::analyze_invalid_case("Default expression is missing.",
                                         meta_source(o->meta),
                                         latest_expansion(macro_expansions));
    }
    auto const default_expr_obj{ it.first().unwrap() };
    auto const default_expr{ analyze(default_expr_obj, f, position, fc, needs_box) };

    it = it.rest();
    if(it.first().is_none())
    {
      return error::analyze_invalid_case("Keys and expressions are missing.",
                                         meta_source(o->meta),
                                         latest_expansion(macro_expansions));
    }
    auto const imap_obj{ it.first().unwrap() };

    struct keys_and_exprs
    {
      native_vector<i64> keys{};
      native_vector<expression_ref> exprs{};
    };

    auto keys_exprs{ visit_map_like(
      [&](auto const typed_imap_obj) -> jtl::string_result<keys_and_exprs> {
        keys_and_exprs ret{};
        for(auto seq{ typed_imap_obj->fresh_seq() }; seq.is_some(); seq = seq->next_in_place())
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
      [&]() -> jtl::string_result<keys_and_exprs> {
        return err("Case keys and expressions should be a map-like.");
      },
      imap_obj) };

    if(keys_exprs.is_err())
    {
      return error::analyze_invalid_case(keys_exprs.expect_err(),
                                         meta_source(o->meta),
                                         latest_expansion(macro_expansions));
    }

    auto pairs{ keys_exprs.expect_ok_move() };

    return jtl::make_ref<expr::case_>(position,
                                      f,
                                      needs_box,
                                      value_expr.expect_ok(),
                                      shift->data,
                                      mask->data,
                                      default_expr.expect_ok(),
                                      std::move(pairs.keys),
                                      std::move(pairs.exprs));
  }

  processor::expression_result
  processor::analyze_symbol(runtime::obj::symbol_ref const sym,
                            local_frame_ptr const current_frame,
                            expression_position const position,
                            jtl::option<expr::function_context_ref> const &fc,
                            bool needs_box)
  {
    if(sym->ns == "cpp" && sym->name != "raw")
    {
      return analyze_cpp_symbol(sym, current_frame, position, fc, needs_box);
    }

    auto const pop_macro_expansions{ push_macro_expansions(*this, sym) };

    jank_debug_assert(!sym->to_string().empty());

    /* TODO: Assert it doesn't start with __. */
    auto found_local(current_frame->find_local_or_capture(sym));
    if(found_local.is_some())
    {
      auto &unwrapped_local(found_local.unwrap());
      local_frame::register_captures(unwrapped_local);

      /* Since we're referring to a local, we're boxed if it is boxed. */
      needs_box |= unwrapped_local.binding->needs_box;

      /* Captured locals are always boxed, even if the originating local is not. */
      if(!unwrapped_local.crossed_fns.empty())
      {
        needs_box = true;

        /* Capturing counts as a boxed usage for the originating local. */
        unwrapped_local.binding->has_boxed_usage = true;

        /* The first time we reference a captured local from within a function, we get here.
         * We determine that we had to cross one or more function scopes to find the relevant
         * local, so it's a new capture. We register the capture above, but we need to search
         * again to get the binding within our current function, since the one we have now
         * is the originating binding.
         *
         * All future lookups for this captured local, in this function, will skip this branch. */
        found_local = current_frame->find_local_or_capture(sym);
      }

      if(needs_box)
      {
        unwrapped_local.binding->has_boxed_usage = true;
      }
      else
      {
        unwrapped_local.binding->has_unboxed_usage = true;
      }

      return jtl::make_ref<expr::local_reference>(position,
                                                  current_frame,
                                                  needs_box,
                                                  sym,
                                                  unwrapped_local.binding);
    }

    /* If it's not a local and it matches a fn's name, we're dealing with a
     * reference to a fn. We don't want to go to var lookup now. */
    auto const found_named_recursion(current_frame->find_named_recursion(sym));
    if(found_named_recursion.is_some())
    {
      return jtl::make_ref<expr::recursion_reference>(position,
                                                      current_frame,
                                                      needs_box,
                                                      found_named_recursion.unwrap());
    }

    auto const qualified_sym(rt_ctx.qualify_symbol(sym));
    auto const var(rt_ctx.find_var(qualified_sym));
    if(var.is_nil())
    {
      return error::analyze_unresolved_symbol(
        util::format("Unable to resolve symbol '{}'.", sym->to_string()),
        meta_source(sym->meta),
        latest_expansion(macro_expansions));
    }

    /* Macros aren't lifted, since they're not used during runtime. */
    auto const macro_kw(rt_ctx.intern_keyword("", "macro", true).expect_ok());
    if(var->meta.is_none() || get(var->meta.unwrap(), macro_kw) == runtime::jank_nil)
    {
      current_frame->lift_var(qualified_sym);
    }
    return jtl::make_ref<expr::var_deref>(position, current_frame, true, qualified_sym, var);
  }

  jtl::result<expr::function_arity, error_ref>
  processor::analyze_fn_arity(runtime::obj::persistent_list_ref const list,
                              jtl::immutable_string const &name,
                              local_frame_ptr const current_frame)
  {
    auto const params_obj(list->data.first().unwrap());
    if(params_obj->type != runtime::object_type::persistent_vector)
    {
      return error::analyze_invalid_fn_parameters("A function parameter vector must be a vector.",
                                                  object_source(params_obj),
                                                  latest_expansion(macro_expansions))
        ->add_usage(read::parse::reparse_nth(list, 0));
    }

    auto const params(runtime::expect_object<runtime::obj::persistent_vector>(params_obj));

    auto frame{
      jtl::make_ref<local_frame>(local_frame::frame_type::fn, current_frame->rt_ctx, current_frame)
    };

    native_vector<runtime::obj::symbol_ref> param_symbols;
    param_symbols.reserve(params->data.size());
    std::set<runtime::obj::symbol> unique_param_symbols;

    bool is_variadic{};
    for(auto it(params->data.begin()); it != params->data.end(); ++it)
    {
      auto const p(*it);
      if(p->type != runtime::object_type::symbol)
      {
        auto const param_idx{ std::distance(params->data.begin(), it) };
        return error::analyze_invalid_fn_parameters("Each function parameter must be a symbol.",
                                                    object_source(p),
                                                    latest_expansion(macro_expansions))
          ->add_usage(read::parse::reparse_nth(params, param_idx));
      }

      auto const sym(runtime::expect_object<runtime::obj::symbol>(p));
      if(!sym->ns.empty())
      {
        return error::analyze_invalid_fn_parameters(
          "Each function parameter must be an unqualified symbol.",
          object_source(p),
          latest_expansion(macro_expansions));
      }
      else if(sym->name == "&")
      {
        if(it + 1 == params->data.end())
        {
          return error::analyze_invalid_fn_parameters(
            "A symbol must be present after '&' to name the variadic parameter.",
            object_source(*it),
            "A symbol should come after this '&'.",
            latest_expansion(macro_expansions));
        }
        else if(it + 2 != params->data.end())
        {
          /* TODO: Note the variadic param. */
          return error::analyze_invalid_fn_parameters(
            "There may be no additional parameters after the variadic parameter.",
            object_source(*(it + 2)),
            "Every parameter starting here is after the variadic parameter.",
            latest_expansion(macro_expansions));
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
      return error::analyze_invalid_fn_parameters(
        util::format("This function has too many parameters. The max is {}.", runtime::max_params),
        object_source(params_obj),
        latest_expansion(macro_expansions));
    }

    auto fn_ctx(jtl::make_ref<expr::function_context>());
    fn_ctx->name = name;
    fn_ctx->is_variadic = is_variadic;
    fn_ctx->param_count = param_symbols.size();
    frame->fn_ctx = fn_ctx;
    auto body_do{ jtl::make_ref<expr::do_>(expression_position::tail, frame, true) };
    usize const form_count{ list->count() - 1 };
    usize i{};
    for(auto const &item : list->data.rest())
    {
      auto const position((++i == form_count) ? expression_position::tail
                                              : expression_position::statement);
      auto form(analyze(item, frame, position, fn_ctx, position != expression_position::statement));
      if(form.is_err())
      {
        return form.expect_err_move();
      }
      body_do->values.emplace_back(form.expect_ok());
    }

    /* If it turns out this function uses recur, we need to ensure that its tail expression
     * is boxed. This is because unboxed values may use IIFE for initialization, which will
     * not work with the generated while/continue we use for recursion. */
    if(fn_ctx->is_tail_recursive)
    {
      step::force_boxed(body_do);
    }

    /* Ensure return type is an object. We'll handle automatic erasure from typed objects during
     * codegen. */
    if(!body_do->values.empty())
    {
      auto const last_expression{ body_do->values.back() };
      auto const last_expression_type{ cpp_util::expression_type(last_expression) };
      if(!cpp_util::is_any_object(last_expression_type)
         && !cpp_util::is_convertible(last_expression_type))
      {
        /* TODO: Error. */
        return error::internal_analyze_failure(
          util::format("This function is returning a native object of type '{}', which is not "
                       "convertible to a jank runtime object.",
                       Cpp::GetTypeAsString(last_expression_type)),
          object_source(list),
          latest_expansion(macro_expansions));
      }
    }

    return ok(expr::function_arity{ std::move(param_symbols),
                                    body_do,
                                    std::move(frame),
                                    std::move(fn_ctx) });
  }

  processor::expression_result
  processor::analyze_fn(runtime::obj::persistent_list_ref const full_list,
                        local_frame_ptr const current_frame,
                        expression_position const position,
                        jtl::option<expr::function_context_ref> const &,
                        bool const)
  {
    auto const pop_macro_expansions{ push_macro_expansions(*this, full_list) };

    auto const length(full_list->count());
    if(length < 2)
    {
      return error::analyze_invalid_fn("This function is missing its parameter vector.",
                                       meta_source(full_list->meta),
                                       latest_expansion(macro_expansions));
    }
    auto list(full_list);

    jtl::immutable_string name, unique_name;
    auto first_elem(list->data.rest().first().unwrap());
    if(first_elem->type == runtime::object_type::symbol)
    {
      auto const s(runtime::expect_object<runtime::obj::symbol>(first_elem));
      name = s->name;
      unique_name = __rt_ctx->unique_string(name);
      if(length < 3)
      {
        return error::analyze_invalid_fn("This function is missing its parameter vector.",
                                         meta_source(full_list->meta),
                                         latest_expansion(macro_expansions));
      }
      first_elem = list->data.rest().rest().first().unwrap();
      list = make_box(list->data.rest());
    }
    else
    {
      name = __rt_ctx->unique_string("fn");
      unique_name = name;
    }

    native_vector<expr::function_arity> arities;

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
    else
    {
      for(auto it(list->data.rest()); !it.empty(); it = it.rest())
      {
        auto arity_list_obj(it.first().unwrap());

        auto const err(runtime::visit_object(
          [&](auto const typed_arity_list) -> jtl::result<void, error_ref> {
            using T = typename decltype(typed_arity_list)::value_type;

            if constexpr(runtime::behavior::sequenceable<T>)
            {
              auto arity_list(runtime::obj::persistent_list::create(typed_arity_list));

              auto result(analyze_fn_arity(arity_list, name, current_frame));
              if(result.is_err())
              {
                return result.expect_err_move();
              }
              arities.emplace_back(result.expect_ok_move());
              return ok();
            }
            else
            {
              return error::analyze_invalid_fn(
                "Invalid 'fn' syntax. Please provide either a list of arities or a "
                "parameter vector.",
                meta_source(full_list->meta),
                latest_expansion(macro_expansions));
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
    usize found_variadic{};
    usize variadic_arity{};
    for(auto const &arity : arities)
    {
      found_variadic += static_cast<int>(arity.fn_ctx->is_variadic);
      variadic_arity = arity.params.size();
    }
    if(found_variadic > 1)
    {
      return error::analyze_invalid_fn("A function may only have one variadic arity.",
                                       meta_source(full_list->meta),
                                       latest_expansion(macro_expansions));
    }

    /* The variadic arity, if present, must have at least as many fixed params as the
     * highest non-variadic arity. Clojure requires this. */
    if(found_variadic > 0)
    {
      for(auto const &arity : arities)
      {
        if(!arity.fn_ctx->is_variadic && arity.params.size() >= variadic_arity)
        {
          /* TODO: Note the variadic arity and the highest fixed arity. */
          return error::analyze_invalid_fn(
            "The variadic arity of this function has fewer parameters than one of "
            "its fixed arities, which would lead to ambiguities when it's called.",
            meta_source(full_list->meta),
            latest_expansion(macro_expansions));
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

    auto ret(jtl::make_ref<expr::function>(position,
                                           current_frame,
                                           true,
                                           name,
                                           unique_name,
                                           native_vector<expr::function_arity>{},
                                           meta));

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
          return error::analyze_invalid_fn(
            "There are multiple overloads with the same number of parameters. Each "
            "one must be unique.",
            meta_source(full_list->meta),
            latest_expansion(macro_expansions));
        }
      }
    }

    static_ref_cast<expr::function>(ret)->arities = std::move(arities);

    return ret;
  }

  processor::expression_result
  processor::analyze_recur(runtime::obj::persistent_list_ref const list,
                           local_frame_ptr const current_frame,
                           expression_position const position,
                           jtl::option<expr::function_context_ref> const &fn_ctx,
                           bool const)
  {
    auto const pop_macro_expansions{ push_macro_expansions(*this, list) };

    if(fn_ctx.is_none())
    {
      return error::analyze_invalid_recur_position(
        "Unable to use 'recur' outside of a function or 'loop'.",
        meta_source(list->meta),
        latest_expansion(macro_expansions));
    }
    else if(rt_ctx.no_recur_var->is_bound() && runtime::truthy(rt_ctx.no_recur_var->deref()))
    {
      /* TODO: Note where the try is. */
      return error::analyze_invalid_recur_from_try(
        "It's not permitted to use 'recur' through a 'try'.",
        meta_source(list->meta),
        latest_expansion(macro_expansions));
    }
    else if(position != expression_position::tail)
    {
      return error::analyze_invalid_recur_position("'recur' must be used in tail position.",
                                                   meta_source(list->meta),
                                                   latest_expansion(macro_expansions));
    }

    /* Minus one to remove recur symbol. */
    auto const arg_count(list->count() - 1);
    if(fn_ctx.unwrap()->param_count != arg_count)
    {
      /* TODO: Note where the loop/fn args are. */
      return error::analyze_invalid_recur_args(
        util::format("{} arg(s) were passed to 'recur', but it needs exactly {} here.",
                     arg_count,
                     fn_ctx.unwrap()->param_count),
        meta_source(list->meta),
        latest_expansion(macro_expansions));
    }


    native_vector<expression_ref> arg_exprs;
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

    return jtl::make_ref<expr::recur>(position,
                                      current_frame,
                                      true,
                                      make_box<runtime::obj::persistent_list>(list->data.rest()),
                                      std::move(arg_exprs));
  }

  processor::expression_result
  processor::analyze_do(runtime::obj::persistent_list_ref const list,
                        local_frame_ptr const current_frame,
                        expression_position const position,
                        jtl::option<expr::function_context_ref> const &fn_ctx,
                        bool const needs_box)
  {
    auto const pop_macro_expansions{ push_macro_expansions(*this, list) };

    expr::do_ ret{ position, current_frame, true, {} };
    usize const form_count{ list->count() - 1 };
    usize i{};
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
        ret.needs_box = form.expect_ok()->needs_box;
      }

      ret.values.emplace_back(form.expect_ok());
    }

    return jtl::make_ref<expr::do_>(std::move(ret));
  }

  processor::expression_result
  processor::analyze_let(runtime::obj::persistent_list_ref const o,
                         local_frame_ptr const current_frame,
                         expression_position const position,
                         jtl::option<expr::function_context_ref> const &fn_ctx,
                         bool const needs_box)
  {
    auto const pop_macro_expansions{ push_macro_expansions(*this, o) };

    if(o->count() < 2)
    {
      return error::analyze_invalid_let("A bindings vector must be provided to 'let'.",
                                        meta_source(o->meta),
                                        latest_expansion(macro_expansions));
    }

    auto const bindings_obj(o->data.rest().first().unwrap());
    if(bindings_obj->type != runtime::object_type::persistent_vector)
    {
      return error::analyze_invalid_let("The bindings of a 'let' must be in a vector.",
                                        object_source(bindings_obj),
                                        latest_expansion(macro_expansions))
        ->add_usage(read::parse::reparse_nth(o, 1));
    }

    auto const bindings(runtime::expect_object<runtime::obj::persistent_vector>(bindings_obj));
    auto const binding_parts(bindings->data.size());
    if(binding_parts % 2 == 1)
    {
      /* TODO: Note the last value (maybe reparse). Check if it's a symbol? */
      return error::analyze_invalid_let("There must be an even number of bindings for a 'let'.",
                                        object_source(bindings_obj),
                                        latest_expansion(macro_expansions));
    }

    auto frame{
      jtl::make_ref<local_frame>(local_frame::frame_type::let, current_frame->rt_ctx, current_frame)
    };
    auto ret{ jtl::make_ref<expr::let>(
      position,
      frame,
      needs_box,
      jtl::make_ref<expr::do_>(position, frame, needs_box, native_vector<expression_ref>{})) };
    for(usize i{}; i < binding_parts; i += 2)
    {
      auto const &sym_obj(bindings->data[i]);
      auto const &val(bindings->data[i + 1]);

      if(sym_obj->type != runtime::object_type::symbol)
      {
        return error::analyze_invalid_let("The left hand side of a 'let' binding must be a symbol.",
                                          object_source(sym_obj),
                                          latest_expansion(macro_expansions))
          ->add_usage(read::parse::reparse_nth(bindings, i));
      }
      auto const &sym(runtime::expect_object<runtime::obj::symbol>(sym_obj));
      if(!sym->ns.empty())
      {
        return error::analyze_invalid_let("Binding symbols for 'let' must be unqualified.",
                                          object_source(sym_obj),
                                          latest_expansion(macro_expansions));
      }

      auto res(analyze(val, ret->frame, expression_position::value, fn_ctx, false));
      if(res.is_err())
      {
        return res.expect_err_move();
      }
      auto const it(ret->pairs.emplace_back(sym, res.expect_ok_move()));
      auto const expr_type{ cpp_util::non_void_expression_type(it.second) };
      ret->frame->locals.emplace(
        sym,
        local_binding{ sym, it.second, current_frame, it.second->needs_box, .type = expr_type });
    }

    usize const form_count{ o->count() - 2 };
    usize i{};
    for(auto const &item : o->data.rest().rest())
    {
      auto const is_last(++i == form_count);
      auto const form_type(is_last ? position : expression_position::statement);
      auto res(analyze(item, ret->frame, form_type, fn_ctx, needs_box));
      if(res.is_err())
      {
        return res.expect_err_move();
      }

      /* Ultimately, whether or not this let is boxed is up to the last form. */
      if(is_last)
      {
        ret->needs_box = res.expect_ok()->needs_box;
      }

      ret->body->values.emplace_back(res.expect_ok_move());
    }

    return ret;
  }

  processor::expression_result
  processor::analyze_letfn(runtime::obj::persistent_list_ref const &o,
                           local_frame_ptr const current_frame,
                           expression_position const position,
                           jtl::option<expr::function_context_ref> const &fn_ctx,
                           bool const needs_box)
  {
    if(o->count() < 2)
    {
      return error::analyze_invalid_letfn("A bindings vector must be provided to 'letfn*'.",
                                          meta_source(o),
                                          latest_expansion(macro_expansions));
    }

    auto const bindings_obj(o->data.rest().first().unwrap());
    if(bindings_obj->type != runtime::object_type::persistent_vector)
    {
      return error::analyze_invalid_letfn("The bindings of a 'letfn*' must be in a vector.",
                                          meta_source(bindings_obj),
                                          latest_expansion(macro_expansions));
    }

    auto const bindings(runtime::expect_object<runtime::obj::persistent_vector>(bindings_obj));
    auto const binding_parts(bindings->data.size());
    if(binding_parts % 2 == 1)
    {
      return error::analyze_invalid_letfn(
        "There must be an even number of bindings for a 'letfn*'.",
        meta_source(bindings_obj),
        latest_expansion(macro_expansions));
    }

    auto frame{
      make_box<local_frame>(local_frame::frame_type::letfn, current_frame->rt_ctx, current_frame)
    };
    auto ret{ make_box<expr::letfn>(
      position,
      frame,
      needs_box,
      make_box<expr::do_>(position, frame, needs_box, native_vector<expression_ref>{})) };

    /* All bindings in a letfn appear simultaneously and may be mutually recursive.
     * This makes creating a letfn locals frame a bit more involved than let, where locals
     * are introduced left-to-right. For example, each binding in (letfn [(a [] b) (b [] a)]) 
     * requires the other to be in scope in order to be analyzed.
     *
     * We tackle this in two steps. First, we create empty local bindings for all names.
     * Then, we analyze each value under the created scope and use the result to mutate the
     * respective local binding value. */
    for(usize i{}; i < binding_parts; i += 2)
    {
      auto const &sym_obj(bindings->data[i]);

      if(sym_obj->type != runtime::object_type::symbol)
      {
        return error::analyze_invalid_letfn(
          "The left hand side of a 'letfn*' binding must be a symbol.",
          meta_source(sym_obj),
          latest_expansion(macro_expansions));
      }
      auto const &sym(runtime::expect_object<runtime::obj::symbol>(sym_obj));
      if(!sym->ns.empty())
      {
        return error::analyze_invalid_letfn("'letfn*' binding symbols must be unqualified.",
                                            meta_source(sym_obj),
                                            latest_expansion(macro_expansions));
      }
      ret->frame->locals.emplace(sym, local_binding{ sym, none, current_frame });
    }

    for(usize i{}; i < binding_parts; i += 2)
    {
      auto const &sym(expect_object<runtime::obj::symbol>(bindings->data[i]));
      auto const &val(bindings->data[i + 1]);

      auto res(analyze(val, ret->frame, expression_position::value, fn_ctx, false));
      if(res.is_err())
      {
        return res.expect_err_move();
      }
      auto maybe_fexpr(res.expect_ok_move());
      if(maybe_fexpr->kind != expression_kind::function)
      {
        return error::analyze_invalid_letfn(
          "The right hand side of a 'letfn*' binding must be a function.",
          meta_source(val),
          latest_expansion(macro_expansions));
      }
      auto fexpr(runtime::static_box_cast<expr::function>(maybe_fexpr));

      /* Populate the local frame we prepared for sym in the previous loop with its binding. */
      auto it(ret->pairs.emplace_back(sym, fexpr));
      auto local(ret->frame->locals.find(sym)->second);
      local.value_expr = some(it.second);
      local.needs_box = it.second->needs_box;
    }

    usize const form_count{ o->count() - 2 };
    usize i{};
    for(auto const &item : o->data.rest().rest())
    {
      auto const is_last(++i == form_count);
      auto const form_type(is_last ? position : expression_position::statement);
      auto res(analyze(item, ret->frame, form_type, fn_ctx, needs_box));
      if(res.is_err())
      {
        return res.expect_err_move();
      }

      /* Ultimately, whether or not this letfn is boxed is up to the last form. */
      if(is_last)
      {
        ret->needs_box = res.expect_ok()->needs_box;
      }

      ret->body->values.emplace_back(res.expect_ok_move());
    }

    return ret;
  }

  processor::expression_result
  processor::analyze_loop(runtime::obj::persistent_list_ref const o,
                          local_frame_ptr const current_frame,
                          expression_position const position,
                          jtl::option<expr::function_context_ref> const &fn_ctx,
                          bool const)
  {
    auto const pop_macro_expansions{ push_macro_expansions(*this, o) };

    if(o->count() < 2)
    {
      return error::analyze_invalid_loop("A 'loop' form requires a binding vector.",
                                         meta_source(o->meta),
                                         latest_expansion(macro_expansions));
    }

    auto const bindings_obj(o->data.rest().first().unwrap());
    if(bindings_obj->type != runtime::object_type::persistent_vector)
    {
      return error::analyze_invalid_loop("The bindings for a 'loop' must be a vector.",
                                         object_source(bindings_obj),
                                         latest_expansion(macro_expansions))
        ->add_usage(read::parse::reparse_nth(o, 1));
    }

    auto const bindings(runtime::expect_object<runtime::obj::persistent_vector>(bindings_obj));

    auto const binding_parts(bindings->data.size());
    if(binding_parts % 2 == 1)
    {
      /* TODO: Note the last item. Check if it's a symbol? */
      return error::analyze_invalid_loop("There must be an even number of bindings for a 'loop'.",
                                         object_source(bindings_obj),
                                         latest_expansion(macro_expansions));
    }

    runtime::detail::native_transient_vector binding_syms, binding_vals;
    for(usize i{}; i < binding_parts; i += 2)
    {
      auto const &sym_obj(bindings->data[i]);
      auto const &val(bindings->data[i + 1]);

      if(sym_obj->type != runtime::object_type::symbol)
      {
        return error::analyze_invalid_loop(
                 "The left hand side of a 'loop' binding must be a symbol.",
                 object_source(sym_obj),
                 latest_expansion(macro_expansions))
          ->add_usage(read::parse::reparse_nth(bindings, i));
      }
      auto const &sym(runtime::expect_object<runtime::obj::symbol>(sym_obj));
      if(!sym->ns.empty())
      {
        return error::analyze_invalid_loop("Binding symbols for 'loop' must be unqualified.",
                                           object_source(sym_obj),
                                           latest_expansion(macro_expansions));
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
     *   (+ a b))
     * ```
     *
     * Becoming this:
     *
     * ```
     * ((fn* [a b]
     *   (+ a b)) 1 2)
     * ```
     *
     * This works great, but loop* can actually be used as a let*. That means we can do something
     * like this:
     *
     * ```
     * (loop* [a 1
     *         b (* 2 a)]
     *   (+ a b))
     * ```
     *
     * But we can't translate that like the one above, since we'd be referring to `a` before it
     * was bound. So we get around this by actually just lifting all of this into a let*:
     *
     * ```
     * (let* [a 1
     *        b (* 2 a)]
     *   ((fn* [a b]
     *     (+ a b)) a b))
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
  processor::analyze_if(runtime::obj::persistent_list_ref const o,
                        local_frame_ptr const current_frame,
                        expression_position const position,
                        jtl::option<expr::function_context_ref> const &fn_ctx,
                        bool needs_box)
  {
    auto const pop_macro_expansions{ push_macro_expansions(*this, o) };

    /* We can't (yet) guarantee that each branch of an if returns the same unboxed type,
     * so we're unable to unbox them. */
    needs_box = true;

    auto const form_count(o->count());
    if(form_count < 3)
    {
      return error::analyze_invalid_if("Each 'if' needs at least a condition and a 'then' form.",
                                       meta_source(o->meta),
                                       latest_expansion(macro_expansions));
    }
    else if(form_count > 4)
    {
      return error::analyze_invalid_if(
               "An extra form specified in this 'if'. There may only be a condition, a 'then' "
               "form, and "
               "then optionally an 'else' form.",
               object_source(o->data.rest().rest().rest().rest().first().unwrap()),
               "Everything starting here is excess. Perhaps you wanted to wrap some of these forms "
               "in a 'do'?",
               latest_expansion(macro_expansions))
        ->add_usage(read::parse::reparse_nth(o, 4));
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

    jtl::option<expression_ref> else_expr_opt;
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

    return jtl::make_ref<expr::if_>(position,
                                    current_frame,
                                    needs_box,
                                    condition_expr.expect_ok(),
                                    then_expr.expect_ok(),
                                    else_expr_opt);
  }

  processor::expression_result
  processor::analyze_quote(runtime::obj::persistent_list_ref const o,
                           local_frame_ptr const current_frame,
                           expression_position const position,
                           jtl::option<expr::function_context_ref> const &fn_ctx,
                           bool const needs_box)
  {
    auto const pop_macro_expansions{ push_macro_expansions(*this, o) };

    if(o->count() != 2)
    {
      return error::analyze_invalid_quote("'quote' requires exactly one form to quote.",
                                          meta_source(o->meta),
                                          latest_expansion(macro_expansions));
    }

    return analyze_primitive_literal(o->data.rest().first().unwrap(),
                                     current_frame,
                                     position,
                                     fn_ctx,
                                     needs_box);
  }

  processor::expression_result
  processor::analyze_var_call(runtime::obj::persistent_list_ref const o,
                              local_frame_ptr const current_frame,
                              expression_position const position,
                              jtl::option<expr::function_context_ref> const &,
                              bool const)
  {
    auto const pop_macro_expansions{ push_macro_expansions(*this, o) };

    if(o->count() != 2)
    {
      return error::analyze_invalid_var_reference("'var' expects exactly one form to resolve.",
                                                  meta_source(o->meta),
                                                  latest_expansion(macro_expansions));
    }

    auto const arg(o->data.rest().first().unwrap());
    if(arg->type != runtime::object_type::symbol)
    {
      return error::analyze_invalid_var_reference("The argument to 'var' must be a symbol.",
                                                  object_source(arg),
                                                  latest_expansion(macro_expansions))
        ->add_usage(read::parse::reparse_nth(o, 1));
    }

    auto const arg_sym(runtime::expect_object<runtime::obj::symbol>(arg));

    auto const qualified_sym(current_frame->lift_var(arg_sym));
    auto const found_var(rt_ctx.find_var(qualified_sym));
    if(found_var.is_nil())
    {
      return error::analyze_unresolved_var(
        util::format("Unable to resolve var '{}'.", qualified_sym->to_string()),
        meta_source(o->meta),
        latest_expansion(macro_expansions));
    }

    return jtl::make_ref<expr::var_ref>(position, current_frame, true, qualified_sym, found_var);
  }

  processor::expression_result
  processor::analyze_var_val(runtime::var_ref const o,
                             local_frame_ptr const current_frame,
                             expression_position const position,
                             jtl::option<expr::function_context_ref> const &,
                             bool const)
  {
    auto const pop_macro_expansions{ push_macro_expansions(*this, o) };

    auto const qualified_sym(
      current_frame->lift_var(make_box<runtime::obj::symbol>(o->n->name->name, o->name->name)));
    return jtl::make_ref<expr::var_ref>(position, current_frame, true, qualified_sym, o);
  }

  processor::expression_result
  processor::analyze_throw(runtime::obj::persistent_list_ref const o,
                           local_frame_ptr const current_frame,
                           expression_position const position,
                           jtl::option<expr::function_context_ref> const &fn_ctx,
                           bool const)
  {
    auto const pop_macro_expansions{ push_macro_expansions(*this, o) };

    if(o->count() != 2)
    {
      return error::analyze_invalid_throw("'throw' requires exactly one argument.",
                                          meta_source(o->meta),
                                          latest_expansion(macro_expansions));
    }

    auto const arg(o->data.rest().first().unwrap());
    auto arg_expr(analyze(arg, current_frame, expression_position::value, fn_ctx, true));
    if(arg_expr.is_err())
    {
      return arg_expr.expect_err_move();
    }

    return jtl::make_ref<expr::throw_>(position, current_frame, true, arg_expr.unwrap_move());
  }

  processor::expression_result
  processor::analyze_try(runtime::obj::persistent_list_ref const list,
                         local_frame_ptr const current_frame,
                         expression_position const position,
                         jtl::option<expr::function_context_ref> const &fn_ctx,
                         bool const)
  {
    auto const pop_macro_expansions{ push_macro_expansions(*this, list) };

    auto try_frame(jtl::make_ref<local_frame>(local_frame::frame_type::try_,
                                              current_frame->rt_ctx,
                                              current_frame));
    /* We introduce a new frame so that we can register the sym as a local.
     * It holds the exception value which was caught. */
    auto catch_frame(jtl::make_ref<local_frame>(local_frame::frame_type::catch_,
                                                current_frame->rt_ctx,
                                                current_frame));
    auto finally_frame(jtl::make_ref<local_frame>(local_frame::frame_type::finally,
                                                  current_frame->rt_ctx,
                                                  current_frame));
    auto ret{ jtl::make_ref<expr::try_>(position, try_frame, true, jtl::make_ref<expr::do_>()) };

    /* Clojure JVM doesn't support recur across try/catch/finally, so we don't either. */
    rt_ctx
      .push_thread_bindings(runtime::obj::persistent_hash_map::create_unique(
        std::make_pair(rt_ctx.no_recur_var, runtime::jank_true)))
      .expect_ok();
    util::scope_exit const finally{ [&]() { rt_ctx.pop_thread_bindings().expect_ok(); } };

    enum class try_expression_type : u8
    {
      other,
      catch_,
      finally_
    };

    static runtime::obj::symbol catch_{ "catch" }, finally_{ "finally" };
    bool has_catch{}, has_finally{};

    for(auto it(list->fresh_seq()->next_in_place()); it.is_some(); it = it->next_in_place())
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
            auto const first(runtime::first(typed_item->seq()));
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
              return error::analyze_invalid_try(
                "No extra forms may appear after 'catch' or 'finally'.",
                object_source(item),
                latest_expansion(macro_expansions));
            }

            auto const is_last(it->next().is_nil());
            auto const form_type(is_last ? position : expression_position::statement);
            auto form(analyze(item, try_frame, form_type, fn_ctx, is_last));
            if(form.is_err())
            {
              return form.expect_err_move();
            }

            ret->body->values.emplace_back(form.expect_ok());
          }
          break;
        case try_expression_type::catch_:
          {
            if(has_finally)
            {
              /* TODO: Note where the finally is. */
              return error::analyze_invalid_try(
                "No 'catch' forms are permitted after a 'finally' form has been been provided.",
                object_source(item),
                latest_expansion(macro_expansions));
            }
            if(has_catch)
            {
              /* TODO: Note where the other catch is. */
              return error::analyze_invalid_try("Only one 'catch' form may be supplied.",
                                                object_source(item),
                                                latest_expansion(macro_expansions));
            }
            has_catch = true;

            /* Verify we have (catch <sym> ...) */
            auto const catch_list(runtime::list(item));
            auto const catch_body_size(catch_list->count());
            if(catch_body_size == 1)
            {
              return error::analyze_invalid_try(
                "A symbol is required after 'catch', which is used as the binding to "
                "hold the exception value.",
                object_source(item),
                latest_expansion(macro_expansions));
            }

            auto const sym_obj(catch_list->data.rest().first().unwrap());
            if(sym_obj->type != runtime::object_type::symbol)
            {
              return error::analyze_invalid_try(
                       "A symbol required after 'catch', which is used as the binding to "
                       "hold the exception value.",
                       object_source(item),
                       error::note{
                         "A symbol is required before this form.",
                         object_source(sym_obj),
                       },
                       latest_expansion(macro_expansions))
                ->add_usage(read::parse::reparse_nth(item, 1));
            }

            auto const sym(runtime::expect_object<runtime::obj::symbol>(sym_obj));
            if(!sym->get_namespace().empty())
            {
              return error::analyze_invalid_try("The symbol after 'catch' must be unqualified.",
                                                object_source(sym_obj),
                                                latest_expansion(macro_expansions));
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

            ret->catch_body = expr::catch_{ sym, static_ref_cast<expr::do_>(do_res.expect_ok()) };
          }
          break;
        case try_expression_type::finally_:
          {
            if(has_finally)
            {
              /* TODO: Note the other finally */
              return error::analyze_invalid_try("Only one finally may be supplied.",
                                                object_source(item),
                                                latest_expansion(macro_expansions));
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
            ret->finally_body = static_ref_cast<expr::do_>(do_res.expect_ok());
          }
          break;
      }
    }

    ret->body->frame = try_frame;
    ret->body->propagate_position(position);
    if(ret->catch_body.is_some())
    {
      ret->catch_body.unwrap().body->frame = catch_frame;
    }
    if(ret->finally_body.is_some())
    {
      ret->finally_body.unwrap()->frame = finally_frame;
    }

    return ret;
  }

  processor::expression_result
  processor::analyze_primitive_literal(runtime::object_ref const o,
                                       local_frame_ptr const current_frame,
                                       expression_position const position,
                                       jtl::option<expr::function_context_ref> const &,
                                       bool const needs_box)
  {
    auto const pop_macro_expansions{ push_macro_expansions(*this, o) };

    current_frame->lift_constant(o);
    return jtl::make_ref<expr::primitive_literal>(position, current_frame, needs_box, o);
  }

  /* TODO: Test for this. */
  processor::expression_result
  processor::analyze_vector(runtime::obj::persistent_vector_ref const o,
                            local_frame_ptr const current_frame,
                            expression_position const position,
                            jtl::option<expr::function_context_ref> const &fn_ctx,
                            bool const)
  {
    auto const pop_macro_expansions{ push_macro_expansions(*this, o) };

    native_vector<expression_ref> exprs;
    exprs.reserve(o->count());
    bool literal{ true };
    for(auto d = o->fresh_seq(); d.is_some(); d = d->next_in_place())
    {
      auto res(analyze(d->first(), current_frame, expression_position::value, fn_ctx, true));
      if(res.is_err())
      {
        return res.expect_err_move();
      }
      exprs.emplace_back(res.expect_ok_move());
      if(exprs.back()->kind != expression_kind::primitive_literal)
      {
        literal = false;
      }
    }

    if(literal)
    {
      /* Eval the literal to resolve exprs such as quotes. */
      auto const pre_eval_expr(
        jtl::make_ref<expr::vector>(position, current_frame, true, std::move(exprs), o->meta));
      auto const o(evaluate::eval(pre_eval_expr));

      /* TODO: Order lifted constants. Use sub constants during codegen. */
      current_frame->lift_constant(o);

      return jtl::make_ref<expr::primitive_literal>(position, current_frame, true, o);
    }

    return jtl::make_ref<expr::vector>(position, current_frame, true, std::move(exprs), o->meta);
  }

  processor::expression_result
  processor::analyze_map(object_ref const o,
                         local_frame_ptr const current_frame,
                         expression_position const position,
                         jtl::option<expr::function_context_ref> const &fn_ctx,
                         bool const)
  {
    auto const pop_macro_expansions{ push_macro_expansions(*this, o) };

    /* TODO: Detect literal and act accordingly. */
    return visit_map_like(
      [&](auto const typed_o) -> processor::expression_result {
        using T = typename decltype(typed_o)::value_type;

        native_vector<std::pair<expression_ref, expression_ref>> exprs;
        exprs.reserve(typed_o->data.size());

        for(auto const &kv : typed_o->data)
        {
          /* The two maps (hash and sorted) have slightly different iterators, so we need to
           * pull out the entries differently. */
          object_ref first{}, second{};
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
        return jtl::make_ref<expr::map>(position,
                                        current_frame,
                                        true,
                                        std::move(exprs),
                                        typed_o->meta);
      },
      o);
  }

  processor::expression_result
  processor::analyze_set(object_ref const o,
                         local_frame_ptr const current_frame,
                         expression_position const position,
                         jtl::option<expr::function_context_ref> const &fn_ctx,
                         bool const)
  {
    auto const pop_macro_expansions{ push_macro_expansions(*this, o) };

    return visit_set_like(
      [&](auto const typed_o) -> processor::expression_result {
        native_vector<expression_ref> exprs;
        exprs.reserve(typed_o->count());
        bool literal{ true };
        for(auto d = typed_o->fresh_seq(); d.is_some(); d = d->next_in_place())
        {
          auto res(analyze(d->first(), current_frame, expression_position::value, fn_ctx, true));
          if(res.is_err())
          {
            return res.expect_err_move();
          }
          exprs.emplace_back(res.expect_ok_move());
          if(exprs.back()->kind != expression_kind::primitive_literal)
          {
            literal = false;
          }
        }

        if(literal)
        {
          /* Eval the literal to resolve exprs such as quotes. */
          auto const pre_eval_expr(jtl::make_ref<expr::set>(position,
                                                            current_frame,
                                                            true,
                                                            std::move(exprs),
                                                            typed_o->meta));
          auto const constant(evaluate::eval(pre_eval_expr));

          /* TODO: Order lifted constants. Use sub constants during codegen. */
          current_frame->lift_constant(constant);

          return jtl::make_ref<expr::primitive_literal>(position, current_frame, true, constant);
        }

        return jtl::make_ref<expr::set>(position,
                                        current_frame,
                                        true,
                                        std::move(exprs),
                                        typed_o->meta);
      },
      o);
  }

  processor::expression_result
  processor::analyze_call(runtime::obj::persistent_list_ref const o,
                          local_frame_ptr const current_frame,
                          expression_position const position,
                          jtl::option<expr::function_context_ref> const &fn_ctx,
                          bool const needs_box)
  {
    std::unique_ptr<util::scope_exit> pop_macro_expansions{};

    /* An empty list evaluates to a list, not a call. */
    auto const count(o->count());
    if(count == 0)
    {
      return analyze_primitive_literal(o, current_frame, position, fn_ctx, needs_box);
    }

    auto const arg_count(count - 1);

    auto const first(o->data.first().unwrap());
    jtl::ptr<expression> source;
    bool needs_ret_box{ true };
    bool needs_arg_box{ true };

    /* TODO: If this is a recursive call, note that and skip the var lookup. */
    if(first->type == runtime::object_type::symbol)
    {
      auto const sym(runtime::expect_object<runtime::obj::symbol>(first));
      auto const found_special(specials.find(sym));
      if(found_special != specials.end())
      {
        return found_special->second(o, current_frame, position, fn_ctx, needs_box);
      }

      pop_macro_expansions = push_macro_expansions(*this, o);

      auto sym_result(analyze_symbol(sym, current_frame, expression_position::value, fn_ctx, true));
      if(sym_result.is_err())
      {
        return sym_result;
      }

      if(sym_result.expect_ok()->kind == expression_kind::cpp_value)
      {
        return analyze_cpp_call(o,
                                llvm::cast<expr::cpp_value>(sym_result.expect_ok().data),
                                current_frame,
                                position,
                                fn_ctx,
                                needs_box);
      }

      object_ref expanded{ o };
      jtl::ptr<error::base> expansion_error{};
      JANK_TRY
      {
        expanded = rt_ctx.macroexpand(o);
      }
      JANK_CATCH_THEN(
        [&](auto const &e) {
          expansion_error
            = error::analyze_macro_expansion_exception(e,
                                                       cpptrace::from_current_exception(),
                                                       object_source(o),
                                                       latest_expansion(macro_expansions));
        },
        return expansion_error.as_ref())

      if(expanded != o)
      {
        return analyze(expanded, current_frame, position, fn_ctx, needs_box);
      }

      source = sym_result.expect_ok();
      auto const var_deref(llvm::dyn_cast<expr::var_deref>(source.data));

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

        bool const supports_unboxed_input(runtime::truthy(
          get(arity_meta, rt_ctx.intern_keyword("", "supports-unboxed-input?", true).expect_ok())));
        bool const supports_unboxed_output(
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
            if(fn_res->second.data->kind != expression_kind::function)
            {
              return error::internal_analyze_failure("Unsupported arity meta on non-function var.",
                                                     object_source(first),
                                                     latest_expansion(macro_expansions));
            }
          }

          needs_arg_box = !supports_unboxed_input;
          needs_ret_box = needs_box | !supports_unboxed_output;
        }
      }
    }
    else
    {
      pop_macro_expansions = push_macro_expansions(*this, o);

      auto callable_expr(
        analyze(first, current_frame, expression_position::value, fn_ctx, needs_box));
      if(callable_expr.is_err())
      {
        return callable_expr;
      }
      source = callable_expr.expect_ok_move();
    }

    native_vector<expression_ref> arg_exprs;
    arg_exprs.reserve(std::min(arg_count, runtime::max_params + 1));

    auto it(o->data.rest());
    for(usize i{}; i < runtime::max_params && i < arg_count; ++i, it = it.rest())
    {
      auto const arg_expr(analyze(it.first().unwrap(),
                                  current_frame,
                                  expression_position::value,
                                  fn_ctx,
                                  needs_arg_box));
      if(arg_expr.is_err())
      {
        return arg_expr;
      }
      auto const arg_conv_expr{ apply_implicit_conversion(arg_expr.expect_ok(),
                                                          cpp_util::untyped_object_ptr_type(),
                                                          current_frame,
                                                          position,
                                                          needs_box,
                                                          macro_expansions) };
      if(arg_conv_expr.is_err())
      {
        return arg_conv_expr;
      }
      arg_exprs.emplace_back(arg_conv_expr.expect_ok());
    }

    /* If we have more args than a fn allows, we need to pack all of the extras
     * into a single list and tack that on at the end. So, if max_params is 10, and
     * we pass 15 args, we'll pass 10 normally and then we'll have a special 11th
     * arg which is a list containing the 5 remaining params. We rely on dynamic_call
     * to do the hard work of packing that in the shape the function actually wants,
     * based on its highest fixed arity flag. */
    if(runtime::max_params < arg_count)
    {
      native_vector<expression_ref> packed_arg_exprs;
      for(usize i{ runtime::max_params }; i < arg_count; ++i, it = it.rest())
      {
        auto const arg_expr(analyze(it.first().unwrap(),
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
      arg_exprs.emplace_back(jtl::make_ref<expr::list>(expression_position::value,
                                                       current_frame,
                                                       needs_arg_box,
                                                       std::move(packed_arg_exprs),
                                                       none));
    }

    auto const recursion_ref(llvm::dyn_cast<expr::recursion_reference>(source.data));
    if(recursion_ref)
    {
      return jtl::make_ref<expr::named_recursion>(
        position,
        current_frame,
        needs_ret_box,
        std::move(*recursion_ref),
        make_box<runtime::obj::persistent_list>(o->data.rest()),
        std::move(arg_exprs));
    }
    else
    {
      return jtl::make_ref<expr::call>(position,
                                       current_frame,
                                       needs_ret_box,
                                       source.as_ref(),
                                       o,
                                       std::move(arg_exprs));
    }
  }

  processor::expression_result
  processor::analyze_cpp_symbol(obj::symbol_ref const sym,
                                local_frame_ptr const current_frame,
                                expression_position const position,
                                jtl::option<expr::function_context_ref> const &,
                                bool const needs_box)
  {
    auto const pop_macro_expansions{ push_macro_expansions(*this, sym) };
    jank_debug_assert(sym->ns == "cpp");

    /* TODO: Error if sym ends in . and we're not in a cpp call. */

    bool is_ctor{};
    auto name{ sym->name };
    if(name.ends_with('.'))
    {
      is_ctor = true;
      name = name.substr(0, name.size() - 1);
    }

    if(name.ends_with('.') || name.contains(".."))
    {
      /* TODO: Error. */
      return error::internal_analyze_failure(
        "Name must not contain consecutive '.' dots. Each '.' corresponds to a '::' in C++.",
        object_source(sym),
        latest_expansion(macro_expansions));
    }

    if(name == "nullptr")
    {
      static auto const scope_res{ cpp_util::resolve_scope("std.nullptr_t") };
      auto const scope{ Cpp::GetUnderlyingScope(scope_res.expect_ok()) };
      auto const type{ Cpp::GetTypeFromScope(scope) };
      return jtl::make_ref<expr::cpp_value>(position,
                                            current_frame,
                                            needs_box,
                                            sym,
                                            type,
                                            scope,
                                            expr::cpp_value::value_kind::null);
    }

    if(name.starts_with(".-"))
    {
      return jtl::make_ref<expr::cpp_value>(position,
                                            current_frame,
                                            needs_box,
                                            sym,
                                            nullptr,
                                            nullptr,
                                            expr::cpp_value::value_kind::member_access);
    }
    else if(name.starts_with('.'))
    {
      return jtl::make_ref<expr::cpp_value>(position,
                                            current_frame,
                                            needs_box,
                                            sym,
                                            nullptr,
                                            nullptr,
                                            expr::cpp_value::value_kind::member_call);
    }

    auto const global_type{ cpp_util::resolve_type(name) };

    /* Find a primitive type first. Then we know it's a cpp_type expression. */
    if(global_type && cpp_util::is_primitive(global_type))
    {
      if(is_ctor)
      {
        return jtl::make_ref<expr::cpp_value>(position,
                                              current_frame,
                                              needs_box,
                                              sym,
                                              global_type,
                                              nullptr,
                                              expr::cpp_value::value_kind::constructor);
      }

      return jtl::make_ref<expr::cpp_type>(position, current_frame, needs_box, sym, global_type);
    }

    auto const scope_res{ cpp_util::resolve_scope(name) };
    if(scope_res.is_err())
    {
      /* TODO: Error. */
      return error::internal_analyze_failure(util::format("{}", scope_res.expect_err()),
                                             object_source(sym),
                                             latest_expansion(macro_expansions));
    }

    /* The scope could represent either a type or a value, if it's valid. However, it's
     * possible that it represents a whole bunch of other things that we need to filter
     * out. */
    auto const scope{ Cpp::GetUnderlyingScope(scope_res.expect_ok()) };

    if(Cpp::IsNamespace(scope))
    {
      return error::internal_analyze_failure("Taking a C++ namespace by value is not permitted.",
                                             object_source(sym),
                                             latest_expansion(macro_expansions));
    }

    auto const type{ Cpp::GetTypeFromScope(scope) };
    if(Cpp::IsClass(scope) || Cpp::IsClassTemplateSpecialization(scope) || Cpp::IsEnumType(type))
    {
      if(is_ctor)
      {
        return jtl::make_ref<expr::cpp_value>(position,
                                              current_frame,
                                              needs_box,
                                              sym,
                                              type,
                                              scope,
                                              expr::cpp_value::value_kind::constructor);
      }

      return jtl::make_ref<expr::cpp_type>(position, current_frame, needs_box, sym, type);
    }

    /* We're not a type, but we have a . suffix, so this is an error. */
    if(is_ctor)
    {
      /* TODO: Error. */
      return error::internal_analyze_failure(
        util::format("The '.' suffix for constructors may only be used on types. In this case, "
                     "'{}' is a value of type '{}'.",
                     name,
                     Cpp::GetTypeAsString(type)),
        object_source(sym),
        latest_expansion(macro_expansions));
    }

    jtl::option<expr::cpp_value::value_kind> vk;
    if(Cpp::IsVariable(scope))
    {
      vk = expr::cpp_value::value_kind::variable;
    }
    else if(Cpp::IsEnumConstant(scope))
    {
      vk = expr::cpp_value::value_kind::enum_constant;
    }
    else if(Cpp::IsFunction(scope))
    {
      vk = expr::cpp_value::value_kind::function;
    }

    if(vk.is_some())
    {
      return jtl::make_ref<expr::cpp_value>(position,
                                            current_frame,
                                            needs_box,
                                            sym,
                                            type,
                                            scope,
                                            vk.unwrap());
    }

    return error::internal_analyze_failure("Unsupported C++ expression.",
                                           object_source(sym),
                                           latest_expansion(macro_expansions));
  }

  processor::expression_result
  processor::analyze_cpp_call(obj::persistent_list_ref const o,
                              expr::cpp_value_ref const val,
                              local_frame_ptr const current_frame,
                              expression_position const position,
                              jtl::option<expr::function_context_ref> const &fn_ctx,
                              bool const needs_box)
  {
    if(val->val_kind == expr::cpp_value::value_kind::member_access)
    {
      return analyze_cpp_member_access(o, val, current_frame, position, fn_ctx, needs_box);
    }

    auto it{ o->data.rest() };
    auto const arg_count{ it.size() };
    native_vector<expression_ref> arg_exprs;
    std::vector<Cpp::TemplateArgInfo> arg_types;
    for(usize i{}; i < arg_count; ++i, it = it.rest())
    {
      auto arg_expr{
        analyze(it.first().unwrap(), current_frame, expression_position::value, fn_ctx, needs_box)
      };
      if(arg_expr.is_err())
      {
        return arg_expr;
      }
      arg_exprs.emplace_back(arg_expr.expect_ok());
      arg_types.emplace_back(cpp_util::expression_type(arg_exprs.back()));
    }

    return build_cpp_call(val,
                          jtl::move(arg_exprs),
                          jtl::move(arg_types),
                          current_frame,
                          position,
                          needs_box,
                          macro_expansions);
  }

  processor::expression_result
  processor::analyze_cpp_cast(obj::persistent_list_ref const l,
                              local_frame_ptr const current_frame,
                              expression_position const position,
                              jtl::option<expr::function_context_ref> const &fn_ctx,
                              bool const needs_box)
  {
    auto const count(l->count());
    if(count != 3)
    {
      /* TODO: Error */
      return error::internal_analyze_failure(
        "A C++ cast must have only a C++ type and a value as arguments.",
        object_source(l),
        latest_expansion(macro_expansions));
    }

    auto const type_obj(l->data.rest().first().unwrap());
    /* TODO: Add a type expression_position and only allow types there? */
    auto type_expr_res(analyze(type_obj, current_frame, expression_position::value, fn_ctx, false));
    if(type_expr_res.is_err())
    {
      return type_expr_res.expect_err_move();
    }

    if(type_expr_res.expect_ok()->kind != expression_kind::cpp_type)
    {
      return error::internal_analyze_failure(
               "The first argument to 'cpp/cast' must be a C++ type. You can either use direct "
               "'cpp/std.string' form or the indirect '(cpp/type \"std::string\")' form.",
               object_source(type_obj),
               latest_expansion(macro_expansions))
        ->add_usage(read::parse::reparse_nth(l, 1));
    }

    /* TODO: Rename to type_expr. */
    auto const typed_expr{ llvm::cast<expr::cpp_type>(type_expr_res.expect_ok().data) };
    auto const value_obj(l->data.rest().rest().first().unwrap());
    auto value_expr_res(
      analyze(value_obj, current_frame, expression_position::value, fn_ctx, false));
    if(value_expr_res.is_err())
    {
      return value_expr_res.expect_err_move();
    }

    auto const value_expr{ value_expr_res.expect_ok() };
    auto const value_type{ cpp_util::expression_type(value_expr) };
    if(Cpp::IsConstructible(typed_expr->type, value_type))
    {
      auto const cpp_value{ jtl::make_ref<expr::cpp_value>(
        position,
        current_frame,
        needs_box,
        typed_expr->sym,
        typed_expr->type,
        Cpp::GetScopeFromType(typed_expr->type),
        expr::cpp_value::value_kind::constructor) };
      /* Since we're reusing analyze_cpp_call, we need to rebuild our list a bit. We
       * want to remove the cpp/cast and the type and then add back in a new head. Since
       * cpp_call takes in a cpp_value, it doesn't look at the head, but it needs to be there. */
      auto const call_l{ make_box(l->data.rest().rest().conj(jank_nil)) };
      return analyze_cpp_call(call_l, cpp_value, current_frame, position, fn_ctx, needs_box);
    }
    if(cpp_util::is_any_object(typed_expr->type) && cpp_util::is_convertible(value_type))
    {
      return jtl::make_ref<expr::cpp_cast>(position,
                                           current_frame,
                                           needs_box,
                                           typed_expr->type,
                                           value_type,
                                           conversion_policy::into_object,
                                           value_expr);
    }
    if(cpp_util::is_any_object(value_type) && cpp_util::is_convertible(typed_expr->type))
    {
      return jtl::make_ref<expr::cpp_cast>(position,
                                           current_frame,
                                           needs_box,
                                           typed_expr->type,
                                           typed_expr->type,
                                           conversion_policy::from_object,
                                           value_expr);
    }

    return error::internal_analyze_failure(
      util::format(
        "Invalid cast from '{}' to '{}'. This is impossible considering both constructors "
        "and any specializations of 'jank::runtime::convert'.",
        Cpp::GetTypeAsString(value_type),
        Cpp::GetTypeAsString(typed_expr->type)),
      object_source(l),
      latest_expansion(macro_expansions));
  }

  processor::expression_result
  processor::analyze_cpp_member_access(obj::persistent_list_ref const l,
                                       expr::cpp_value_ref const val,
                                       local_frame_ptr const current_frame,
                                       expression_position const position,
                                       jtl::option<expr::function_context_ref> const &fn_ctx,
                                       bool const needs_box)
  {
    auto const name{ try_object<obj::symbol>(val->form)->name.substr(2) };
    auto const count(l->count());
    if(count < 2)
    {
      /* TODO: Error */
      return error::internal_analyze_failure(
        util::format("Missing value from which to access '{}' member.", name),
        object_source(l),
        latest_expansion(macro_expansions));
    }
    else if(count > 3)
    {
      /* TODO: Error */
      return error::internal_analyze_failure(
        util::format("Excess arguments provided for '{}' member access. Only one is expected.",
                     name),
        object_source(l),
        latest_expansion(macro_expansions));
    }

    auto const obj(l->data.rest().first().unwrap());
    auto const obj_res(analyze(obj, current_frame, expression_position::value, fn_ctx, false));
    if(obj_res.is_err())
    {
      return obj_res;
    }

    auto const obj_expr{ obj_res.expect_ok() };
    auto const parent_type{ cpp_util::expression_type(obj_expr) };
    auto const parent_scope{ Cpp::GetScopeFromType(parent_type) };
    auto const member_scope{ Cpp::LookupDatamember(name, parent_scope) };
    if(!parent_scope || !member_scope)
    {
      return error::internal_analyze_failure(util::format("There is no '{}' member within '{}'.",
                                                          name,
                                                          Cpp::GetTypeAsString(parent_type)),
                                             object_source(l),
                                             latest_expansion(macro_expansions));
    }
    if(Cpp::IsPrivateVariable(member_scope))
    {
      return error::internal_analyze_failure(
        util::format(
          "The '{}' member within '{}' is private. It can only be accessed if it's public.",
          name,
          Cpp::GetTypeAsString(parent_type)),
        object_source(l),
        latest_expansion(macro_expansions));
    }
    if(Cpp::IsProtectedVariable(member_scope))
    {
      return error::internal_analyze_failure(
        util::format(
          "The '{}' member within '{}' is protected. It can only be accessed if it's public.",
          name,
          Cpp::GetTypeAsString(parent_type)),
        object_source(l),
        latest_expansion(macro_expansions));
    }
    auto const member_type{ Cpp::GetLValueReferenceType(Cpp::GetTypeFromScope(member_scope)) };

    return jtl::make_ref<expr::cpp_member_access>(position,
                                                  current_frame,
                                                  needs_box,
                                                  member_type,
                                                  member_scope,
                                                  name,
                                                  obj_expr);
  }

  processor::expression_result
  processor::analyze(object_ref const o, expression_position const position)
  {
    return analyze(o, root_frame, position, none, true);
  }

  processor::expression_result
  processor::analyze(object_ref const o,
                     local_frame_ptr const current_frame,
                     expression_position const position,
                     jtl::option<expr::function_context_ref> const &fn_ctx,
                     bool const needs_box)
  {
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
          return analyze_call(runtime::obj::persistent_list::create(meta(typed_o), typed_o->seq()),
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
          return error::internal_analyze_failure(
            util::format("Unimplemented analysis for object type '{}'.",
                         object_type_str(typed_o->base.type)),
            object_source(o),
            latest_expansion(macro_expansions));
        }
      },
      o);
  }

  bool processor::is_special(runtime::object_ref const form)
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
