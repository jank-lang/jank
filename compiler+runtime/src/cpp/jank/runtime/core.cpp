#include <jank/runtime/core.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/behavior/nameable.hpp>
#include <jank/runtime/behavior/derefable.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/sequence_range.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime
{
  jtl::immutable_string type(object_ref const o)
  {
    return object_type_str(o->type);
  }

  bool is_nil(object_ref const o)
  {
    return o == jank_nil;
  }

  bool is_true(object_ref const o)
  {
    return o == jank_true;
  }

  bool is_false(object_ref const o)
  {
    return o == jank_false;
  }

  bool is_some(object_ref const o)
  {
    return o != jank_nil;
  }

  bool is_string(object_ref const o)
  {
    return o->type == object_type::persistent_string;
  }

  bool is_char(object_ref const o)
  {
    return o->type == object_type::character;
  }

  bool is_symbol(object_ref const o)
  {
    return o->type == object_type::symbol;
  }

  bool is_simple_symbol(object_ref const o)
  {
    return o->type == object_type::symbol && expect_object<obj::symbol>(o)->ns.empty();
  }

  bool is_qualified_symbol(object_ref const o)
  {
    return o->type == object_type::symbol && !expect_object<obj::symbol>(o)->ns.empty();
  }

  object_ref print(object_ref const args)
  {
    visit_object(
      [](auto const typed_args) {
        using T = typename decltype(typed_args)::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          util::string_builder buff;
          runtime::to_string(typed_args->first().erase(), buff);
          for(auto const e : make_sequence_range(typed_args).skip(1))
          {
            buff(' ');
            runtime::to_string(e.erase(), buff);
          }
          std::fwrite(buff.data(), 1, buff.size(), stdout);
        }
        else
        {
          throw std::runtime_error{ util::format("expected a sequence: {}",
                                                 typed_args->to_string()) };
        }
      },
      args);
    return jank_nil;
  }

  object_ref println(object_ref const args)
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
          runtime::to_string(typed_more->first().erase(), buff);
          for(auto const e : make_sequence_range(typed_more).skip(1))
          {
            buff(' ');
            runtime::to_string(e.erase(), buff);
          }
          std::fwrite(buff.data(), 1, buff.size(), stdout);
          std::putc('\n', stdout);
        }
        else
        {
          throw std::runtime_error{ util::format("expected a sequence: {}",
                                                 typed_more->to_string()) };
        }
      },
      args);
    return jank_nil;
  }

  object_ref pr(object_ref const args)
  {
    visit_object(
      [](auto const typed_args) {
        using T = typename decltype(typed_args)::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          util::string_builder buff;
          runtime::to_code_string(typed_args->first().erase(), buff);
          for(auto const e : make_sequence_range(typed_args).skip(1))
          {
            buff(' ');
            runtime::to_code_string(e.erase(), buff);
          }
          std::fwrite(buff.data(), 1, buff.size(), stdout);
        }
        else
        {
          throw std::runtime_error{ util::format("expected a sequence: {}",
                                                 typed_args->to_string()) };
        }
      },
      args);
    return jank_nil;
  }

  object_ref prn(object_ref const args)
  {
    visit_object(
      [](auto const typed_args) {
        using T = typename decltype(typed_args)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          std::putc('\n', stdout);
        }
        else if constexpr(behavior::sequenceable<T>)
        {
          util::string_builder buff;
          runtime::to_code_string(typed_args->first().erase(), buff);
          for(auto const e : make_sequence_range(typed_args).skip(1))
          {
            buff(' ');
            runtime::to_code_string(e.erase(), buff);
          }
          std::fwrite(buff.data(), 1, buff.size(), stdout);
          std::putc('\n', stdout);
        }
        else
        {
          throw std::runtime_error{ util::format("expected a sequence: {}",
                                                 typed_args->to_string()) };
        }
      },
      args);
    return jank_nil;
  }

  f64 to_real(object_ref const o)
  {
    return visit_number_like(
      [](auto const typed_o) -> f64 { return typed_o->to_real(); },
      [=]() -> f64 {
        throw std::runtime_error{ util::format("not a number: {}", to_string(o)) };
      },
      o);
  }

  obj::persistent_string_ref subs(object_ref const s, object_ref const start)
  {
    return visit_type<obj::persistent_string>(
      [](auto const typed_s, i64 const start) -> obj::persistent_string_ref {
        return typed_s->substring(start).expect_ok();
      },
      s,
      to_int(start));
  }

  obj::persistent_string_ref subs(object_ref const s, object_ref const start, object_ref const end)
  {
    return visit_type<obj::persistent_string>(
      [](auto const typed_s, i64 const start, i64 const end)
        -> obj::persistent_string_ref { return typed_s->substring(start, end).expect_ok(); },
      s,
      to_int(start),
      to_int(end));
  }

  i64 first_index_of(object_ref const s, object_ref const m)
  {
    return visit_type<obj::persistent_string>(
      [](auto const typed_s, object_ref const m) -> i64 {
        return typed_s->first_index_of(m);
      },
      s,
      m);
  }

  i64 last_index_of(object_ref const s, object_ref const m)
  {
    return visit_type<obj::persistent_string>(
      [](auto const typed_s, object_ref const m) -> i64 {
        return typed_s->last_index_of(m);
      },
      s,
      m);
  }

  bool is_named(object_ref const o)
  {
    return visit_object(
      [](auto const typed_o) {
        using T = typename decltype(typed_o)::value_type;

        return behavior::nameable<T>;
      },
      o);
  }

  jtl::immutable_string name(object_ref const o)
  {
    return visit_object(
      [](auto const typed_o) -> jtl::immutable_string {
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
          throw std::runtime_error{ util::format("not nameable: {}", typed_o->to_string()) };
        }
      },
      o);
  }

  object_ref namespace_(object_ref const o)
  {
    return visit_object(
      [](auto const typed_o) -> object_ref {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::nameable<T>)
        {
          auto const ns(typed_o->get_namespace());
          if(ns.empty())
          {
            return jank_nil;
          }
          return make_box<obj::persistent_string>(ns);
        }
        else
        {
          throw std::runtime_error{ util::format("not nameable: {}", typed_o->to_string()) };
        }
      },
      o);
  }

  object_ref keyword(object_ref const ns, object_ref const name)
  {
    return __rt_ctx->intern_keyword(runtime::to_string(ns), runtime::to_string(name)).expect_ok();
  }

  bool is_keyword(object_ref const o)
  {
    return o->type == object_type::keyword;
  }

  bool is_simple_keyword(object_ref const o)
  {
    return o->type == object_type::keyword && expect_object<obj::keyword>(o)->sym->ns.empty();
  }

  bool is_qualified_keyword(object_ref const o)
  {
    return o->type == object_type::keyword && !expect_object<obj::keyword>(o)->sym->ns.empty();
  }

  bool is_callable(object_ref const o)
  {
    return visit_object(
      [=](auto const typed_o) -> bool {
        using T = typename decltype(typed_o)::value_type;

        return std::is_base_of_v<behavior::callable, T>;
      },
      o);
  }

  uhash to_hash(object_ref const o)
  {
    return visit_object([=](auto const typed_o) -> uhash { return typed_o->to_hash(); }, o);
  }

  object_ref macroexpand1(object_ref const o)
  {
    return __rt_ctx->macroexpand1(o);
  }

  object_ref macroexpand(object_ref const o)
  {
    return __rt_ctx->macroexpand(o);
  }

  object_ref gensym(object_ref const o)
  {
    return make_box<obj::symbol>(__rt_ctx->unique_symbol(to_string(o)));
  }

  object_ref atom(object_ref const o)
  {
    return make_box<obj::atom>(o);
  }

  object_ref swap_atom(object_ref const atom, object_ref const fn)
  {
    return try_object<obj::atom>(atom)->swap(fn);
  }

  object_ref swap_atom(object_ref const atom, object_ref const fn, object_ref const a1)
  {
    return try_object<obj::atom>(atom)->swap(fn, a1);
  }

  object_ref
  swap_atom(object_ref const atom, object_ref const fn, object_ref const a1, object_ref const a2)
  {
    return try_object<obj::atom>(atom)->swap(fn, a1, a2);
  }

  object_ref swap_atom(object_ref const atom,
                       object_ref const fn,
                       object_ref const a1,
                       object_ref const a2,
                       object_ref const rest)
  {
    return try_object<obj::atom>(atom)->swap(fn, a1, a2, rest);
  }

  object_ref swap_vals(object_ref const atom, object_ref const fn)
  {
    return try_object<obj::atom>(atom)->swap_vals(fn);
  }

  object_ref swap_vals(object_ref const atom, object_ref const fn, object_ref const a1)
  {
    return try_object<obj::atom>(atom)->swap_vals(fn, a1);
  }

  object_ref
  swap_vals(object_ref const atom, object_ref const fn, object_ref const a1, object_ref const a2)
  {
    return try_object<obj::atom>(atom)->swap_vals(fn, a1, a2);
  }

  object_ref swap_vals(object_ref const atom,
                       object_ref const fn,
                       object_ref const a1,
                       object_ref const a2,
                       object_ref const rest)
  {
    return try_object<obj::atom>(atom)->swap_vals(fn, a1, a2, rest);
  }

  object_ref
  compare_and_set(object_ref const atom, object_ref const old_val, object_ref const new_val)
  {
    return try_object<obj::atom>(atom)->compare_and_set(old_val, new_val);
  }

  object_ref reset(object_ref const atom, object_ref const new_val)
  {
    return try_object<obj::atom>(atom)->reset(new_val);
  }

  object_ref reset_vals(object_ref const atom, object_ref const new_val)
  {
    return try_object<obj::atom>(atom)->reset_vals(new_val);
  }

  object_ref deref(object_ref const o)
  {
    return visit_object(
      [=](auto const typed_o) -> object_ref {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::derefable<T>)
        {
          return typed_o->deref();
        }
        else
        {
          throw std::runtime_error{ util::format("not derefable: {}", typed_o->to_string()) };
        }
      },
      o);
  }

  object_ref volatile_(object_ref const o)
  {
    return make_box<obj::volatile_>(o);
  }

  bool is_volatile(object_ref const o)
  {
    return o->type == object_type::volatile_;
  }

  object_ref vswap(object_ref const v, object_ref const fn)
  {
    auto const v_obj(try_object<obj::volatile_>(v));
    return v_obj->reset(dynamic_call(fn, v_obj->deref()));
  }

  object_ref vswap(object_ref const v, object_ref const fn, object_ref const args)
  {
    auto const v_obj(try_object<obj::volatile_>(v));
    return v_obj->reset(apply_to(fn, make_box<obj::cons>(v_obj->deref(), args)));
  }

  object_ref vreset(object_ref const v, object_ref const new_val)
  {
    return try_object<obj::volatile_>(v)->reset(new_val);
  }

  void push_thread_bindings(object_ref const o)
  {
    __rt_ctx->push_thread_bindings(o).expect_ok();
  }

  void pop_thread_bindings()
  {
    __rt_ctx->pop_thread_bindings().expect_ok();
  }

  object_ref get_thread_bindings()
  {
    return __rt_ctx->get_thread_bindings();
  }

  object_ref force(object_ref const o)
  {
    if(o->type == object_type::delay)
    {
      return expect_object<obj::delay>(o)->deref();
    }
    return o;
  }

  object_ref tagged_literal(object_ref const tag, object_ref const form)
  {
    return make_box<obj::tagged_literal>(tag, form);
  }

  bool is_tagged_literal(object_ref const o)
  {
    return o->type == object_type::tagged_literal;
  }
}
