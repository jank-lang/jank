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

namespace jank::read::parse
{
  namespace detail
  {
    string_result<native_transient_string> unescape(native_transient_string const &input)
    {
      native_transient_string ss;
      ss.reserve(input.size());
      native_bool escape{};

      for(auto const c : input)
      {
        if(!escape)
        {
          if(c == '\\')
          {
            escape = true;
          }
          else
          {
            ss += c;
          }
        }
        else
        {
          switch(c)
          {
            case 'n':
              ss += '\n';
              break;
            case 't':
              ss += '\t';
              break;
            case 'r':
              ss += '\r';
              break;
            case '\\':
              ss += '\\';
              break;
            case '"':
              ss += '"';
              break;
            default:
              return err(fmt::format("invalid escape sequence: \\{}", c));
          }
          escape = false;
        }
      }

      return ok(ss);
    }
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
  {
  }

  processor::object_result processor::next()
  {
    /* TODO: Replace nullptr with none. */
    if(token_current == token_end)
    {
      return ok(nullptr);
    }

    while(true)
    {
      auto token_result(*token_current);
      if(token_result.is_err())
      {
        return token_result.err().unwrap();
      }
      auto token(token_result.expect_ok());
      switch(token.kind)
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
          if(expected_closer != token.kind)
          {
            return err(
              error{ token.pos, native_persistent_string{ "unexpected closing character" } });
          }
          ++token_current;
          expected_closer = none;
          return ok(nullptr);
        case lex::token_kind::single_quote:
          return parse_quote();
        case lex::token_kind::meta_hint:
          return parse_meta_hint();
        case lex::token_kind::reader_macro:
          return parse_reader_macro();
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
          return ok(nullptr);
        default:
          {
            native_persistent_string msg{ fmt::format("unexpected token kind: {}",
                                                      magic_enum::enum_name(token.kind)) };
            return err(error{ token.pos, std::move(msg) });
          }
      }
    }
  }

  processor::object_result processor::parse_list()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;
    auto const prev_expected_closer(expected_closer);
    expected_closer = some(lex::token_kind::close_paren);

    runtime::detail::native_transient_vector ret;
    for(auto it(begin()); it != end(); ++it)
    {
      if(it.latest.unwrap().is_err())
      {
        return err(it.latest.unwrap().expect_err());
      }
      ret.push_back(it.latest.unwrap().expect_ok());
    }
    if(expected_closer.is_some())
    {
      return err(error{ start_token.pos, "Unterminated list" });
    }

