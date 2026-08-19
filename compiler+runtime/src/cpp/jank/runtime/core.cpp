#include <algorithm>
#include <iterator>
#include <deque>
#include <pthread.h>
#include <cxxabi.h>
#include <charconv>

#include <cpptrace/basic.hpp>

#include <jank/gc.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/behavior/nameable.hpp>
#include <jank/runtime/behavior/ref_like.hpp>
#include <jank/runtime/behavior/realizable.hpp>
#include <jank/runtime/core/call.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/sequence_range.hpp>
#include <jank/runtime/detail/std_format.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/scope_exit.hpp>

namespace jank::runtime
{
  jtl::immutable_string type(object_ref const o)
  {
    return object_type_str(o.get_type());
  }

  bool is_nil(object_ref const o)
  {
    return o.is_nil();
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
    return o.is_some();
  }

  bool is_string(object_ref const o)
  {
    return o.get_type() == object_type::persistent_string;
  }

  bool is_char(object_ref const o)
  {
    return o.get_type() == object_type::character;
  }

  bool is_symbol(object_ref const o)
  {
    return o.get_type() == object_type::symbol;
  }

  bool is_simple_symbol(object_ref const o)
  {
    return o.get_type() == object_type::symbol && expect_object<obj::symbol>(o)->ns.empty();
  }

  bool is_qualified_symbol(object_ref const o)
  {
    return o.get_type() == object_type::symbol && !expect_object<obj::symbol>(o)->ns.empty();
  }

