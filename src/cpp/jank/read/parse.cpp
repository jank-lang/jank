#include <iostream>
#include <iomanip>

#include <magic_enum.hpp>

#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/vector.hpp>
#include <jank/runtime/obj/list.hpp>
#include <jank/runtime/obj/map.hpp>
#include <jank/runtime/obj/set.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/obj/string.hpp>
#include <jank/read/parse.hpp>

namespace jank::read::parse
{
  processor::iterator::value_type processor::iterator::operator *() const
  { return latest.unwrap(); }
  processor::iterator::pointer processor::iterator::operator ->()
  { return &latest.unwrap(); }
  processor::iterator& processor::iterator::operator ++()
  {
    latest = some(p.next());
    return *this;
  }
  bool processor::iterator::operator !=(processor::iterator const &rhs) const
  { return latest != rhs.latest; }
  bool processor::iterator::operator ==(processor::iterator const &rhs) const
  { return latest == rhs.latest; }
  processor::iterator& processor::iterator::operator=(processor::iterator const &rhs)
  {
    latest = rhs.latest;
    return *this;
  }

  processor::processor
  (
    runtime::context &rt_ctx,
    lex::processor::iterator const &b,
    lex::processor::iterator const &e
  )
    : rt_ctx{ rt_ctx }, token_current{ b }, token_end{ e }
  { }

  processor::object_result processor::next()
  {
    /* TODO: Replace nullptr with none. */
    if(token_current == token_end)
    { return ok(nullptr); }

    while(true)
    {
      auto token_result(*token_current);
      if(token_result.is_err())
      { return token_result.err().unwrap(); }
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
          { return err(error{ token.pos, native_string{ "unexpected closing character" } }); }
          ++token_current;
          expected_closer = none;
          return ok(nullptr);
        case lex::token_kind::single_quote:
          return parse_quote();
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
        case lex::token_kind::eof:
          return ok(nullptr);
        default:
        {
          native_string msg{ "unexpected token kind: " };
          msg += magic_enum::enum_name(token.kind);
          return err(error{ token.pos, msg });
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

    runtime::detail::transient_vector ret;
    for(auto it(begin()); it != end(); ++it)
    {
      if(it.latest.unwrap().is_err())
      { return err(it.latest.unwrap().expect_err()); }
      ret.push_back(it.latest.unwrap().expect_ok());
    }
    if(expected_closer.is_some())
    { return err(error{ start_token.pos, "Unterminated list" }); }

    expected_closer = prev_expected_closer;
    return make_box<runtime::obj::list>(ret.rbegin(), ret.rend());
  }

  processor::object_result processor::parse_vector()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;
    auto const prev_expected_closer(expected_closer);
    expected_closer = some(lex::token_kind::close_square_bracket);

    runtime::detail::transient_vector ret;
    for(auto it(begin()); it != end(); ++it)
    {
      if(it.latest.unwrap().is_err())
      { return err(it.latest.unwrap().expect_err()); }
      ret.push_back(it.latest.unwrap().expect_ok());
    }
    if(expected_closer.is_some())
    { return err(error{ start_token.pos, "Unterminated vector" }); }

    expected_closer = prev_expected_closer;
    return make_box<runtime::obj::vector>(ret.persistent());
  }

  /* TODO: Uniqueness check. */
  processor::object_result processor::parse_map()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;
    auto const prev_expected_closer(expected_closer);
    expected_closer = some(lex::token_kind::close_curly_bracket);

    /* TODO: Map transient. */
    runtime::detail::persistent_map ret;
    for(auto it(begin()); it != end(); ++it)
    {
      if(it.latest.unwrap().is_err())
      { return err(it.latest.unwrap().expect_err()); }
      auto const key(it.latest.unwrap().expect_ok());

      if(++it == end())
      { return err(error{ start_token.pos, "odd number of items in map" }); }

      if(it.latest.unwrap().is_err())
      { return err(it.latest.unwrap().expect_err()); }
      auto const value(it.latest.unwrap().expect_ok());

      ret.insert_or_assign(key, value);
    }
    if(expected_closer.is_some())
    { return err(error{ start_token.pos, "Unterminated map" }); }

    expected_closer = prev_expected_closer;
    return make_box<runtime::obj::map>(ret);
  }

  processor::object_result processor::parse_quote()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;
    auto val_result(next());
    if(val_result.is_err())
    { return val_result; }
    else if(val_result.expect_ok() == nullptr)
    { return err(error{ start_token.pos, native_string{ "invalid value after quote" } }); }

    return runtime::erase
    (
      jank::make_box<runtime::obj::list>
      (
        jank::make_box<runtime::obj::symbol>("quote"),
        val_result.expect_ok_move()
      )
    );
  }

  processor::object_result processor::parse_nil()
  {
    ++token_current;
    return ok(make_box<runtime::obj::nil>());
  }

  processor::object_result processor::parse_boolean()
  {
    auto const token((*token_current).expect_ok());
    ++token_current;
    auto const b(boost::get<bool>(token.data));
    return ok(make_box<runtime::obj::boolean>(b));
  }

  processor::object_result processor::parse_symbol()
  {
    auto const token((*token_current).expect_ok());
    ++token_current;
    auto const sv(boost::get<native_string_view>(token.data));
    auto const slash(sv.find('/'));
    native_string ns, name;
    if(slash != native_string::npos)
    {
      /* If it's only a slash, that a name. Otherwise, it's a ns/name separator. */
      if(sv.size() == 1)
      { name = sv; }
      else
      {
        ns = sv.substr(0, slash);
        name = sv.substr(slash + 1);
      }
    }
    else
    { name = sv; }
    return ok(make_box<runtime::obj::symbol>(ns, name));
  }

  processor::object_result processor::parse_keyword()
  {
    auto const token((*token_current).expect_ok());
    ++token_current;
    auto const sv(boost::get<native_string_view>(token.data));
    bool const resolved{ sv[0] != ':' };

    auto const slash(sv.find('/'));
    native_string ns, name;
    if(slash != native_string::npos)
    {
      if(resolved)
      { ns = sv.substr(0, slash); }
      else
      { ns = sv.substr(1, slash - 1); }
      name = sv.substr(slash + 1);
    }
    else
    { name = sv.substr(resolved ? 0 : 1); }
    return ok(rt_ctx.intern_keyword(runtime::obj::symbol{ ns, name }, resolved));
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
    auto const sv(boost::get<native_string_view>(token.data));
    return ok(make_box<runtime::obj::string>(native_string{ sv.data(), sv.size() }));
  }

  processor::iterator processor::begin()
  { return { some(next()), *this }; }
  processor::iterator processor::end()
  { return { some(ok(nullptr)), *this }; }
}

