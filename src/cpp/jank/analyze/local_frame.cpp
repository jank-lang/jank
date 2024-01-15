#include <functional>

#include <fmt/core.h>

#include <jank/runtime/util.hpp>
#include <jank/runtime/behavior/numberable.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze
{
  runtime::object_ptr lifted_var::to_runtime_data() const
  {
    return runtime::obj::persistent_array_map::create_unique(
      make_box("__type"),
      make_box("lifted_var"),
      make_box("native_name"),
      make_box<runtime::obj::symbol>(native_name),
      make_box("var_name"),
      var_name);
  }

  runtime::object_ptr lifted_constant::to_runtime_data() const
  {
    return runtime::obj::persistent_array_map::create_unique(
      make_box("__type"),
      make_box("lifted_constant"),
      make_box("native_name"),
      make_box<runtime::obj::symbol>(native_name),
      make_box("unboxed_native_name"),
      detail::to_runtime_data(unboxed_native_name),
      make_box("data"),
      data);
  }

  runtime::object_ptr local_binding::to_runtime_data() const
  {
    return runtime::obj::persistent_array_map::create_unique(
      make_box("__type"),
      make_box("local_binding"),
      make_box("name"),
      name,
      make_box("value_expr"),
      (value_expr.is_none() ? make_box("none") : value_expr.unwrap()->to_runtime_data()),
      make_box("originating_frame"),
      detail::to_runtime_data(originating_frame),
      make_box("needs_box"),
      make_box(needs_box),
      make_box("has_boxed_usage"),
      make_box(has_boxed_usage),
      make_box("has_unboxed_usage"),
      make_box(has_unboxed_usage));
  }

  local_frame::local_frame(frame_type const &type,
                           runtime::context &rt_ctx,
                           option<native_box<local_frame>> const &p)
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

    type = rhs.type;
    parent = rhs.parent;
    locals = rhs.locals;

    return *this;
  }

  local_frame &local_frame::operator=(local_frame &&rhs)
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

  option<local_frame::find_result> find_local_impl(local_frame_ptr const start,
                                                   runtime::obj::symbol_ptr sym,
                                                   native_bool const allow_captures)
  {
    decltype(local_frame::find_result::crossed_fns) crossed_fns;

    for(local_frame_ptr it{ start }; it != nullptr;)
    {
      auto const local_result(it->locals.find(sym));
      if(local_result != it->locals.end())
      {
        return local_frame::find_result{ local_result->second, std::move(crossed_fns) };
      }

      if(allow_captures)
      {
        auto const capture_result(it->captures.find(sym));
        if(capture_result != it->locals.end())
        {
          return local_frame::find_result{ capture_result->second, std::move(crossed_fns) };
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

    throw std::runtime_error{ fmt::format("unable to find local: {}", sym->to_string()) };
  }

  option<local_frame::find_result>
  local_frame::find_local_or_capture(runtime::obj::symbol_ptr const sym)
  {
    return find_local_impl(this, sym, true);
  }

  void local_frame::register_captures(find_result const &result)
  {
    for(auto const &crossed_fn : result.crossed_fns)
    {
      auto res(crossed_fn->captures.emplace(result.binding.name, result.binding));
      /* We know it needs a box, since it's captured. */
      res.first->second.needs_box = true;
      res.first->second.has_boxed_usage = true;
      /* To start with, we assume it's only boxed. */
      res.first->second.has_unboxed_usage = false;
    }
  }

  option<local_frame::find_result> local_frame::find_originating_local(runtime::obj::symbol_ptr sym)
  {
    return find_local_impl(this, sym, false);
  }

  local_frame const &local_frame::find_closest_fn_frame(local_frame const &frame)
  {
    if(frame.type == local_frame::frame_type::fn)
    {
      return frame;
    }
    else if(frame.parent.is_some())
    {
      return find_closest_fn_frame(*frame.parent.unwrap());
    }

    /* Default to the root frame, if there is no fn frame. */
    return frame;
  }
  local_frame &local_frame::find_closest_fn_frame(local_frame &frame)
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast): Avoiding duplication.
  {
    return const_cast<local_frame &>(find_closest_fn_frame(std::as_const(frame)));
  }

  native_bool local_frame::within_same_fn(local_frame_ptr const l, local_frame_ptr const r)
  {
    return &find_closest_fn_frame(*l) == &find_closest_fn_frame(*r);
  }

  runtime::obj::symbol_ptr local_frame::lift_var(runtime::obj::symbol_ptr const &sym)
  {
    auto &closest_fn(find_closest_fn_frame(*this));
    auto const &found(closest_fn.lifted_vars.find(sym));
    if(found != closest_fn.lifted_vars.end())
    {
      return found->first;
    }

    auto qualified_sym(make_box<runtime::obj::symbol>(*sym));
    if(qualified_sym->ns.empty())
    {
      qualified_sym->ns
        = runtime::expect_object<runtime::ns>(rt_ctx.current_ns_var->deref())->name->name;
    }
    /* We use unique native names, just so var names don't clash with the underlying C++ API. */
    lifted_var lv{ runtime::context::unique_symbol(runtime::munge(qualified_sym->name)),
                   qualified_sym };
    closest_fn.lifted_vars.emplace(qualified_sym, std::move(lv));
    return qualified_sym;
  }

  option<std::reference_wrapper<lifted_var const>>
  local_frame::find_lifted_var(runtime::obj::symbol_ptr const &sym) const
  {
    auto const &closest_fn(find_closest_fn_frame(*this));
    auto const &found(closest_fn.lifted_vars.find(sym));
    if(found != closest_fn.lifted_vars.end())
    {
      return some(std::ref(found->second));
    }
    return none;
  }

  void local_frame::lift_constant(runtime::object_ptr const constant)
  {
    auto &closest_fn(find_closest_fn_frame(*this));
    auto const &found(closest_fn.lifted_constants.find(constant));
    if(found != closest_fn.lifted_constants.end())
    {
      return;
    }

    auto name(runtime::context::unique_symbol("const"));
    option<runtime::obj::symbol> unboxed_name{ runtime::visit_object(
      [&](auto const typed_constant) -> option<runtime::obj::symbol> {
        using T = typename decltype(typed_constant)::value_type;

        if constexpr(runtime::behavior::numberable<T>)
        {
          return runtime::obj::symbol{ name.ns, name.name + "__unboxed" };
        }
        else
        {
          return none;
        }
      },
      constant) };

    lifted_constant l{ std::move(name), std::move(unboxed_name), constant };
    closest_fn.lifted_constants.emplace(constant, std::move(l));
  }

  option<std::reference_wrapper<lifted_constant const>>
  local_frame::find_lifted_constant(runtime::object_ptr const o) const
  {
    auto const &closest_fn(find_closest_fn_frame(*this));
    auto const &found(closest_fn.lifted_constants.find(o));
    if(found != closest_fn.lifted_constants.end())
    {
      return some(std::ref(found->second));
    }
    return none;
  }

  runtime::object_ptr local_frame::to_runtime_data() const
  {
    return runtime::obj::persistent_array_map::create_unique(
      make_box("__type"),
      make_box("local_frame"),
      make_box("type"),
      make_box(magic_enum::enum_name(type)),
      make_box("parent"),
      detail::to_runtime_data(parent),
      make_box("locals"),
      detail::to_runtime_data(locals),
      make_box("captures"),
      detail::to_runtime_data(captures),
      make_box("lifted_vars"),
      detail::to_runtime_data(lifted_vars),
      make_box("lifted_constants"),
      detail::to_runtime_data(lifted_constants));
  }
}
