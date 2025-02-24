#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/behavior/comparable.hpp>
#include <jank/runtime/behavior/nameable.hpp>
#include <jank/runtime/behavior/derefable.hpp>
#include <jank/runtime/context.hpp>

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
    return object_type_str(o->type);
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

  native_bool is_char(object_ptr const o)
  {
    return o->type == object_type::character;
  }

  native_bool is_symbol(object_ptr const o)
  {
    return o->type == object_type::symbol;
  }

  native_bool is_simple_symbol(object_ptr const o)
  {
    return o->type == object_type::symbol && expect_object<obj::symbol>(o)->ns.empty();
  }

  native_bool is_qualified_symbol(object_ptr const o)
  {
    return o->type == object_type::symbol && !expect_object<obj::symbol>(o)->ns.empty();
  }

  object_ptr print(object_ptr const args)
  {
    visit_object(
      [](auto const typed_args) {
        using T = typename decltype(typed_args)::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          util::string_builder buff;
          runtime::to_string(typed_args->first(), buff);
          for(auto it(next_in_place(typed_args)); it != nullptr; it = next_in_place(it))
          {
            buff(' ');
            runtime::to_string(it->first(), buff);
          }
          std::fwrite(buff.data(), 1, buff.size(), stdout);
        }
        else
        {
          throw std::runtime_error{ fmt::format("expected a sequence: {}",
                                                typed_args->to_string()) };
        }
      },
      args);
    return obj::nil::nil_const();
  }

  object_ptr println(object_ptr const args)
  {
    visit_object(
      [](auto const typed_more) {
        using T = typename decltype(typed_more)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          std::putc('\n', stdout);
        }
        else if constexpr(behavior::sequenceable<T>)
        {
          util::string_builder buff;
          runtime::to_string(typed_more->first(), buff);
          for(auto it(next_in_place(typed_more)); it != nullptr; it = next_in_place(it))
          {
            buff(' ');
            runtime::to_string(it->first(), buff);
          }
          std::fwrite(buff.data(), 1, buff.size(), stdout);
          std::putc('\n', stdout);
        }
        else
        {
          throw std::runtime_error{ fmt::format("expected a sequence: {}",
                                                typed_more->to_string()) };
        }
      },
      args);
    return obj::nil::nil_const();
  }

  object_ptr pr(object_ptr const args)
  {
    visit_object(
      [](auto const typed_args) {
        using T = typename decltype(typed_args)::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          util::string_builder buff;
          runtime::to_code_string(typed_args->first(), buff);
          for(auto it(next_in_place(typed_args)); it != nullptr; it = next_in_place(it))
          {
            buff(' ');
            runtime::to_code_string(it->first(), buff);
          }
          std::fwrite(buff.data(), 1, buff.size(), stdout);
        }
        else
        {
          throw std::runtime_error{ fmt::format("expected a sequence: {}",
                                                typed_args->to_string()) };
        }
      },
      args);
    return obj::nil::nil_const();
  }

  object_ptr prn(object_ptr const args)
  {
    visit_object(
      [](auto const typed_more) {
        using T = typename decltype(typed_more)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          std::putc('\n', stdout);
        }
        else if constexpr(behavior::sequenceable<T>)
        {
          util::string_builder buff;
          runtime::to_code_string(typed_more->first(), buff);
          for(auto it(next_in_place(typed_more)); it != nullptr; it = next_in_place(it))
          {
            buff(' ');
            runtime::to_code_string(it->first(), buff);
          }
          std::fwrite(buff.data(), 1, buff.size(), stdout);
          std::putc('\n', stdout);
        }
        else
        {
          throw std::runtime_error{ fmt::format("expected a sequence: {}",
                                                typed_more->to_string()) };
        }
      },
      args);
    return obj::nil::nil_const();
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
      return false;
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

  object_ptr namespace_(object_ptr const o)
  {
    return visit_object(
      [](auto const typed_o) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::nameable<T>)
        {
          auto const ns(typed_o->get_namespace());
          return (ns.empty() ? obj::nil::nil_const() : make_box<obj::persistent_string>(ns));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not nameable: {}", typed_o->to_string()) };
        }
      },
      o);
  }

  object_ptr keyword(object_ptr const ns, object_ptr const name)
  {
    return __rt_ctx->intern_keyword(runtime::to_string(ns), runtime::to_string(name)).expect_ok();
  }

  native_bool is_keyword(object_ptr const o)
  {
    return o->type == object_type::keyword;
  }

  native_bool is_simple_keyword(object_ptr const o)
  {
    return o->type == object_type::keyword && expect_object<obj::keyword>(o)->sym->ns.empty();
  }

  native_bool is_qualified_keyword(object_ptr const o)
  {
    return o->type == object_type::keyword && !expect_object<obj::keyword>(o)->sym->ns.empty();
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
    return make_box<obj::symbol>(__rt_ctx->unique_symbol(to_string(o)));
  }

  object_ptr atom(object_ptr const o)
  {
    return make_box<obj::atom>(o);
  }

  object_ptr swap_atom(object_ptr const atom, object_ptr const fn)
  {
    return try_object<obj::atom>(atom)->swap(fn);
  }

  object_ptr swap_atom(object_ptr const atom, object_ptr const fn, object_ptr const a1)
  {
    return try_object<obj::atom>(atom)->swap(fn, a1);
  }

  object_ptr
  swap_atom(object_ptr const atom, object_ptr const fn, object_ptr const a1, object_ptr const a2)
  {
    return try_object<obj::atom>(atom)->swap(fn, a1, a2);
  }

  object_ptr swap_atom(object_ptr const atom,
                       object_ptr const fn,
                       object_ptr const a1,
                       object_ptr const a2,
                       object_ptr const rest)
  {
    return try_object<obj::atom>(atom)->swap(fn, a1, a2, rest);
  }

  object_ptr swap_vals(object_ptr const atom, object_ptr const fn)
  {
    return try_object<obj::atom>(atom)->swap_vals(fn);
  }

  object_ptr swap_vals(object_ptr const atom, object_ptr const fn, object_ptr const a1)
  {
    return try_object<obj::atom>(atom)->swap_vals(fn, a1);
  }

  object_ptr
  swap_vals(object_ptr const atom, object_ptr const fn, object_ptr const a1, object_ptr const a2)
  {
    return try_object<obj::atom>(atom)->swap_vals(fn, a1, a2);
  }

  object_ptr swap_vals(object_ptr const atom,
                       object_ptr const fn,
                       object_ptr const a1,
                       object_ptr const a2,
                       object_ptr const rest)
  {
    return try_object<obj::atom>(atom)->swap_vals(fn, a1, a2, rest);
  }

  object_ptr
  compare_and_set(object_ptr const atom, object_ptr const old_val, object_ptr const new_val)
  {
    return try_object<obj::atom>(atom)->compare_and_set(old_val, new_val);
  }

  object_ptr reset(object_ptr const atom, object_ptr const new_val)
  {
    return try_object<obj::atom>(atom)->reset(new_val);
  }

  object_ptr reset_vals(object_ptr const atom, object_ptr const new_val)
  {
    return try_object<obj::atom>(atom)->reset_vals(new_val);
  }

  object_ptr deref(object_ptr const o)
  {
    return visit_object(
      [=](auto const typed_o) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::derefable<T>)
        {
          return typed_o->deref();
        }
        else
        {
          throw std::runtime_error{ fmt::format("not derefable: {}", typed_o->to_string()) };
        }
      },
      o);
  }

  object_ptr volatile_(object_ptr const o)
  {
    return make_box<obj::volatile_>(o);
  }

  native_bool is_volatile(object_ptr const o)
  {
    return o->type == object_type::volatile_;
  }

  object_ptr vswap(object_ptr const v, object_ptr const fn, object_ptr const args)
  {
    auto const v_obj(expect_object<obj::volatile_>(v));
    return v_obj->reset(apply_to(fn, make_box<obj::cons>(v_obj->deref(), args)));
  }

  object_ptr vreset(object_ptr const v, object_ptr const new_val)
  {
    return expect_object<obj::volatile_>(v)->reset(new_val);
  }

  void push_thread_bindings(object_ptr const o)
  {
    __rt_ctx->push_thread_bindings(o).expect_ok();
  }

  void pop_thread_bindings()
  {
    __rt_ctx->pop_thread_bindings().expect_ok();
  }

  object_ptr get_thread_bindings()
  {
    return __rt_ctx->get_thread_bindings();
  }

  object_ptr force(object_ptr const o)
  {
    if(o->type == object_type::delay)
    {
      return expect_object<obj::delay>(o)->deref();
    }
    return o;
  }

  object_ptr tagged_literal(object_ptr const tag, object_ptr const form)
  {
    return make_box<obj::tagged_literal>(tag, form);
  }

  native_bool is_tagged_literal(object_ptr const o)
  {
    return o->type == object_type::tagged_literal;
  }

}