  obj::symbol_ref to_unqualified_symbol(object_ref const o)
  {
    /* TODO: Port visit_object: Not all types. */
    return runtime::visit_object(
      [&](auto const typed_o) -> obj::symbol_ref {
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
          throw std::runtime_error{ util::format(
                                      "Objects of type `{}` cannot be converted to symbols.",
                                      object_type_str(typed_o.get_type()))
                                      .c_str() };
        }
      },
      o);
  }

  obj::symbol_ref to_qualified_symbol(object_ref const ns, object_ref const name)
  {
    return make_box<obj::symbol>(ns, name);
  }

  static FILE *get_stdout()
  {
    static auto const stream_val{ __rt_ctx->stream_var->deref() };
    static auto const stream_box{ try_object<obj::opaque_box>(stream_val) };

    auto const out_val{ __rt_ctx->current_out_var->deref() };
    auto const out_box{ try_object<obj::opaque_box>(out_val) };

    if(stream_box->canonical_type != out_box->canonical_type)
    {
      throw std::runtime_error{ util::format(
        "The binding for `*out*` must match the current stream type. The current binding is `{}`.",
        out_box->canonical_type) };
    }

    return reinterpret_cast<FILE *>(out_box->data.data);
  }

  object_ref print(object_ref const args)
  {
    if(args.is_nil())
    {
      return {};
    }

    auto const out{ get_stdout() };

    jtl::string_builder buff;
    args.first().to_string(buff);
    for(auto const e : make_sequence_range(args).skip(1))
    {
      buff(' ');
      e.to_string(buff);
    }
    std::fwrite(buff.data(), 1, buff.size(), out);
    return {};
  }

  object_ref print1(object_ref const o)
  {
    jtl::string_builder buff;
    o.to_string(buff);
    std::fwrite(buff.data(), 1, buff.size(), get_stdout());
    return {};
  }

  object_ref println(object_ref const args)
  {
    auto const out{ get_stdout() };

    if(args.is_nil())
    {
      std::putc('\n', out);
    }
    else
    {
      jtl::string_builder buff;
      args.first().to_string(buff);
      for(auto const e : make_sequence_range(args).skip(1))
      {
        buff(' ');
        e.to_string(buff);
      }
      std::fwrite(buff.data(), 1, buff.size(), out);
      std::putc('\n', out);
    }
    return {};
  }

  object_ref pr(object_ref const args)
  {
    if(args.is_nil())
    {
      return {};
    }

    auto const out{ get_stdout() };

    jtl::string_builder buff;
    buff(args.first().to_code_string());
    for(auto const e : make_sequence_range(args).skip(1))
    {
      buff(' ');
      buff(e.to_code_string());
    }
    std::fwrite(buff.data(), 1, buff.size(), out);

    return {};
  }

  object_ref prn(object_ref const args)
  {
    auto const out{ get_stdout() };

    if(args.is_nil())
    {
      std::putc('\n', out);
    }
    else
    {
      jtl::string_builder buff;
      buff(args.first().to_code_string());
      for(auto const e : make_sequence_range(args).skip(1))
      {
        buff(' ');
        buff(e.to_code_string());
      }
      std::fwrite(buff.data(), 1, buff.size(), out);
      std::putc('\n', out);
    }

    return {};
  }

  jtl::immutable_string format(jtl::immutable_string const &format, object_ref const args)
  {
    enum class indexing_mode : u8
    {
      unspecified,
      /* Argument indices are automatically incremented as they are encountered,
       * specifiers are bare e.g. {} or {:}. */
      automatic,
      /* User specifies the argument index, e.g. {2}. */
      manual
    };

    auto const args_vec{ vec(args) };

    jtl::string_builder out;
    jtl::string_builder fmt;
    i64 depth{};

    indexing_mode mode{ indexing_mode::unspecified };
    std::deque<u64> arg_indices{};
    u64 auto_idx{};

    for(auto it{ format.begin() }; it != format.end(); ++it)
    {
      auto const peek{ std::next(it) != format.end() ? *std::next(it) : '\0' };

      /* Top-level {{ or }} escape sequences. */
      if(depth == 0 && ((*it == '{' && peek == '{') || (*it == '}' && peek == '}')))
      {
        out(*it);
        ++it;
      }
      else if(*it == '{')
      {
        fmt(*it);
        ++depth;

        if(std::isdigit(peek))
        {
          /* Manual indexing. */
          switch(mode)
          {
            case indexing_mode::unspecified:
              mode = indexing_mode::manual;
              break;
            case indexing_mode::automatic:
              throw std::format_error{
                "Automatic and manual format indexes cannot be mixed in the same format string."
              };
            case indexing_mode::manual:
              break;
          }

          auto start{ std::next(it) };
          auto end{ std::find_if(start, format.end(), [](auto c) { return !std::isdigit(c); }) };

          u64 idx{};
          std::from_chars(start, end, idx);
          arg_indices.push_back(idx);

          /* Don't add the index to the format string passed to std::format, we
           * will deal with indexing ourselves. */
          it = std::prev(end);
        }
        else
        {
          /* Automatic indexing. */
          switch(mode)
          {
            case indexing_mode::unspecified:
              mode = indexing_mode::automatic;
              break;
            case indexing_mode::automatic:
              break;
            case indexing_mode::manual:
              throw std::format_error{
                "Automatic and manual format indexes cannot be mixed in the same format string."
              };
          }

          arg_indices.push_back(auto_idx++);
        }
      }
      else if(*it == '}')
      {
        fmt(*it);
        --depth;

        /* End of a top-level format specification, process it in-situ. */
        if(depth == 0)
        {
          auto nargs{ arg_indices.size() };
          auto next_arg{ [&]() {
            auto idx{ arg_indices.front() };
            arg_indices.pop_front();

            if(idx >= args_vec->count())
            {
              throw std::format_error{
                util::format("The format argument index `{}` is out of range.", idx).c_str()
              };
            }
            return args_vec->nth(make_box(idx));
          } };

          /* Depending on the number of embedded replacement fields we
           * encountered, pop the right number of values off the argument stack. */
          auto v1{ next_arg() };

          /* Width and precision are the only supported nested field
           * replacements in std::format as of C++20, so we only need to support
           * up to 1 value + 2 nested arguments (always integral). */
          if(nargs == 1)
          {
            detail::vformat_object_to(std::back_inserter(out), fmt.view(), v1);
          }
          else if(nargs == 2)
          {
            auto v2{ next_arg().to_integer() };
            detail::vformat_object_to(std::back_inserter(out), fmt.view(), v1, v2);
          }
          else if(nargs == 3)
          {
            auto v2{ next_arg().to_integer() };
            auto v3{ next_arg().to_integer() };
            detail::vformat_object_to(std::back_inserter(out), fmt.view(), v1, v2, v3);
          }
          else
          {
            throw std::format_error{ "This format string contains too many replacement fields." };
          }

          /* Reset for the next format specification. */
          fmt.clear();
          /* Ignore extra args. */
          arg_indices.clear();
        }
      }
      else
      {
        if(depth > 0)
        {
          /* We're inside a replacement field. */
          fmt(*it);
        }
        else
        {
          /* This is an ordinary character. */
          out(*it);
        }
      }
    }

    if(depth != 0)
    {
      throw std::format_error{ "This format string has an unmatched brace." };
    }

    return out.release();
  }

  obj::persistent_string_ref subs(object_ref const s, object_ref const start)
  {
    auto const str{ dyn_cast<obj::persistent_string>(s) };
    if(str.is_nil())
    {
      throw std::runtime_error{ util::format(
        "The first argument to `subs` must be a `persistent_string`, not a `{}`.",
        object_type_str(s.get_type())) };
    }

    return str->substring(to_int(start)).expect_ok();
  }

  obj::persistent_string_ref subs(object_ref const s, object_ref const start, object_ref const end)
  {
    auto const str{ dyn_cast<obj::persistent_string>(s) };
    if(str.is_nil())
    {
      throw std::runtime_error{ util::format(
        "The first argument to `subs` must be a `persistent_string`, not a `{}`.",
        object_type_str(s.get_type())) };
    }

    return str->substring(to_int(start), to_int(end)).expect_ok();
  }

  i64 first_index_of(object_ref const s, object_ref const m)
  {
    auto const str{ dyn_cast<obj::persistent_string>(s) };
    if(str.is_nil())
    {
      throw std::runtime_error{ util::format(
        "The first argument to `first-index-of` must be a `persistent_string`, not a `{}`.",
        object_type_str(s.get_type())) };
    }
    return str->first_index_of(m);
  }

  i64 last_index_of(object_ref const s, object_ref const m)
  {
    auto const str{ dyn_cast<obj::persistent_string>(s) };
    if(str.is_nil())
    {
      throw std::runtime_error{ util::format(
        "The first argument to `last-index-of` must be a `persistent_string`, not a `{}`.",
        object_type_str(s.get_type())) };
    }
    return str->last_index_of(m);
  }

  bool is_named(object_ref const o)
  {
    /* TODO: Port visit_object: nameable. */
    return visit_object(
      [](auto const typed_o) {
        using T = typename jtl::decay_t<decltype(typed_o)>::value_type;

        return behavior::nameable<T>;
      },
      o);
  }

  jtl::immutable_string name(object_ref const o)
  {
    /* TODO: Port visit_object: nameable. */
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
          throw std::runtime_error{ util::format("Objects of type `{}` are not nameable.",
                                                 object_type_str(typed_o.get_type())) };
        }
      },
      o);
  }

  object_ref namespace_(object_ref const o)
  {
    /* TODO: Port visit_object: nameable. */
    return visit_object(
      [](auto const typed_o) -> object_ref {
        using T = typename jtl::decay_t<decltype(typed_o)>::value_type;

        if constexpr(behavior::nameable<T>)
        {
          auto const ns(typed_o->get_namespace());
          if(ns.empty())
          {
            return {};
          }
          return make_box<obj::persistent_string>(ns);
        }
        else
        {
          throw std::runtime_error{ util::format("Objects of type `{}` are not nameable.",
                                                 object_type_str(typed_o.get_type())) };
        }
      },
      o);
  }

  obj::native_vector_sequence_ref all_ns()
  {
    auto const all{ __rt_ctx->all_ns() };
    native_vector<object_ref> v;
    v.reserve(all.size());
    for(auto const n : all)
    {
      v.emplace_back(n.erase());
    }
    return make_box<obj::native_vector_sequence>(jtl::move(v));
  }

  obj::keyword_ref keyword(object_ref const ns, object_ref const name)
  {
    if(!ns.is_nil() && ns.get_type() != object_type::persistent_string)
    {
      throw std::runtime_error{ util::format("The `keyword` function expects the namespace to be "
                                             "`nil` or a `persistent_string`, not a `{}`.",
                                             object_type_str(ns.get_type())) };
    }
    if(name.get_type() != object_type::persistent_string)
    {
      throw std::runtime_error{ util::format(
        "The `keyword` function expects the name to be a `persistent_string`, not a `{}`.",
        object_type_str(name.get_type())) };
    }
    if(ns.is_nil())
    {
      return __rt_ctx->intern_keyword(name.to_string()).expect_ok();
    }

    return __rt_ctx->intern_keyword(ns.to_string(), name.to_string()).expect_ok();
  }

  bool is_keyword(object_ref const o)
  {
    return o.get_type() == object_type::keyword;
  }

  bool is_simple_keyword(object_ref const o)
  {
    return o.get_type() == object_type::keyword && expect_object<obj::keyword>(o)->sym->ns.empty();
  }

  bool is_qualified_keyword(object_ref const o)
  {
    return o.get_type() == object_type::keyword && !expect_object<obj::keyword>(o)->sym->ns.empty();
  }

  bool is_callable(object_ref const o)
  {
    return o.has_behavior(object_behavior::call);
  }

  uhash to_hash(object_ref const o)
  {
    return o.to_hash();
  }

  object_ref macroexpand1(object_ref const o)
  {
    return __rt_ctx->macroexpand1(o);
  }

  object_ref macroexpand(object_ref const o)
  {
    return __rt_ctx->macroexpand(o);
  }

  obj::symbol_ref gensym(object_ref const o)
  {
    return __rt_ctx->unique_symbol(o.to_string());
  }

  obj::atom_ref atom(object_ref const o)
  {
    return make_box<obj::atom>(o);
  }

  object_ref swap_atom(obj::atom_ref const atom, object_ref const fn)
  {
    return atom->swap(fn);
  }

  object_ref swap_atom(obj::atom_ref const atom, object_ref const fn, object_ref const a1)
  {
    return atom->swap(fn, a1);
  }

  object_ref
  swap_atom(obj::atom_ref const atom, object_ref const fn, object_ref const a1, object_ref const a2)
  {
    return atom->swap(fn, a1, a2);
  }

  object_ref swap_atom(obj::atom_ref const atom,
                       object_ref const fn,
                       object_ref const a1,
                       object_ref const a2,
                       object_ref const rest)
  {
    return atom->swap(fn, a1, a2, rest);
  }

  object_ref swap_vals(obj::atom_ref const atom, object_ref const fn)
  {
    return atom->swap_vals(fn);
  }

  object_ref swap_vals(obj::atom_ref const atom, object_ref const fn, object_ref const a1)
  {
    return atom->swap_vals(fn, a1);
  }

  object_ref
  swap_vals(obj::atom_ref const atom, object_ref const fn, object_ref const a1, object_ref const a2)
  {
    return atom->swap_vals(fn, a1, a2);
  }

  object_ref swap_vals(obj::atom_ref const atom,
                       object_ref const fn,
                       object_ref const a1,
                       object_ref const a2,
                       object_ref const rest)
  {
    return atom->swap_vals(fn, a1, a2, rest);
  }

  object_ref
  compare_and_set(obj::atom_ref const atom, object_ref const old_val, object_ref const new_val)
  {
    return atom->compare_and_set(old_val, new_val);
  }

  object_ref reset(obj::atom_ref const atom, object_ref const new_val)
  {
    return atom->reset(new_val);
  }

  object_ref reset_vals(obj::atom_ref const atom, object_ref const new_val)
  {
    return atom->reset_vals(new_val);
  }

  object_ref deref(object_ref const o)
  {
    return o.deref();
  }

  bool is_realized(object_ref const o)
  {
    /* TODO: Port visit_object: realizable. */
    return visit_object(
      [=](auto const typed_o) -> bool {
        using T = typename jtl::decay_t<decltype(typed_o)>::value_type;

        if constexpr(behavior::realizable<T>)
        {
          return typed_o->is_realized();
        }
        else
        {
          throw std::runtime_error{ util::format("Objects of type `{}` are not realizable.",
                                                 object_type_str(typed_o.get_type())) };
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
    return o.get_type() == object_type::volatile_;
  }

  object_ref vswap(obj::volatile_ref const v, object_ref const fn)
  {
    return v->reset(fn.call(v->deref()));
  }

  object_ref vswap(obj::volatile_ref const v, object_ref const fn, object_ref const args)
  {
    return v->reset(apply_to(fn, make_box<obj::cons>(v->deref(), args)));
  }

  object_ref vreset(obj::volatile_ref const v, object_ref const new_val)
  {
    return v->reset(new_val);
  }

  void push_thread_bindings(object_ref const o)
  {
    __rt_ctx->push_thread_bindings(o).expect_ok();
  }

  void pop_thread_bindings()
  {
    __rt_ctx->pop_thread_bindings();
  }

  object_ref get_thread_bindings()
  {
    return __rt_ctx->get_thread_bindings();
  }

  object_ref force(object_ref const o)
  {
    if(o.get_type() == object_type::delay)
    {
      return expect_object<obj::delay>(o)->deref();
    }
    return o;
  }

  obj::tagged_literal_ref tagged_literal(object_ref const tag, object_ref const form)
  {
    return make_box<obj::tagged_literal>(tag, form);
  }

  bool is_tagged_literal(object_ref const o)
  {
    return o.get_type() == object_type::tagged_literal;
  }

  obj::re_pattern_ref re_pattern(object_ref const o)
  {
    if(o.get_type() == object_type::re_pattern)
    {
      return expect_object<obj::re_pattern>(o);
    }

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
        return {};
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
            runtime::detail::native_persistent_vector(vec.begin(), vec.end()));
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
      throw std::runtime_error{ "No match was found for this regular expression." };
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
      return {};
    }

    return smatch_to_vector(match_results);
  }

  object_ref parse_uuid(object_ref const o)
  {
    if(o.get_type() == object_type::persistent_string)
    {
      try
      {
        return make_box<obj::uuid>(expect_object<obj::persistent_string>(o)->data);
      }
      catch(...)
      {
        return {};
      }
    }
    else
    {
      throw std::runtime_error{ util::format(
        "The `parse-uuid` function expects a `persistent_string`, not a `{}`.",
        object_type_str(o.get_type())) };
    }
  }

  bool is_uuid(object_ref const o)
  {
    return o.get_type() == object_type::uuid;
  }

  obj::uuid_ref random_uuid()
  {
    return make_box<obj::uuid>();
  }

  bool is_inst(object_ref const o)
  {
    return o.get_type() == object_type::inst;
  }

  i64 inst_ms(object_ref const o)
  {
    if(o.get_type() != object_type::inst)
    {
      throw std::runtime_error{ util::format(
        "The `inst-ms` function expects an `inst`, not a `{}`.",
        object_type_str(o.get_type())) };
    }

    return std::chrono::duration_cast<std::chrono::milliseconds>(
             expect_object<obj::inst>(o)->value.time_since_epoch())
      .count();
  }

  void set_validator(object_ref reference, object_ref const validator_fn)
  {
    reference.set_validator(validator_fn);
  }

  object_ref get_validator(object_ref const reference)
  {
    return reference.get_validator();
  }

  object_ref add_watch(object_ref reference, object_ref const key, object_ref const fn)
  {
    reference.add_watch(key, fn);
    return reference;
  }

  object_ref remove_watch(object_ref reference, object_ref const key)
  {
    reference.remove_watch(key);
    return reference;
  }

  obj::future_ref future(object_ref const fn)
  {
    auto const bindings{ __rt_ctx->get_thread_bindings() };
    auto const ret{ make_box<obj::future>() };
    /* NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage): False positive. */
    ret->thread = std::thread{ [=]() {
      /* GC threads should be explicitly registered so that the GC is prepared to perform
       * allocations from this thread. Unregistering is equally important.
       *
       * We don't do this on macOS, since experimentation has found that BDWGC does it
       * for us. */
      if constexpr(jtl::current_platform != jtl::platform::macos_like)
      {
        GC_stack_base sb{};
        GC_get_stack_base(&sb);
        GC_register_my_thread(&sb);
      }
      util::scope_exit const unregister{ []() {
        if constexpr(jtl::current_platform != jtl::platform::macos_like)
        {
          GC_unregister_my_thread();
        }
      } };

      __rt_ctx->push_thread_bindings(bindings).expect_ok();

      try
      {
        auto const res{ fn.call() };
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

  void cancel_future(obj::future_ref const future)
  {
    /* We need to hold this lock the whole time we're checking, to ensure the thread
     * doesn't finish while we're here checking. */
    auto const locked_state{ future->state.rlock() };
    if(locked_state->status == obj::future_status::running)
    {
      auto const locked_thread{ future->thread.wlock() };
      auto const thread_handle{ locked_thread->native_handle() };
      pthread_cancel(reinterpret_cast<pthread_t>(thread_handle));
    }
  }

  bool is_future_cancelled(obj::future_ref const future)
  {
    /* We need to hold this lock the whole time we're checking, to ensure the thread
     * doesn't finish while we're here checking. */
    auto locked_state{ future->state.ulock() };
    switch(locked_state->status)
    {
      case obj::future_status::done:
        return false;
      case obj::future_status::cancelled:
        return true;
      case obj::future_status::running:
        break;
    }

#if defined(JANK_MACOS_LIKE) || defined(JANK_WINDOWS_LIKE)
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
      auto const locked_thread{ future->thread.wlock() };
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

  obj::promise_ref promise()
  {
    return make_box<obj::promise>();
  }

  object_ref read_string(object_ref const form_string, object_ref const opts)
  {
    if(form_string.get_type() != object_type::persistent_string)
    {
      throw std::runtime_error{ util::format(
        "The `read-string` function expects a string representing a form, not a `{}`.",
        object_type_str(form_string.get_type())) };
    }

    auto const typed_o{ expect_object<obj::persistent_string>(form_string) };
    return __rt_ctx->read_string(typed_o->data, opts);
  }

  object_ref read_file(object_ref const file_path, object_ref const opts)
  {
    if(file_path.get_type() != object_type::persistent_string)
    {
      throw std::runtime_error{ util::format(
        "The `read-file` function expects a string representing a file path, not a `{}`.",
        object_type_str(file_path.get_type())) };
    }

    auto const typed_o{ expect_object<obj::persistent_string>(file_path) };
    return __rt_ctx->read_file(typed_o->data, opts);
  }

  obj::character_ref to_char(object_ref const x)
  {
    if(x.get_type() == object_type::character)
    {
      return expect_object<obj::character>(x);
    }

    return make_box<obj::character>(to_int(x));
  }

  obj::exception_info_ref ex_info(jtl::immutable_string const &message, object_ref const data)
  {
    return ex_info(message, data, {});
  }

  obj::exception_info_ref
  ex_info(jtl::immutable_string const &message, object_ref const data, object_ref const cause)
  {
    auto const ret{ make_box<obj::exception_info>(message, data) };
    ret->raw_trace = std::make_unique<cpptrace::raw_trace>(cpptrace::generate_raw_trace());
    if(cause.is_some())
    {
      ret->cause = try_object<obj::exception_info>(cause);
    }
    return ret;
  }

  jtl::immutable_string ex_message(object_ref const o)
  {
    return try_object<obj::exception_info>(o)->message;
  }

  object_ref ex_data(object_ref const o)
  {
    return try_object<obj::exception_info>(o)->data;
  }

  obj::exception_info_ref ex_cause(object_ref const o)
  {
    return try_object<obj::exception_info>(o)->cause;
  }

  obj::persistent_array_map_ref ex_map(object_ref const o)
  {
    return try_object<obj::exception_info>(o)->to_map();
  }
}