    expected_closer = prev_expected_closer;
    return make_box<runtime::obj::persistent_list>(ret.rbegin(), ret.rend());
  }

  processor::object_result processor::parse_vector()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;
    auto const prev_expected_closer(expected_closer);
    expected_closer = some(lex::token_kind::close_square_bracket);

    runtime::detail::native_transient_vector ret;
    for(auto it(begin()); it != end(); ++it)
    {
      if(it.latest.unwrap().is_err())
      {
        return err(it.latest.unwrap().expect_err());
      }
      ret.push_back(it.latest.unwrap().expect_ok());
    }
    if(expected_closer.is_some())
    {
      return err(error{ start_token.pos, "Unterminated vector" });
    }

    expected_closer = prev_expected_closer;
    return make_box<runtime::obj::persistent_vector>(ret.persistent());
  }

  /* TODO: Uniqueness check. */
  processor::object_result processor::parse_map()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;
    auto const prev_expected_closer(expected_closer);
    expected_closer = some(lex::token_kind::close_curly_bracket);

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

      ret.insert_or_assign(key, value);
    }
    if(expected_closer.is_some())
    {
      return err(error{ start_token.pos, "Unterminated map" });
    }

    expected_closer = prev_expected_closer;
    return make_box<runtime::obj::persistent_array_map>(ret);
  }

  processor::object_result processor::parse_quote()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;
    auto const old_quoted(quoted);
    quoted = true;
    auto val_result(next());
    quoted = old_quoted;
    if(val_result.is_err())
    {
      return val_result;
    }
    else if(val_result.expect_ok() == nullptr)
    {
      return err(error{ start_token.pos, native_persistent_string{ "invalid value after quote" } });
    }

    return runtime::erase(
      make_box<runtime::obj::persistent_list>(make_box<runtime::obj::symbol>("quote"),
                                              val_result.expect_ok_move()));
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
    else if(meta_val_result.expect_ok() == nullptr)
    {
      return err(
        error{ start_token.pos, native_persistent_string{ "invalid meta value after meta hint" } });
    }

    auto meta_result(runtime::visit_object(
      [&](auto const typed_val) -> processor::object_result {
        using T = typename decltype(typed_val)::value_type;
        if constexpr(std::same_as<T, runtime::obj::keyword>)
        {
          return runtime::obj::persistent_array_map::create_unique(
            typed_val,
            runtime::obj::boolean::true_const());
        }
        /* TODO: Concept for map-like. */
        if constexpr(std::same_as<T, runtime::obj::persistent_hash_map>
                     || std::same_as<T, runtime::obj::persistent_array_map>)
        {
          return typed_val;
        }
        else
        {
          return err(error{
            start_token.pos,
            native_persistent_string{ "value after meta hint ^ must be a keyword or map" } });
        }
      },
      meta_val_result.expect_ok()));
    if(meta_result.is_err())
    {
      return meta_result;
    }

    auto target_val_result(next());
    if(target_val_result.is_err())
    {
      return target_val_result;
    }
    else if(target_val_result.expect_ok() == nullptr)
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
            return typed_val->with_meta(meta_result.expect_ok());
          }
          else
          {
            return typed_val->with_meta(
              runtime::merge(typed_val->meta.unwrap(), meta_result.expect_ok()));
          }
        }
        else
        {
          return err(
            error{ start_token.pos,
                   native_persistent_string{ "target value for meta hint must accept metadata" } });
        }
      },
      target_val_result.expect_ok());
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

    runtime::detail::native_transient_set ret;
    for(auto it(begin()); it != end(); ++it)
    {
      if(it.latest.unwrap().is_err())
      {
        return err(it.latest.unwrap().expect_err());
      }
      ret.insert(it.latest.unwrap().expect_ok());
    }
    if(expected_closer.is_some())
    {
      return err(error{ start_token.pos, "Unterminated set" });
    }

    expected_closer = prev_expected_closer;
    return make_box<runtime::obj::persistent_set>(std::move(ret));
  }

  processor::object_result processor::parse_nil()
  {
    ++token_current;
    return ok(runtime::obj::nil::nil_const());
  }

  processor::object_result processor::parse_boolean()
  {
    auto const token((*token_current).expect_ok());
    ++token_current;
    auto const b(boost::get<native_bool>(token.data));
    return ok(make_box<runtime::obj::boolean>(b));
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
    return ok(make_box<runtime::obj::symbol>(ns, name));
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
    return ok(intern_res.expect_ok());
  }

  processor::object_result processor::parse_integer()
  {
    auto const token(token_current->expect_ok());
    ++token_current;
    return ok(make_box<runtime::obj::integer>(boost::get<native_integer>(token.data)));
  }

  processor::object_result processor::parse_real()
  {
    auto const token(token_current->expect_ok());
    ++token_current;
    return ok(make_box<runtime::obj::real>(boost::get<native_real>(token.data)));
  }

  processor::object_result processor::parse_string()
  {
    auto const token(token_current->expect_ok());
    ++token_current;
    auto const sv(boost::get<native_persistent_string_view>(token.data));
    return ok(
      make_box<runtime::obj::persistent_string>(native_persistent_string{ sv.data(), sv.size() }));
  }

  processor::object_result processor::parse_escaped_string()
  {
    auto const token(token_current->expect_ok());
    ++token_current;
    auto const sv(boost::get<native_persistent_string_view>(token.data));
    auto res(detail::unescape({ sv.data(), sv.size() }));
    if(res.is_err())
    {
      return err(error{ token.pos, res.expect_err_move() });
    }
    return ok(make_box<runtime::obj::persistent_string>(res.expect_ok_move()));
  }

  processor::iterator processor::begin()
  {
    return { some(next()), *this };
  }

  processor::iterator processor::end()
  {
    return { some(ok(nullptr)), *this };
  }
}
