#pragma once

#include <vector>
#include <list>

#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expr/do.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct function_arity
  {
    std::vector<runtime::obj::symbol_ptr> params;
    bool is_variadic{};
    do_<E> body;
    local_frame_ptr frame;
  };

  struct arity_key
  {
    bool operator==(arity_key const &rhs) const
    { return arg_count == rhs.arg_count && is_variadic == rhs.is_variadic; }

    size_t arg_count{};
    bool is_variadic{};
  };

  template <typename E>
  struct function
  {
    option<std::string> name;
    std::vector<function_arity<E>> arities;
  };
}

namespace std
{
  template <>
  struct hash<jank::analyze::expr::arity_key>
  {
    size_t operator()(jank::analyze::expr::arity_key const &k) const noexcept
    {
      static auto hasher(std::hash<decltype(jank::analyze::expr::arity_key::arg_count)>{});
      return jank::runtime::detail::hash_combine(hasher(k.arg_count), k.is_variadic);
    }
  };
}
