#include <functional>

#include <jank/runtime/context.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/behavior/number_like.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/detail/to_runtime_data.hpp>
#include <jank/util/fmt.hpp>

namespace jank::analyze
{
  using namespace jank::runtime;

  object_ref lifted_var::to_runtime_data() const
  {
    return obj::persistent_array_map::create_unique(make_box("var_name"), var_name);
  }

  object_ref lifted_constant::to_runtime_data() const
  {
    return obj::persistent_array_map::create_unique(make_box("data"), data);
  }

  object_ref local_binding::to_runtime_data() const
  {
    return obj::persistent_array_map::create_unique(
      make_box("name"),
      name,
      make_box("value_expr"),
      (value_expr.is_none() ? make_box("none") : value_expr.unwrap()->to_runtime_data()),
      make_box("originating_frame"),
      jank::detail::to_runtime_data(originating_frame),
      make_box("needs_box"),
      make_box(needs_box),
      make_box("has_boxed_usage"),
      make_box(has_boxed_usage),
      make_box("has_unboxed_usage"),
      make_box(has_unboxed_usage));
  }

  local_frame::local_frame(frame_type const &type,
                           context &rt_ctx,
                           jtl::option<jtl::ptr<local_frame>> const &p)
    : type{ type }
    , parent{ p }
    , rt_ctx{ rt_ctx }
  {
  }

  local_frame &local_frame::operator=(local_frame const &rhs)
  {
    if(this == &rhs)
    {
      return *this;
    }

    /* TODO: Is this operator used? It's missing some members. */
    type = rhs.type;
    parent = rhs.parent;
    locals = rhs.locals;

    return *this;
  }

  local_frame &local_frame::operator=(local_frame &&rhs) noexcept
  {
    if(this == &rhs)
    {
      return *this;
    }

    type = rhs.type;
    parent = std::move(rhs.parent);
    locals = std::move(rhs.locals);

    return *this;
  }

  static jtl::option<local_frame::find_result>
  find_local_impl(local_frame_ptr const start, obj::symbol_ref sym, bool const allow_captures)
  {
    decltype(local_frame::find_result::crossed_fns) crossed_fns;

    for(local_frame_ptr it{ start }; it != nullptr;)
    {
      auto const local_result(it->locals.find(sym));
      if(local_result != it->locals.end())
      {
        return local_frame::find_result{ &local_result->second, std::move(crossed_fns) };
      }

      if(allow_captures)
      {
        auto const capture_result(it->captures.find(sym));
        if(capture_result != it->locals.end())
        {
          return local_frame::find_result{ &capture_result->second, std::move(crossed_fns) };
        }
      }

      if(it->parent.is_some())
      {
        if(it->type == local_frame::frame_type::fn)
        {
          crossed_fns.emplace_back(it);
        }
        it = it->parent.unwrap();
      }
      else
      {
        return none;
      }
    }

    throw std::runtime_error{ util::format("unable to find local: {}", sym->to_string()) };
  }

  jtl::option<local_frame::find_result>
  local_frame::find_local_or_capture(obj::symbol_ref const sym)
  {
    return find_local_impl(this, sym, true);
  }

  void local_frame::register_captures(find_result const &result)
  {
    for(auto const &crossed_fn : result.crossed_fns)
    {
      auto res(crossed_fn->captures.emplace(result.binding->name, *result.binding));
      /* We know it needs a box, since it's captured. */
      res.first->second.needs_box = true;
      res.first->second.has_boxed_usage = true;
      /* To start with, we assume it's only boxed. */
      res.first->second.has_unboxed_usage = false;
    }
  }

  jtl::option<local_frame::find_result>
  local_frame::find_originating_local(obj::symbol_ref const sym)
  {
    return find_local_impl(this, sym, false);
  }

  jtl::option<expr::function_context_ref>
  local_frame::find_named_recursion(obj::symbol_ref const sym)
  {
    auto const sym_str(sym->to_string());
    for(local_frame_ptr it{ this }; it != nullptr;)
    {
      if(it->type == frame_type::fn && it->fn_ctx->name == sym_str)
      {
        return it->fn_ctx;
      }

      if(it->parent.is_some())
      {
        it = it->parent.unwrap();
      }
      else
      {
        break;
      }
    }
    return none;
  }

