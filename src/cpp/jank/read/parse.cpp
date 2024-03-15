#include <iostream>
#include <iomanip>

#include <magic_enum.hpp>

#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/persistent_list.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/runtime/obj/persistent_set.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/read/parse.hpp>
#include <jank/util/escape.hpp>

namespace jank::read::parse
{
  native_bool
  processor::object_source_info::operator==(processor::object_source_info const &rhs) const
  {
    return !(*this != rhs);
  }

  native_bool
  processor::object_source_info::operator!=(processor::object_source_info const &rhs) const
  {
    return ptr != rhs.ptr || start != rhs.start || end != rhs.end;
  }

  processor::iterator::value_type processor::iterator::operator*() const
  {
    return latest.unwrap();
  }

  processor::iterator::pointer processor::iterator::operator->()
  {
    return &latest.unwrap();
  }

  processor::iterator &processor::iterator::operator++()
  {
    latest = some(p.next());
    return *this;
  }

  native_bool processor::iterator::operator!=(processor::iterator const &rhs) const
  {
    return latest != rhs.latest;
  }

  native_bool processor::iterator::operator==(processor::iterator const &rhs) const
  {
    return latest == rhs.latest;
  }

  processor::iterator &processor::iterator::operator=(processor::iterator const &rhs)
  {
    latest = rhs.latest;
    return *this;
  }

  processor::processor(runtime::context &rt_ctx,
                       lex::processor::iterator const &b,
                       lex::processor::iterator const &e)
    : rt_ctx{ rt_ctx }
    , token_current{ b }
    , token_end{ e }
    , splicing_allowed_var{ make_box<runtime::var>(
                              rt_ctx.intern_ns(make_box<runtime::obj::symbol>("clojure.core")),
                              make_box<runtime::obj::symbol>("*splicing-allowed?*"),
                              runtime::obj::boolean::false_const())
                              ->set_dynamic(true) }
  {
  }

  processor::object_result processor::next()
  {
    if(token_current == token_end)
    {
      return ok(none);
    }

    while(true)
    {
      if(!pending_forms.empty())
      {
        auto const front(pending_forms.front());
        pending_forms.pop_front();
        /* NOTE: Source info for spliced forms will be incorrect. Right now,
         * that doesn't matter. If it begins to matter, we need to store the
         * source info along with the object in `pending_forms`. */
        return object_source_info{ front, latest_token, latest_token };
      }

      auto token_result(*token_current);
      if(token_result.is_err())
      {
        return token_result.err().unwrap();
      }
      latest_token = token_result.expect_ok();
      switch(latest_token.kind)
      {
        /* We ignore comments, but everything else returns out of the loop. */
        case lex::token_kind::comment:
          ++token_current;
          continue;

        case lex::token_kind::open_paren:
          return parse_list();
        case lex::token_kind::open_square_bracket:
          return parse_vector();
        case lex::token_kind::open_curly_bracket:
          return parse_map();
        case lex::token_kind::close_square_bracket:
        case lex::token_kind::close_paren:
        case lex::token_kind::close_curly_bracket:
          if(expected_closer != latest_token.kind)
          {
            return err(error{ latest_token.pos,
                              native_persistent_string{ "unexpected closing character" } });
          }
          ++token_current;
          expected_closer = none;
          return ok(none);
        case lex::token_kind::single_quote:
          return parse_quote();
        case lex::token_kind::meta_hint:
          return parse_meta_hint();
        case lex::token_kind::reader_macro:
          {
            auto res(parse_reader_macro());
            /* Reader macros can skip some items entirely. */
            if(res.is_ok() && res.expect_ok().is_none())
            {
              continue;
            }
            return res;
          }
        case lex::token_kind::reader_macro_comment:
          {
            auto res(parse_reader_macro_comment());
            if(res.is_ok() && res.expect_ok().is_none())
            {
              continue;
            }
            return res;
          }
        case lex::token_kind::reader_macro_conditional:
          {
            auto res(parse_reader_macro_conditional(false));
            if(res.is_ok() && res.expect_ok().is_none())
            {
              continue;
            }
            return res;
          }
        case lex::token_kind::reader_macro_conditional_splice:
          {
            auto res(parse_reader_macro_conditional(true));
            if(res.is_ok() && res.expect_ok().is_none())
            {
              continue;
            }
            return res;
          }
        case lex::token_kind::nil:
          return parse_nil();
        case lex::token_kind::boolean:
          return parse_boolean();
        case lex::token_kind::symbol:
          return parse_symbol();
        case lex::token_kind::keyword:
          return parse_keyword();
        case lex::token_kind::integer:
          return parse_integer();
        case lex::token_kind::real:
          return parse_real();
        case lex::token_kind::string:
          return parse_string();
        case lex::token_kind::escaped_string:
          return parse_escaped_string();
        case lex::token_kind::eof:
          return ok(none);
        default:
          {
            native_persistent_string msg{ fmt::format("unexpected token kind: {}",
                                                      magic_enum::enum_name(latest_token.kind)) };
            return err(error{ latest_token.pos, std::move(msg) });
          }
      }
    }
  }

