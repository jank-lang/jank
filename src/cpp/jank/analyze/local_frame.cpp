#include <functional>

#include <fmt/core.h>

#include <jank/runtime/util.hpp>
#include <jank/runtime/behavior/numberable.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/analyze/local_frame.hpp>

namespace jank::analyze
{
  local_frame::local_frame
  (
    frame_type const &type,
    runtime::context &rt_ctx,
    option<native_box<local_frame>> const &p
  ) : type{ type }, parent{ p }, rt_ctx{ rt_ctx }
  { }

  local_frame& local_frame::operator=(local_frame const &rhs)
  {
    if(this == &rhs)
    { return *this; }

    type = rhs.type;
    parent = rhs.parent;
    locals = rhs.locals;

    return *this;
  }
  local_frame& local_frame::operator=(local_frame &&rhs)
  {
    if(this == &rhs)
    { return *this; }

    type = rhs.type;
    parent = std::move(rhs.parent);
    locals = std::move(rhs.locals);

    return *this;
  }

  option<local_frame::find_result> local_frame::find_capture(runtime::obj::symbol_ptr const sym)
  {
    decltype(local_frame::find_result::crossed_fns) crossed_fns;

    for(local_frame_ptr it{ this }; it != nullptr; )
    {
      auto const result(it->locals.find(sym));
      if(result != it->locals.end())
      { return local_frame::find_result{ result->second, std::move(crossed_fns) }; }
      else if(it->parent.is_some())
      {
        if(it->type == frame_type::fn)
        { crossed_fns.emplace_back(it); }
        it = it->parent.unwrap();
      }
      else
      { return none; }
    }

    throw "unable to find local";
  }

  void local_frame::register_captures(find_result const &result)
  {
    for(auto const &crossed_fn : result.crossed_fns)
    { crossed_fn->captures.emplace(result.binding.name, result.binding); }
  }

  local_frame const& find_closest_fn_frame(local_frame const &frame)
  {
    if(frame.type == local_frame::frame_type::fn)
    { return frame; }
    else if(frame.parent.is_some())
    { return find_closest_fn_frame(*frame.parent.unwrap()); }

    /* Default to the root frame, if there is no fn frame. */
    return frame;
  }
  local_frame& find_closest_fn_frame(local_frame &frame)
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast): Avoiding duplication.
  { return const_cast<local_frame&>(find_closest_fn_frame(std::as_const(frame))); }

  runtime::obj::symbol_ptr local_frame::lift_var(runtime::obj::symbol_ptr const &sym)
  {
    auto &closest_fn(find_closest_fn_frame(*this));
    auto const &found(closest_fn.lifted_vars.find(sym));
    if(found != closest_fn.lifted_vars.end())
    { return found->first; }

    auto qualified_sym(make_box<runtime::obj::symbol>(*sym));
    if(qualified_sym->ns.empty())
    {
      auto const state(rt_ctx.get_thread_state());
      qualified_sym->ns = runtime::expect_object<runtime::ns>(state.current_ns->get_root())->name->name;
    }
    /* We use unique native names, just so var names don't clash with the underlying C++ API. */
    lifted_var lv
    { runtime::context::unique_symbol(runtime::munge(qualified_sym->name)), qualified_sym };
    closest_fn.lifted_vars.emplace(qualified_sym, std::move(lv));
    return qualified_sym;
  }

  option<std::reference_wrapper<lifted_var const>> local_frame::find_lifted_var
  (runtime::obj::symbol_ptr const &sym) const
  {
    auto const &closest_fn(find_closest_fn_frame(*this));
    auto const &found(closest_fn.lifted_vars.find(sym));
    if(found != closest_fn.lifted_vars.end())
    { return some(std::ref(found->second)); }
    return none;
  }

  void local_frame::lift_constant(runtime::object_ptr const constant)
  {
    auto &closest_fn(find_closest_fn_frame(*this));
    auto const &found(closest_fn.lifted_constants.find(constant));
    if(found != closest_fn.lifted_constants.end())
    { return; }

    auto name(runtime::context::unique_symbol("const"));
    option<runtime::obj::symbol> unboxed_name
    {
      runtime::visit_object
      (
        constant,
        [&](auto const typed_constant) -> option<runtime::obj::symbol>
        {
          using T = typename decltype(typed_constant)::value_type;

          if constexpr(runtime::behavior::numberable<T>)
          { return runtime::obj::symbol{ name.ns, name.name + "__unboxed" }; }
          else
          { return none; }
        }
      )
    };

    lifted_constant l{ std::move(name), std::move(unboxed_name), constant };
    closest_fn.lifted_constants.emplace(constant, std::move(l));
  }

  option<std::reference_wrapper<lifted_constant const>> local_frame::find_lifted_constant
  (runtime::object_ptr const o) const
  {
    auto const &closest_fn(find_closest_fn_frame(*this));
    auto const &found(closest_fn.lifted_constants.find(o));
    if(found != closest_fn.lifted_constants.end())
    { return some(std::ref(found->second)); }
    return none;
  }
}
