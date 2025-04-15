#include <jank/analyze/expr/function.hpp>
#include <jank/detail/to_runtime_data.hpp>
#include <jank/analyze/local_frame.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  function::function()
    : expression{ expr_kind }
  {
  }

  function::function(expression_position const position,
                     local_frame_ptr const frame,
                     native_bool const needs_box,
                     jtl::immutable_string const &name,
                     jtl::immutable_string const &unique_name,
                     native_vector<function_arity> &&arities,
                     runtime::obj::persistent_hash_map_ref const meta)
    : expression{ expr_kind, position, frame, needs_box }
    , name{ name }
    , unique_name{ unique_name }
    , arities{ std::move(arities) }
    , meta{ meta }
  {
  }

  object_ref function_arity::to_runtime_data() const
  {
    object_ref param_maps(make_box<obj::persistent_vector>());
    for(auto const e : params)
    {
      param_maps = conj(param_maps, e);
    }

    return obj::persistent_array_map::create_unique(make_box("params"),
                                                    param_maps,
                                                    make_box("body"),
                                                    body->to_runtime_data(),
                                                    make_box("frame"),
                                                    jank::detail::to_runtime_data(frame),
                                                    make_box("fn_ctx"),
                                                    jank::detail::to_runtime_data(fn_ctx));
  }

  native_bool arity_key::operator==(arity_key const &rhs) const
  {
    return param_count == rhs.param_count && is_variadic == rhs.is_variadic;
  }

  native_unordered_map<obj::symbol_ref, analyze::local_binding_ptr> function::captures() const
  {
    native_unordered_map<obj::symbol_ref, analyze::local_binding_ptr> ret;
    for(auto const &arity : arities)
    {
      for(auto const &capture : arity.frame->captures)
      {
        ret.emplace(capture.first, &capture.second);
      }
    }
    return ret;
  }

  object_ref function::to_runtime_data() const
  {
    auto arity_maps(make_box<obj::persistent_vector>());
    for(auto const &e : arities)
    {
      arity_maps = arity_maps->conj(e.to_runtime_data());
    }

    return merge(
      expression::to_runtime_data(),
      obj::persistent_array_map::create_unique(make_box("name"),
                                               jank::detail::to_runtime_data(name),
                                               make_box("unique_name"),
                                               jank::detail::to_runtime_data(unique_name),
                                               make_box("arities"),
                                               arity_maps));
  }
}

namespace std
{
  size_t hash<jank::analyze::expr::arity_key>::operator()(
    jank::analyze::expr::arity_key const &k) const noexcept
  {
    static auto hasher(std::hash<decltype(jank::analyze::expr::arity_key::param_count)>{});
    return jank::hash::combine(hasher(k.param_count), k.is_variadic);
  }
}
