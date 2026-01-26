#include <pthread.h>
#include <cxxabi.h>

#include <jank/runtime/core.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/behavior/nameable.hpp>
#include <jank/runtime/behavior/derefable.hpp>
#include <jank/runtime/behavior/ref_like.hpp>
#include <jank/runtime/behavior/realizable.hpp>
#include <jank/runtime/core/call.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/sequence_range.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::runtime
{
  jtl::immutable_string type(object_ref const o)
  {
    return object_type_str(o->type);
  }

  bool is_nil(object_ref const o)
  {
    return o == jank_nil();
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
    return o != jank_nil();
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

  object_ref to_unqualified_symbol(object_ref const o)
  {
    return runtime::visit_object(
      [&](auto const typed_o) -> object_ref {
        using T = typename jtl::decay_t<decltype(typed_o)>::value_type;

        if constexpr(std::same_as<T, obj::symbol>)
        {
          return typed_o;
        }
        else if constexpr(std::same_as<T, obj::persistent_string>)
        {
          return make_box<obj::symbol>(typed_o->data);
        }
        else if constexpr(std::same_as<T, var>)
        {
          return make_box<obj::symbol>(typed_o->n->name->name, typed_o->name->name);
        }
        else if constexpr(std::same_as<T, obj::keyword>)
        {
          return typed_o->sym;
        }
        else
        {
          throw std::runtime_error{ util::format("can't convert {} to a symbol",
                                                 typed_o->to_code_string()) };
        }
      },
      o);
  }

  object_ref to_qualified_symbol(object_ref const ns, object_ref const name)
  {
    return make_box<obj::symbol>(ns, name);
  }

  object_ref print(object_ref const args)
  {
    visit_object(
      [](auto const typed_args) {
        using T = typename jtl::decay_t<decltype(typed_args)>::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          jtl::string_builder buff;
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
    return jank_nil();
  }

  object_ref println(object_ref const args)
  {
    visit_object(
      [](auto const typed_more) {
        using T = typename jtl::decay_t<decltype(typed_more)>::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          std::putc('\n', stdout);
        }
        else if constexpr(behavior::sequenceable<T>)
        {
          jtl::string_builder buff;
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
    return jank_nil();
  }

  object_ref pr(object_ref const args)
  {
    visit_object(
      [](auto const typed_args) {
        using T = typename jtl::decay_t<decltype(typed_args)>::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          jtl::string_builder buff;
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
    return jank_nil();
  }

  object_ref prn(object_ref const args)
  {
    visit_object(
      [](auto const typed_args) {
        using T = typename jtl::decay_t<decltype(typed_args)>::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          std::putc('\n', stdout);
        }
        else if constexpr(behavior::sequenceable<T>)
        {
          jtl::string_builder buff;
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
    return jank_nil();
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
      [](auto const typed_s, i64 const start, i64 const end) -> obj::persistent_string_ref {
        return typed_s->substring(start, end).expect_ok();
      },
      s,
      to_int(start),
      to_int(end));
  }

  i64 first_index_of(object_ref const s, object_ref const m)
  {
    return visit_type<obj::persistent_string>(
      [](auto const typed_s, object_ref const m) -> i64 { return typed_s->first_index_of(m); },
      s,
      m);
  }

  i64 last_index_of(object_ref const s, object_ref const m)
  {
    return visit_type<obj::persistent_string>(
      [](auto const typed_s, object_ref const m) -> i64 { return typed_s->last_index_of(m); },
      s,
      m);
  }

  bool is_named(object_ref const o)
  {
    return visit_object(
      [](auto const typed_o) {
        using T = typename jtl::decay_t<decltype(typed_o)>::value_type;

        return behavior::nameable<T>;
      },
      o);
  }

  jtl::immutable_string name(object_ref const o)
  {
    return visit_object(
      [](auto const typed_o) -> jtl::immutable_string {
        using T = typename jtl::decay_t<decltype(typed_o)>::value_type;

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
        using T = typename jtl::decay_t<decltype(typed_o)>::value_type;

        if constexpr(behavior::nameable<T>)
        {
          auto const ns(typed_o->get_namespace());
          if(ns.empty())
          {
            return jank_nil();
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
    if(!ns.is_nil() && ns->type != object_type::persistent_string)
    {
      throw std::runtime_error{ util::format(
        "The 'keyword' function expects a namespace to be 'nil' or a 'string', got {} instead.",
        runtime::to_code_string(ns)) };
    }
    if(name->type != object_type::persistent_string)
    {
      throw std::runtime_error{ util::format(
        "The 'keyword' function expects the name to be a 'string', got {} instead.",
        runtime::to_code_string(name)) };
    }

    if(ns.is_nil())
    {
      return __rt_ctx->intern_keyword(runtime::to_string(name)).expect_ok();
    }

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
    return (o->behaviors & object_behavior::call) != object_behavior::none;
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
    return __rt_ctx->unique_symbol(to_string(o));
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
        using T = typename jtl::decay_t<decltype(typed_o)>::value_type;

        if constexpr(behavior::derefable<T>)
        {
          return typed_o->deref();
        }
        else
        {
          throw std::runtime_error{ util::format("not derefable: {}",
                                                 object_type_str(typed_o->type)) };
        }
      },
      o);
  }

  bool is_realized(object_ref const o)
  {
    return visit_object(
      [=](auto const typed_o) -> bool {
        using T = typename jtl::decay_t<decltype(typed_o)>::value_type;

        if constexpr(behavior::realizable<T>)
        {
          return typed_o->is_realized();
        }
        else
        {
          throw std::runtime_error{ util::format("not realizable: {}",
                                                 object_type_str(typed_o->type)) };
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

  object_ref re_pattern(object_ref const o)
  {
    return make_box<obj::re_pattern>(try_object<obj::persistent_string>(o)->data);
  }

  object_ref re_matcher(object_ref const re, object_ref const s)
  {
    return make_box<obj::re_matcher>(try_object<obj::re_pattern>(re),
                                     try_object<obj::persistent_string>(s)->data);
  }

  object_ref smatch_to_vector(std::smatch const &match_results)
  {
    auto const size(match_results.size());
    switch(size)
    {
      case 0:
        return jank_nil();
      case 1:
        {
          return make_box<obj::persistent_string>(match_results[0].str());
        }
      default:
        {
          native_vector<object_ref> vec;
          vec.reserve(size);

          for(auto const s : match_results)
          {
            vec.emplace_back(make_box<obj::persistent_string>(s.str()));
          }

          return make_box<obj::persistent_vector>(
            runtime::detail::native_persistent_vector{ vec.begin(), vec.end() });
        }
    }
  }

  object_ref re_find(object_ref const m)
  {
    std::smatch match_results{};
    auto const matcher(try_object<obj::re_matcher>(m));
    std::regex_search(matcher->match_input, match_results, matcher->re->regex);

    // Copy out the match result substrings before mutating the source
    // match_input string below.
    matcher->groups = smatch_to_vector(match_results);

    if(!match_results.empty())
    {
      matcher->match_input = match_results.suffix().str();
    }

    return matcher->groups;
  }

  object_ref re_groups(object_ref const m)
  {
    auto const matcher(try_object<obj::re_matcher>(m));

    if(matcher->groups.is_nil())
    {
      throw std::runtime_error{ "No match found" };
    }

    return matcher->groups;
  }

  object_ref re_matches(object_ref const re, object_ref const s)
  {
    std::smatch match_results{};
    std::string const search_str{ try_object<obj::persistent_string>(s)->data.c_str() };

    std::regex_search(search_str,
                      match_results,
                      try_object<obj::re_pattern>(re)->regex,
                      std::regex_constants::match_continuous);

    if(!match_results.suffix().str().empty())
    {
      return jank_nil();
    }

    return smatch_to_vector(match_results);
  }

  object_ref parse_uuid(object_ref const o)
  {
    if(o->type == object_type::persistent_string)
    {
      try
      {
        return make_box<obj::uuid>(expect_object<obj::persistent_string>(o)->data);
      }
      catch(...)
      {
        return jank_nil();
      }
    }
    else
    {
      throw std::runtime_error{ util::format("expected string, got {}", object_type_str(o->type)) };
    }
  }

  bool is_uuid(object_ref const o)
  {
    return o->type == object_type::uuid;
  }

  object_ref random_uuid()
  {
    return make_box<obj::uuid>();
  }

  bool is_inst(object_ref const o)
  {
    return o->type == object_type::inst;
  }

  i64 inst_ms(object_ref const o)
  {
    if(o->type != object_type::inst)
    {
      throw std::runtime_error{ util::format("The function 'inst-ms' expects an inst, got {}",
                                             object_type_str(o->type)) };
    }

    return std::chrono::duration_cast<std::chrono::milliseconds>(
             expect_object<obj::inst>(o)->value.time_since_epoch())
      .count();
  }

  object_ref add_watch(object_ref const reference, object_ref const key, object_ref const fn)
  {
    visit_object(
      [=](auto const typed_reference) -> void {
        using T = typename jtl::decay_t<decltype(typed_reference)>::value_type;

        if constexpr(behavior::ref_like<T>)
        {
          typed_reference->add_watch(key, fn);
        }
        else
        {
          throw std::runtime_error{ util::format(
            "Value does not support 'add-watch' because it is not ref_like: {}",
            typed_reference->to_code_string()) };
        }
      },
      reference);

    return reference;
  }

  object_ref remove_watch(object_ref const reference, object_ref const key)
  {
    visit_object(
      [=](auto const typed_reference) -> void {
        using T = typename jtl::decay_t<decltype(typed_reference)>::value_type;

        if constexpr(behavior::ref_like<T>)
        {
          typed_reference->remove_watch(key);
        }
        else
        {
          throw std::runtime_error{ util::format(
            "Value does not support 'remove-watch' because it is not ref_like: {}",
            typed_reference->to_code_string()) };
        }
      },
      reference);

    return reference;
  }

  object_ref future(object_ref const fn)
  {
    auto const bindings{ __rt_ctx->thread_binding_frames };
    auto const ret{ make_box<obj::future>() };
    ret->thread = std::thread{ [=]() {
      __rt_ctx->thread_binding_frames = bindings;

      try
      {
        auto const res{ dynamic_call(fn) };
        {
          auto const locked_state{ ret->state.wlock() };
          locked_state->status = obj::future_status::done;
          locked_state->result = res;
        }
      }
      catch(object_ref const o)
      {
        auto const locked_state{ ret->state.wlock() };
        locked_state->status = obj::future_status::done;
        locked_state->error = o;
      }
      catch(std::exception const &e)
      {
        auto const locked_state{ ret->state.wlock() };
        locked_state->status = obj::future_status::done;
        locked_state->error = make_box(e.what());
      }
      /* When we cancel, pthread will implicitly throw this force unwind. We want to intercept
       * that so we can mark our thread as cancelled. We then rethrow, since pthread is excepting
       * this to unwind all the way. */
#ifdef JANK_LINUX_LIKE
      catch(abi::__forced_unwind const &fu)
      {
        auto const locked_state{ ret->state.wlock() };
        locked_state->status = obj::future_status::cancelled;
        locked_state->error = make_box("Thread was cancelled.");
        throw;
      }
#endif
      /* In this case, we don't know what was thrown, but at least we can preserve
       * the fact that *something* was thrown. */
      catch(...)
      {
        auto const locked_state{ ret->state.wlock() };
        locked_state->status = obj::future_status::done;
        locked_state->error = make_box("Unknown exception.");
        throw;
      }
    } };
    return ret;
  }

  void cancel_future(object_ref const future)
  {
    auto const fut{ try_object<obj::future>(future) };

    /* We need to hold this lock the whole time we're checking, to ensure the thread
     * doesn't finish while we're here checking. */
    auto const locked_state{ fut->state.rlock() };
    if(locked_state->status == obj::future_status::running)
    {
      auto const locked_thread{ fut->thread.wlock() };
      auto const thread_handle{ locked_thread->native_handle() };
      pthread_cancel(thread_handle);
    }
  }

  bool is_future_cancelled(object_ref const future)
  {
    auto const fut{ try_object<obj::future>(future) };

    /* We need to hold this lock the whole time we're checking, to ensure the thread
     * doesn't finish while we're here checking. */
    auto locked_state{ fut->state.ulock() };
    switch(locked_state->status)
    {
      case obj::future_status::done:
        return false;
      case obj::future_status::cancelled:
        return true;
      case obj::future_status::running:
        break;
    }

#ifdef JANK_MACOS_LIKE
    /* macOS doesn't have pthread_tryjoin_np, or any similar function, so we can only
     * pthread_join, to get the cancellation state, which is blocking. So we just have
     * to return false here. That means it's not currently possible to know if a thread
     * was cancelled on macOS. */
    return false;
#else
    void *thread_state{};
    int code{};

    /* It's undefined behavior to have multiple threads join a single thread object at the
     * same time, so we need to synchronize here. */
    {
      auto const locked_thread{ fut->thread.wlock() };
      auto const thread_handle{ locked_thread->native_handle() };
      code = pthread_tryjoin_np(thread_handle, &thread_state);
    }

    switch(code)
    {
      /* We'll get this if two threads are joining each other. Not cancelled. */
      case EDEADLK:
      /* We'll get this if the thread is not joinable. Not cancelled. */
      case EINVAL:
      /* We'll get this if no matching thread is found. Not cancelled. */
      case ESRCH:
      /* We'll get this if the thread has not yet been terminated. Not cancelled. */
      case EBUSY:
        return false;
      /* No error. */
      default:
        break;
    }

    /* Our join succeeded, but we can only join once, so we need to save the result here
     * so we can short circuit next time. */
    auto const write_locked_state{ locked_state.moveFromUpgradeToWrite() };
    if(thread_state == PTHREAD_CANCELED)
    {
      write_locked_state->status = obj::future_status::cancelled;
      return true;
    }
    else
    {
      write_locked_state->status = obj::future_status::done;
      return false;
    }
#endif
  }
}
