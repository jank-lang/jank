#pragma once

#include <list>

#include <jank/runtime/obj/symbol.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expr/do.hpp>
#include <jank/analyze/expression_base.hpp>
#include <jank/detail/to_runtime_data.hpp>
#include <jank/runtime/seq.hpp>

namespace jank::analyze::expr
{
  struct function_context : gc
  {
    static constexpr native_bool pointer_free{ true };

    size_t param_count{};
    native_bool is_variadic{};
    native_bool is_tail_recursive{};
    /* TODO: is_pure */
  };

  using function_context_ptr = native_box<function_context>;

  template <typename E>
  struct function_arity
  {
    native_vector<runtime::obj::symbol_ptr> params;
    do_<E> body;
    local_frame_ptr frame{};
    function_context_ptr fn_ctx{};

    runtime::object_ptr to_runtime_data() const
    {
      runtime::object_ptr param_maps(make_box<runtime::obj::persistent_vector>());
      for(auto const &e : params)
      {
        param_maps = runtime::conj(param_maps, e);
      }

      return runtime::obj::persistent_array_map::create_unique(make_box("__type"),
                                                               make_box("expr::function_arity"),
                                                               make_box("params"),
                                                               param_maps,
                                                               make_box("body"),
                                                               detail::to_runtime_data(body),
                                                               make_box("frame"),
                                                               detail::to_runtime_data(frame),
                                                               make_box("fn_ctx"),
                                                               detail::to_runtime_data(fn_ctx));
    }
  };

  struct arity_key
  {
    native_bool operator==(arity_key const &rhs) const
    {
      return param_count == rhs.param_count && is_variadic == rhs.is_variadic;
    }

    size_t param_count{};
    native_bool is_variadic{};
  };

  template <typename E>
  struct function : expression_base
  {
    native_persistent_string name;
    native_persistent_string unique_name;
    native_vector<function_arity<E>> arities;

    runtime::object_ptr to_runtime_data() const
    {
      runtime::object_ptr arity_maps(make_box<runtime::obj::persistent_vector>());
      for(auto const &e : arities)
      {
        arity_maps = runtime::conj(arity_maps, e.to_runtime_data());
      }

      return runtime::merge(
        static_cast<expression_base const *>(this)->to_runtime_data(),
        runtime::obj::persistent_array_map::create_unique(make_box("__type"),
                                                          make_box("expr::function"),
                                                          make_box("name"),
                                                          detail::to_runtime_data(name),
                                                          make_box("unique_name"),
                                                          detail::to_runtime_data(unique_name),
                                                          make_box("arities"),
                                                          arity_maps));
    }
  };
}

namespace std
{
  template <>
  struct hash<jank::analyze::expr::arity_key>
  {
    size_t operator()(jank::analyze::expr::arity_key const &k) const noexcept
    {
      static auto hasher(std::hash<decltype(jank::analyze::expr::arity_key::param_count)>{});
      return jank::hash::combine(hasher(k.param_count), k.is_variadic);
    }
  };
}