  local_frame const &local_frame::find_closest_fn_frame(local_frame const &frame)
  {
    if(frame.type == local_frame::frame_type::fn)
    {
      /* NOLINTNEXTLINE(bugprone-return-const-ref-from-parameter): I expect this to not be a temporary. */
      return frame;
    }
    else if(frame.parent.is_some())
    {
      return find_closest_fn_frame(*frame.parent.unwrap());
    }

    /* Default to the root frame, if there is no fn frame. */
    /* NOLINTNEXTLINE(bugprone-return-const-ref-from-parameter): I expect this to not be a temporary. */
    return frame;
  }
  local_frame &local_frame::find_closest_fn_frame(local_frame &frame)
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast): Avoiding duplication.
  {
    return const_cast<local_frame &>(find_closest_fn_frame(std::as_const(frame)));
  }

  bool local_frame::within_same_fn(local_frame_ptr const l, local_frame_ptr const r)
  {
    return &find_closest_fn_frame(*l) == &find_closest_fn_frame(*r);
  }

  obj::symbol_ref local_frame::lift_var(obj::symbol_ref const &sym)
  {
    auto &closest_fn(find_closest_fn_frame(*this));
    auto const &found(closest_fn.lifted_vars.find(sym));
    if(found != closest_fn.lifted_vars.end())
    {
      return found->first;
    }

    obj::symbol_ref qualified_sym{};
    if(sym->ns.empty())
    {
      qualified_sym
        = make_box<obj::symbol>(expect_object<ns>(rt_ctx.current_ns_var->deref())->name->name,
                                sym->name);
    }
    else
    {
      qualified_sym = make_box<obj::symbol>(*sym);
    }

    /* We use unique native names, just so var names don't clash with the underlying C++ API. */
    lifted_var lv{ qualified_sym };
    closest_fn.lifted_vars.emplace(qualified_sym, std::move(lv));
    return qualified_sym;
  }

  jtl::option<std::reference_wrapper<lifted_var const>>
  local_frame::find_lifted_var(obj::symbol_ref const &sym) const
  {
    auto const &closest_fn(find_closest_fn_frame(*this));
    auto const &found(closest_fn.lifted_vars.find(sym));
    if(found != closest_fn.lifted_vars.end())
    {
      return some(std::ref(found->second));
    }
    return none;
  }

  void local_frame::lift_constant(object_ref const constant)
  {
    auto &closest_fn(find_closest_fn_frame(*this));
    auto const &found(closest_fn.lifted_constants.find(constant));
    if(found != closest_fn.lifted_constants.end())
    {
      return;
    }

    auto const name(__rt_ctx->unique_symbol("const"));
    auto const unboxed_name{ visit_number_like(
      [&](auto const) -> jtl::option<obj::symbol> {
        return obj::symbol{ name.ns, name.name + "__unboxed" };
      },
      []() -> jtl::option<obj::symbol> { return none; },
      constant) };

    lifted_constant l{ constant };
    closest_fn.lifted_constants.emplace(constant, std::move(l));
  }

  jtl::option<std::reference_wrapper<lifted_constant const>>
  local_frame::find_lifted_constant(object_ref const o) const
  {
    auto const &closest_fn(find_closest_fn_frame(*this));
    auto const &found(closest_fn.lifted_constants.find(o));
    if(found != closest_fn.lifted_constants.end())
    {
      return some(std::ref(found->second));
    }
    return none;
  }

  object_ref local_frame::to_runtime_data() const
  {
    return obj::persistent_array_map::create_unique(
      make_box("type"),
      make_box(frame_type_str(type)),
      make_box("parent"),
      jank::detail::to_runtime_data(parent),
      make_box("locals"),
      jank::detail::to_runtime_data(locals),
      make_box("captures"),
      jank::detail::to_runtime_data(captures),
      make_box("lifted_vars"),
      jank::detail::to_runtime_data(lifted_vars),
      make_box("lifted_constants"),
      jank::detail::to_runtime_data(lifted_constants));
  }
}
