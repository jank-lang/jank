#include <functional>

#include <jank/runtime/context.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/behavior/number_like.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/cpp_util.hpp>
#include <jank/detail/to_runtime_data.hpp>
#include <jank/util/fmt.hpp>

namespace jank::analyze
{
  using namespace jank::runtime;

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

  local_frame::local_frame(frame_type const &type, jtl::option<jtl::ptr<local_frame>> const &p)
    : type{ type }
    , parent{ p }
  {
  }

  static jtl::option<local_frame::binding_find_result>
  find_local_impl(local_frame_ptr const start, obj::symbol_ref const sym, bool const allow_captures)
  {
    decltype(local_frame::binding_find_result::crossed_fns) crossed_fns;

    for(local_frame_ptr it{ start }; it != nullptr;)
    {
      auto const local_result(it->locals.find(sym));
      if(local_result != it->locals.end())
      {
        return local_frame::binding_find_result{ &local_result->second, std::move(crossed_fns) };
      }

      if(allow_captures)
      {
        auto const capture_result(it->captures.find(sym));
        if(capture_result != it->locals.end())
        {
          return local_frame::binding_find_result{ &capture_result->second,
                                                   std::move(crossed_fns) };
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

  jtl::option<local_frame::binding_find_result>
  local_frame::find_local_or_capture(obj::symbol_ref const sym)
  {
    return find_local_impl(this, sym, true);
  }

  void local_frame::register_captures(binding_find_result const &result)
  {
    for(auto const &crossed_fn : result.crossed_fns)
    {
      /* We intentionally copy the binding here. */
      auto res(crossed_fn->captures.emplace(result.binding->name, *result.binding));
      /* We know it needs a box, since it's captured. */
      res.first->second.needs_box = true;
      res.first->second.has_boxed_usage = true;
      /* To start with, we assume it's only boxed. */
      res.first->second.has_unboxed_usage = false;

      /* Native values which are captured get auto-boxed, so we need to adjust the type
       * of the binding. */
      if(!cpp_util::is_any_object(res.first->second.type))
      {
        res.first->second.type = cpp_util::untyped_object_ptr_type();
      }
    }
  }

  void local_frame::register_captures(local_frame_ptr const frame,
                                      named_recursion_find_result const &result)
  {
    local_binding b{ make_box<obj::symbol>(result.fn_frame->fn_ctx->name),
                     result.fn_frame->fn_ctx->name,
                     none,
                     frame };
    if(result.fn_frame->fn_ctx->fn)
    {
      b.value_expr = result.fn_frame->fn_ctx->fn;
    }
    register_captures(binding_find_result{ &b, result.crossed_fns });
  }

  jtl::option<local_frame::binding_find_result>
  local_frame::find_originating_local(obj::symbol_ref const sym)
  {
    return find_local_impl(this, sym, false);
  }

  jtl::option<local_frame::named_recursion_find_result>
  local_frame::find_named_recursion(obj::symbol_ref const sym)
  {
    decltype(local_frame::named_recursion_find_result::crossed_fns) crossed_fns;

    auto const sym_str(sym->to_string());
    for(local_frame_ptr it{ this }; it != nullptr;)
    {
      if(it->type == frame_type::fn && it->fn_ctx->name == sym_str)
      {
        return local_frame::named_recursion_find_result{ it, jtl::move(crossed_fns) };
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

  object_ref local_frame::to_runtime_data() const
  {
    return obj::persistent_array_map::create_unique(make_box("type"),
                                                    make_box(frame_type_str(type)),
                                                    make_box("parent"),
                                                    jank::detail::to_runtime_data(parent),
                                                    make_box("locals"),
                                                    jank::detail::to_runtime_data(locals),
                                                    make_box("captures"),
                                                    jank::detail::to_runtime_data(captures));
  }
}
