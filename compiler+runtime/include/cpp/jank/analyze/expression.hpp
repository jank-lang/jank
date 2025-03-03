#pragma once

#include <jank/runtime/object.hpp>

namespace jank::analyze
{
  using local_frame_ptr = runtime::native_box<struct local_frame>;

  enum class expression_position : uint8_t
  {
    value,
    statement,
    tail
  };

  constexpr char const *expression_position_str(expression_position const pos)
  {
    switch(pos)
    {
      case expression_position::value:
        return "value";
      case expression_position::statement:
        return "statement";
      case expression_position::tail:
        return "tail";
    }
    return "unknown";
  }

  enum class expression_kind : uint8_t
  {
    uninitialized,
    def,
    var_deref,
    var_ref,
    call,
    primitive_literal,
    list,
    vector,
    map,
    set,
    function,
    recur,
    recursion_reference,
    named_recursion,
    local_reference,
    let,
    letfn,
    do_,
    if_,
    throw_,
    try_,
    case_
  };

  constexpr char const *expression_kind_str(expression_kind const kind)
  {
    switch(kind)
    {
      case expression_kind::uninitialized:
        return "uninitialized";
      case expression_kind::def:
        return "def";
      case expression_kind::var_deref:
        return "var_deref";
      case expression_kind::var_ref:
        return "var_ref";
      case expression_kind::call:
        return "call";
      case expression_kind::primitive_literal:
        return "primitive_literal";
      case expression_kind::list:
        return "list";
      case expression_kind::vector:
        return "vector";
      case expression_kind::map:
        return "map";
      case expression_kind::set:
        return "set";
      case expression_kind::function:
        return "function";
      case expression_kind::recur:
        return "recur";
      case expression_kind::recursion_reference:
        return "recursion_reference";
      case expression_kind::named_recursion:
        return "named_recursion";
      case expression_kind::local_reference:
        return "local_reference";
      case expression_kind::let:
        return "let";
      case expression_kind::letfn:
        return "letfn";
      case expression_kind::do_:
        return "do_";
      case expression_kind::if_:
        return "if_";
      case expression_kind::throw_:
        return "throw_";
      case expression_kind::try_:
        return "try_";
      case expression_kind::case_:
        return "case_";
    }
    return "unknown";
  }

  /* Common base class for every expression. */
  struct expression : gc
  {
    static constexpr native_bool pointer_free{ false };

    expression(expression_kind kind);
    expression(expression_kind kind,
               expression_position position,
               local_frame_ptr frame,
               native_bool needs_box);
    virtual ~expression() = default;

    virtual void propagate_position(expression_position const pos);
    virtual runtime::object_ptr to_runtime_data() const;

    expression_kind kind{};
    expression_position position{};
    local_frame_ptr frame{};
    native_bool needs_box{ true };
  };

  /* TODO: Use something non-nullable. */
  using expression_ptr = runtime::native_box<expression>;

  /* Captures both expressions and things which inherit from expression. */
  template <typename T>
  concept expression_like = std::convertible_to<T *, expression *>;
}