  processor::object_result processor::parse_list()
  {
    auto const start_token((*token_current).expect_ok());
    ++token_current;
    auto const prev_expected_closer(expected_closer);
    expected_closer = some(lex::token_kind::close_paren);

    rt_ctx
      .push_thread_bindings(runtime::obj::persistent_hash_map::create_unique(
        std::make_pair(splicing_allowed_var, runtime::obj::boolean::true_const())))
      .expect_ok();
    util::scope_exit const finally{ [&]() { rt_ctx.pop_thread_bindings().expect_ok(); } };

    runtime::detail::native_transient_vector ret;
    for(auto it(begin()); it != end(); ++it)
    {
      if(it.latest.unwrap().is_err())
      {
        return err(it.latest.unwrap().expect_err());
      }
      ret.push_back(it.latest.unwrap().expect_ok().unwrap().ptr);
    }
    if(expected_closer.is_some())
    {
      return err(error{ start_token.pos, "Unterminated list" });
    }

    expected_closer = prev_expected_closer;
    return object_source_info{
      make_box<runtime::obj::persistent_list>(std::in_place, ret.rbegin(), ret.rend()),
      start_token,
      latest_token
    };
  }

  processor::object_result processor::parse_vector()
  {
    auto const start_token((*token_current).expect_ok());
    ++token_current;
    auto const prev_expected_closer(expected_closer);
    expected_closer = some(lex::token_kind::close_square_bracket);

    rt_ctx
      .push_thread_bindings(runtime::obj::persistent_hash_map::create_unique(
        std::make_pair(splicing_allowed_var, runtime::obj::boolean::true_const())))
      .expect_ok();
    util::scope_exit const finally{ [&]() { rt_ctx.pop_thread_bindings().expect_ok(); } };

    runtime::detail::native_transient_vector ret;
    for(auto it(begin()); it != end(); ++it)
    {
      if(it.latest.unwrap().is_err())
      {
        return err(it.latest.unwrap().expect_err());
      }
      ret.push_back(it.latest.unwrap().expect_ok().unwrap().ptr);
    }
    if(expected_closer.is_some())
    {
      return err(error{ start_token.pos, "Unterminated vector" });
    }

    expected_closer = prev_expected_closer;
    return object_source_info{ make_box<runtime::obj::persistent_vector>(ret.persistent()),
                               start_token,
                               latest_token };
  }

  /* TODO: Uniqueness check. */
  processor::object_result processor::parse_map()
  {
    auto const start_token((*token_current).expect_ok());
    ++token_current;
    auto const prev_expected_closer(expected_closer);
    expected_closer = some(lex::token_kind::close_curly_bracket);

    rt_ctx
      .push_thread_bindings(runtime::obj::persistent_hash_map::create_unique(
        std::make_pair(splicing_allowed_var, runtime::obj::boolean::true_const())))
      .expect_ok();
    util::scope_exit const finally{ [&]() { rt_ctx.pop_thread_bindings().expect_ok(); } };

    runtime::detail::native_persistent_array_map ret;
    for(auto it(begin()); it != end(); ++it)
    {
      if(it.latest.unwrap().is_err())
      {
        return err(it.latest.unwrap().expect_err());
      }
      auto const key(it.latest.unwrap().expect_ok());

      if(++it == end())
      {
        return err(error{ start_token.pos, "odd number of items in map" });
      }

      if(it.latest.unwrap().is_err())
      {
        return err(it.latest.unwrap().expect_err());
      }
      auto const value(it.latest.unwrap().expect_ok());

      ret.insert_or_assign(key.unwrap().ptr, value.unwrap().ptr);
    }
    if(expected_closer.is_some())
    {
      return err(error{ start_token.pos, "Unterminated map" });
    }

    expected_closer = prev_expected_closer;
    return object_source_info{ make_box<runtime::obj::persistent_array_map>(ret),
                               start_token,
                               latest_token };
  }

