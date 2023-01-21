#pragma once

#include <list>

#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expr/do.hpp>
#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  struct function_context
  {
    size_t param_count{};
    bool is_variadic{};
    bool is_tail_recursive{};
    /* TODO: is_pure */
  };
  using function_context_ptr = native_box<function_context>;

  template <typename E>
  struct function_arity
  {
    native_vector<runtime::obj::symbol_ptr> params;
    do_<E> body;
    local_frame_ptr frame;
    function_context_ptr fn_ctx;
  };

  struct arity_key
  {
    bool operator==(arity_key const &rhs) const
    { return param_count == rhs.param_count && is_variadic == rhs.is_variadic; }

    size_t param_count{};
    bool is_variadic{};
  };

  template <typename E>
  struct function : expression_base
  {
    option<native_string> name;
    native_vector<function_arity<E>> arities;
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
      return jank::runtime::detail::hash_combine(hasher(k.param_count), k.is_variadic);
    }
  };
}
