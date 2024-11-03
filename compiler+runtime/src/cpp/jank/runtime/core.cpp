#include <jank/runtime/core.hpp>

namespace jank::runtime
{
  native_integer compare(object_ptr const l, object_ptr const r)
  {
    if(l == r)
    {
      return 0;
    }

    if(l != obj::nil::nil_const())
    {
      if(r == obj::nil::nil_const())
      {
        return 1;
      }

      return visit_object(
        [](auto const typed_l, auto const r) -> native_integer {
          using L = typename decltype(typed_l)::value_type;
          if constexpr(behavior::comparable<L>)
          {
            return typed_l->compare(*r);
          }
          else
          {
            throw std::runtime_error{ fmt::format("not comparable: {}", typed_l->to_string()) };
          }
        },
        l,
        r);
    }

    return -1;
  }

  native_bool is_identical(object_ptr const lhs, object_ptr const rhs)
  {
    return lhs == rhs;
  }

  native_persistent_string type(object_ptr const o)
  {
    return magic_enum::enum_name(o->type);
  }

  native_bool is_nil(object_ptr const o)
  {
    return o == obj::nil::nil_const();
  }

  native_bool is_true(object_ptr const o)
  {
    return o == obj::boolean::true_const();
  }

  native_bool is_false(object_ptr const o)
  {
    return o == obj::boolean::false_const();
  }

  native_bool is_some(object_ptr const o)
  {
    return o != obj::nil::nil_const();
  }

  native_bool is_string(object_ptr const o)
  {
    return o->type == object_type::persistent_string;
  }

  native_bool is_symbol(object_ptr const o)
  {
    return o->type == object_type::symbol;
  }

  native_real to_real(object_ptr const o)
  {
    return visit_number_like(
      [](auto const typed_o) -> native_real { return typed_o->to_real(); },
      [=]() -> native_real {
        throw std::runtime_error{ fmt::format("not a number: {}", to_string(o)) };
      },
      o);
  }

  native_bool equal(char const lhs, object_ptr const rhs)
  {
    if(!rhs || rhs->type != object_type::character)
    {
      return false;
    }

    auto const typed_rhs = expect_object<obj::character>(rhs);
    return typed_rhs->to_hash() == static_cast<native_hash>(lhs);
  }

  native_bool equal(object_ptr const lhs, object_ptr const rhs)
  {
    if(!lhs)
    {
      return !rhs;
    }
    else if(!rhs)
    {
      return !lhs;
    }

    return visit_object([&](auto const typed_lhs) { return typed_lhs->equal(*rhs); }, lhs);
  }

  object_ptr meta(object_ptr const m)
  {
    if(m == nullptr || m == obj::nil::nil_const())
    {
      return obj::nil::nil_const();
    }

    return visit_object(
      [](auto const typed_m) -> object_ptr {
        using T = typename decltype(typed_m)::value_type;

        if constexpr(behavior::metadatable<T>)
        {
          return typed_m->meta.unwrap_or(obj::nil::nil_const());
        }
        else
        {
          return obj::nil::nil_const();
        }
      },
      m);
  }

  object_ptr with_meta(object_ptr const o, object_ptr const m)
  {
    return visit_object(
      [](auto const typed_o, object_ptr const m) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::metadatable<T>)
        {
          return typed_o->with_meta(m);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not metadatable: {}", to_string(m)) };
        }
      },
      o,
      m);
  }

  object_ptr reset_meta(object_ptr const o, object_ptr const m)
  {
    return visit_object(
      [](auto const typed_o, object_ptr const m) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::metadatable<T>)
        {
          auto const meta(behavior::detail::validate_meta(m));
          typed_o->meta = meta;
          return m;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not metadatable: {}", to_string(m)) };
        }
      },
      o,
      m);
  }

  obj::persistent_string_ptr subs(object_ptr const s, object_ptr const start)
  {
    return visit_type<obj::persistent_string>(
      [](auto const typed_s, native_integer const start) -> obj::persistent_string_ptr {
        return typed_s->substring(start).expect_ok();
      },
      s,
      to_int(start));
  }

  obj::persistent_string_ptr subs(object_ptr const s, object_ptr const start, object_ptr const end)
  {
    return visit_type<obj::persistent_string>(
      [](auto const typed_s, native_integer const start, native_integer const end)
        -> obj::persistent_string_ptr { return typed_s->substring(start, end).expect_ok(); },
      s,
      to_int(start),
      to_int(end));
  }

  native_integer first_index_of(object_ptr const s, object_ptr const m)
  {
    return visit_type<obj::persistent_string>(
      [](auto const typed_s, object_ptr const m) -> native_integer {
        return typed_s->first_index_of(m);
      },
      s,
      m);
  }

  native_integer last_index_of(object_ptr const s, object_ptr const m)
  {
    return visit_type<obj::persistent_string>(
      [](auto const typed_s, object_ptr const m) -> native_integer {
        return typed_s->last_index_of(m);
      },
      s,
      m);
  }

  native_bool is_named(object_ptr const o)
  {
    return visit_object(
      [](auto const typed_o) {
        using T = typename decltype(typed_o)::value_type;

        return behavior::nameable<T>;
      },
      o);
  }

  native_persistent_string name(object_ptr const o)
  {
    return visit_object(
      [](auto const typed_o) -> native_persistent_string {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(std::same_as<T, obj::persistent_string>)
        {
          return typed_o->data;
        }
        else if constexpr(behavior::nameable<T>)
        {
          return typed_o->get_name();
        }
        else
        {
          throw std::runtime_error{ fmt::format("not nameable: {}", typed_o->to_string()) };
        }
      },
      o);
  }

  native_persistent_string namespace_(object_ptr const o)
  {
    return visit_object(
      [](auto const typed_o) -> native_persistent_string {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::nameable<T>)
        {
          return typed_o->get_namespace();
        }
        else
        {
          throw std::runtime_error{ fmt::format("not nameable: {}", typed_o->to_string()) };
        }
      },
      o);
  }

  native_bool is_callable(object_ptr const o)
  {
    return visit_object(
      [=](auto const typed_o) -> native_bool {
        using T = typename decltype(typed_o)::value_type;

        return std::is_base_of_v<behavior::callable, T>;
      },
      o);
  }

  native_hash to_hash(object_ptr const o)
  {
    return visit_object([=](auto const typed_o) -> native_hash { return typed_o->to_hash(); }, o);
  }

  object_ptr macroexpand1(object_ptr const o)
  {
    return __rt_ctx->macroexpand1(o);
  }

  object_ptr macroexpand(object_ptr const o)
  {
    return __rt_ctx->macroexpand(o);
  }

  object_ptr gensym(object_ptr const o)
  {
    return make_box<obj::symbol>(runtime::context::unique_symbol(to_string(o)));
  }
}