  processor::object_result processor::parse_quote()
  {
    auto const start_token((*token_current).expect_ok());
    ++token_current;
    auto const old_quoted(quoted);
    quoted = true;
    auto val_result(next());
    quoted = old_quoted;
    if(val_result.is_err())
    {
      return val_result;
    }
    else if(val_result.expect_ok().is_none())
    {
      return err(error{ start_token.pos, native_persistent_string{ "invalid value after quote" } });
    }

    return object_source_info{ runtime::erase(make_box<runtime::obj::persistent_list>(
                                 std::in_place,
                                 make_box<runtime::obj::symbol>("quote"),
                                 val_result.expect_ok().unwrap().ptr)),
                               start_token,
                               latest_token };
  }

  processor::object_result processor::parse_meta_hint()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;
    auto meta_val_result(next());
    if(meta_val_result.is_err())
    {
      return meta_val_result;
    }
    else if(meta_val_result.expect_ok().is_none())
    {
      return err(
        error{ start_token.pos, native_persistent_string{ "invalid meta value after meta hint" } });
    }

    auto meta_result(runtime::visit_object(
      [&](auto const typed_val) -> processor::object_result {
        using T = typename decltype(typed_val)::value_type;
        if constexpr(std::same_as<T, runtime::obj::keyword>)
        {
          return object_source_info{ runtime::obj::persistent_array_map::create_unique(
                                       typed_val,
                                       runtime::obj::boolean::true_const()),
                                     start_token,
                                     latest_token };
        }
        /* TODO: Concept for map-like. */
        if constexpr(std::same_as<T, runtime::obj::persistent_hash_map>
                     || std::same_as<T, runtime::obj::persistent_array_map>)
        {
          return object_source_info{ typed_val, start_token, latest_token };
        }
        else
        {
          return err(error{
            start_token.pos,
            native_persistent_string{ "value after meta hint ^ must be a keyword or map" } });
        }
      },
      meta_val_result.expect_ok().unwrap().ptr));
    if(meta_result.is_err())
    {
      return meta_result;
    }

    auto target_val_result(next());
    if(target_val_result.is_err())
    {
      return target_val_result;
    }
    else if(target_val_result.expect_ok().is_none())
    {
      return err(error{ start_token.pos,
                        native_persistent_string{ "invalid target value after meta hint" } });
    }

    return runtime::visit_object(
      [&](auto const typed_val) -> processor::object_result {
        using T = typename decltype(typed_val)::value_type;
        if constexpr(runtime::behavior::metadatable<T>)
        {
          if(typed_val->meta.is_none())
          {
            return object_source_info{ typed_val->with_meta(meta_result.expect_ok().unwrap().ptr),
                                       start_token,
                                       latest_token };
          }
          else
          {
            return object_source_info{ typed_val->with_meta(
                                         runtime::merge(typed_val->meta.unwrap(),
                                                        meta_result.expect_ok().unwrap().ptr)),
                                       start_token,
                                       latest_token };
          }
        }
        else
        {
          return err(
            error{ start_token.pos,
                   native_persistent_string{ "target value for meta hint must accept metadata" } });
        }
      },
      target_val_result.expect_ok().unwrap().ptr);
  }

  processor::object_result processor::parse_reader_macro()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;

    auto next_token_result(*token_current);
    if(next_token_result.is_err())
    {
      return next_token_result.err().unwrap();
    }
    auto next_token(next_token_result.expect_ok());
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
    switch(next_token.kind)
    {
      case lex::token_kind::open_curly_bracket:
        return parse_reader_macro_set();
      default:
        return err(
          error{ start_token.pos, native_persistent_string{ "unsupported reader macro" } });
    }
#pragma clang diagnostic pop
  }

  processor::object_result processor::parse_reader_macro_set()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;

    auto const prev_expected_closer(expected_closer);
    expected_closer = some(lex::token_kind::close_curly_bracket);

    rt_ctx
      .push_thread_bindings(runtime::obj::persistent_hash_map::create_unique(
        std::make_pair(splicing_allowed_var, runtime::obj::boolean::true_const())))
      .expect_ok();
    util::scope_exit const finally{ [&]() { rt_ctx.pop_thread_bindings().expect_ok(); } };

    runtime::detail::native_transient_set ret;
    for(auto it(begin()); it != end(); ++it)
    {
      if(it.latest.unwrap().is_err())
      {
        return err(it.latest.unwrap().expect_err());
      }
      ret.insert(it.latest.unwrap().expect_ok().unwrap().ptr);
    }
    if(expected_closer.is_some())
    {
      return err(error{ start_token.pos, "Unterminated set" });
    }

    expected_closer = prev_expected_closer;
    return object_source_info{ make_box<runtime::obj::persistent_set>(std::move(ret)),
                               start_token,
                               latest_token };
  }

  processor::object_result processor::parse_reader_macro_comment()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;

    auto ignored_result(next());
    if(ignored_result.is_err())
    {
      return ignored_result;
    }
    else if(ignored_result.expect_ok().is_none())
    {
      return err(
        error{ start_token.pos, native_persistent_string{ "value after #_ must be present" } });
    }

    return ok(none);
  }

  processor::object_result processor::parse_reader_macro_conditional(native_bool const splice)
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;

    auto list_result(next());
    if(list_result.is_err())
    {
      return list_result;
    }
    else if(list_result.expect_ok().is_none())
    {
      return err(
        error{ start_token.pos, native_persistent_string{ "value after #? must be present" } });
    }
    else if(list_result.expect_ok().unwrap().ptr->type != runtime::object_type::persistent_list)
    {
      return err(
        error{ start_token.pos, native_persistent_string{ "value after #? must be a list" } });
    }

    auto const list(
      runtime::expect_object<runtime::obj::persistent_list>(list_result.expect_ok().unwrap().ptr));
    auto const list_end(list_result.expect_ok().unwrap().end);

    if(list.data->count() % 2 == 1)
    {
      return err(
        error{ start_token.pos, native_persistent_string{ "#? expects an even number of forms" } });
    }

    auto const jank_keyword(rt_ctx.intern_keyword("", "jank").expect_ok());
    auto const default_keyword(rt_ctx.intern_keyword("", "default").expect_ok());

    for(auto it(list->fresh_seq()); it != nullptr;)
    {
      auto const kw(it->first());
      /* We take the first match, checking for :jank first. If there are duplicates, it doesn't
       * matter. If :default comes first, we'll always take it. In short, order is important. This
       * matches Clojure's behavior. */
      if(runtime::detail::equal(kw, jank_keyword) || runtime::detail::equal(kw, default_keyword))
      {
        if(splice)
        {
          if(!runtime::detail::truthy(splicing_allowed_var->deref()))
          {
            return err(error{ start_token.pos,
                              native_persistent_string{ "#?@ splice must not be top-level" } });
          }

          auto const s(it->next_in_place()->first());
          return runtime::visit_seqable(
            [&](auto const typed_s) -> processor::object_result {
              auto const seq(typed_s->fresh_seq());
              if(seq == nullptr)
              {
                return ok(none);
              }
              auto const first(seq->first());

              auto const front(pending_forms.begin());
              for(auto it(seq->next_in_place()); it != nullptr; it = it->next_in_place())
              {
                pending_forms.insert(front, it->first());
              }

              return object_source_info{ first, start_token, list_end };
            },
            [=]() -> processor::object_result {
              return err(error{ start_token.pos,
                                native_persistent_string{ "#?@ splice must be a sequence" } });
            },
            s);
        }
        else
        {
          return object_source_info{ it->next_in_place()->first(), start_token, list_end };
        }
      }

      it = it->next_in_place()->next_in_place();
    }

    return ok(none);
  }

  processor::object_result processor::parse_nil()
  {
    ++token_current;
    return object_source_info{ runtime::obj::nil::nil_const(), latest_token, latest_token };
  }

  processor::object_result processor::parse_boolean()
  {
    auto const token((*token_current).expect_ok());
    ++token_current;
    auto const b(boost::get<native_bool>(token.data));
    return object_source_info{ make_box<runtime::obj::boolean>(b), token, token };
  }

  processor::object_result processor::parse_symbol()
  {
    auto const token((*token_current).expect_ok());
    ++token_current;
    auto const sv(boost::get<native_persistent_string_view>(token.data));
    auto const slash(sv.find('/'));
    native_persistent_string ns, name;
    if(slash != native_persistent_string::npos)
    {
      /* If it's only a slash, it's a name. Otherwise, it's a ns/name separator. */
      if(sv.size() == 1)
      {
        name = sv;
      }
      else
      {
        auto const ns_portion(sv.substr(0, slash));
        /* Quoted symbols can have any ns and it doesn't need to exist. */
        if(quoted)
        {
          ns = ns_portion;
        }
        /* Normal symbols will have the ns resolved immediately. */
        else
        {
          auto const resolved_ns(rt_ctx.resolve_ns(make_box<runtime::obj::symbol>(ns_portion)));
          if(resolved_ns.is_none())
          {
            return err(error{ token.pos, fmt::format("unknown namespace: {}", ns_portion) });
          }
          ns = resolved_ns.unwrap()->name->name;
        }
        name = sv.substr(slash + 1);
      }
    }
    else
    {
      name = sv;
    }
    return object_source_info{ make_box<runtime::obj::symbol>(ns, name), token, token };
  }

  processor::object_result processor::parse_keyword()
  {
    auto const token((*token_current).expect_ok());
    ++token_current;
    auto const sv(boost::get<native_persistent_string_view>(token.data));
    /* A :: keyword either resolves to the current ns or an alias, depending on
     * whether or not it's qualified. */
    native_bool const resolved{ sv[0] != ':' };

    auto const slash(sv.find('/'));
    native_persistent_string ns, name;
    if(slash != native_persistent_string::npos)
    {
      if(resolved)
      {
        ns = sv.substr(0, slash);
      }
      else
      {
        ns = sv.substr(1, slash - 1);
      }
      name = sv.substr(slash + 1);
    }
    else
    {
      name = sv.substr(resolved ? 0 : 1);
    }

    auto const intern_res(rt_ctx.intern_keyword(runtime::obj::symbol{ ns, name }, resolved));
    if(intern_res.is_err())
    {
      return err(intern_res.expect_err());
    }
    return object_source_info{ intern_res.expect_ok(), token, token };
  }

  processor::object_result processor::parse_integer()
  {
    auto const token(token_current->expect_ok());
    ++token_current;
    return object_source_info{ make_box<runtime::obj::integer>(
                                 boost::get<native_integer>(token.data)),
                               token,
                               token };
  }

  processor::object_result processor::parse_real()
  {
    auto const token(token_current->expect_ok());
    ++token_current;
    return object_source_info{ make_box<runtime::obj::real>(boost::get<native_real>(token.data)),
                               token,
                               token };
  }

  processor::object_result processor::parse_string()
  {
    auto const token(token_current->expect_ok());
    ++token_current;
    auto const sv(boost::get<native_persistent_string_view>(token.data));
    return object_source_info{ make_box<runtime::obj::persistent_string>(
                                 native_persistent_string{ sv.data(), sv.size() }),
                               token,
                               token };
  }

  processor::object_result processor::parse_escaped_string()
  {
    auto const token(token_current->expect_ok());
    ++token_current;
    auto const sv(boost::get<native_persistent_string_view>(token.data));
    auto res(util::unescape({ sv.data(), sv.size() }));
    if(res.is_err())
    {
      return err(error{ token.pos, res.expect_err_move() });
    }
    return object_source_info{ make_box<runtime::obj::persistent_string>(res.expect_ok_move()),
                               token,
                               token };
  }

  processor::iterator processor::begin()
  {
    return { some(next()), *this };
  }

  processor::iterator processor::end()
  {
    return { some(ok(none)), *this };
  }
}
