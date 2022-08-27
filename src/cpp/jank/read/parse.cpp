#include <iostream>
#include <iomanip>

#include <magic_enum.hpp>

#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/vector.hpp>
#include <jank/runtime/obj/list.hpp>
#include <jank/runtime/obj/map.hpp>
#include <jank/runtime/obj/set.hpp>
#include <jank/runtime/obj/symbol.hpp>
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

  processor::object_result processor::next()
  {
    /* TODO: Replace nullptr with none. */
    if(token_current == token_end)
    { return ok(nullptr); }

    auto token_result(*token_current);
    if(token_result.is_err())
    { return token_result.err().unwrap(); }
    auto token(token_result.expect_ok());
    switch(token.kind)
    {
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
        { return err(error{ token.pos, std::string{ "unexpected closing character" } }); }
        ++token_current;
        expected_closer = none;
        return ok(nullptr);
      case lex::token_kind::single_quote:
        return parse_quote();
      case lex::token_kind::nil:
        return parse_nil();
      case lex::token_kind::symbol:
        return parse_symbol();
      case lex::token_kind::keyword:
        return parse_keyword();
      case lex::token_kind::integer:
        return parse_integer();
      case lex::token_kind::string:
        return parse_string();
      case lex::token_kind::eof:
      default:
        return err(error{ token.pos, std::string{ "unexpected character" } });
    }
  }

  processor::object_result processor::parse_list()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;
    auto const prev_expected_closer(expected_closer);
    expected_closer = some(lex::token_kind::close_paren);

    runtime::detail::vector_transient_type ret;
    for(auto it(begin()); it != end(); ++it)
    {
      if(it.latest.unwrap().is_err())
      { return err(it.latest.unwrap().expect_err()); }
      ret.push_back(it.latest.unwrap().expect_ok());
    }
    if(expected_closer.is_some())
    { return err(error{ start_token.pos, "Unterminated list" }); }

    expected_closer = prev_expected_closer;
    return runtime::make_box<runtime::obj::list>(ret.rbegin(), ret.rend());
  }

  processor::object_result processor::parse_vector()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;
    auto const prev_expected_closer(expected_closer);
    expected_closer = some(lex::token_kind::close_square_bracket);

    runtime::detail::vector_transient_type ret;
    for(auto it(begin()); it != end(); ++it)
    {
      if(it.latest.unwrap().is_err())
      { return err(it.latest.unwrap().expect_err()); }
      ret.push_back(it.latest.unwrap().expect_ok());
    }
    if(expected_closer.is_some())
    { return err(error{ start_token.pos, "Unterminated vector" }); }

    expected_closer = prev_expected_closer;
    return runtime::make_box<runtime::obj::vector>(ret.persistent());
  }

  processor::object_result processor::parse_map()
  {
    ++token_current;
    auto const prev_expected_closer(expected_closer);
    expected_closer = some(lex::token_kind::close_curly_bracket);

    /* TODO */


    expected_closer = prev_expected_closer;
    return err(error{ "unimplemented: maps" });
  }

  processor::object_result processor::parse_quote()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;
    auto val_result(next());
    if(val_result.is_err())
    { return val_result; }
    else if(val_result.expect_ok() == nullptr)
    { return err(error{ start_token.pos, std::string{ "invalid value after quote" } }); }

    return runtime::obj::list::create
    (
      runtime::obj::symbol::create("quote"),
      val_result.expect_ok_move()
    );
  }

  processor::object_result processor::parse_nil()
  {
    ++token_current;
    return ok(runtime::make_box<runtime::obj::nil>());
  }

  processor::object_result processor::parse_symbol()
  {
    auto token((*token_current).expect_ok());
    ++token_current;
    auto const sv(std::get<std::string_view>(token.data));
    auto const slash(sv.find('/'));
    std::string ns, name;
    if(slash != std::string::npos)
    {
      ns = sv.substr(0, slash);
      name = sv.substr(slash + 1);
    }
    else
    { name = sv; }
    return ok(runtime::make_box<runtime::obj::symbol>(ns, name));
  }

  processor::object_result processor::parse_keyword()
  {
    /* TODO */
    return err(error{ "unimplemented: keywords" });
  }

  processor::object_result processor::parse_integer()
  {
    auto const token(token_current->expect_ok());
    ++token_current;
    return ok(runtime::make_box<runtime::obj::integer>(std::get<runtime::detail::integer_type>(token.data)));
  }

  processor::object_result processor::parse_string()
  {
    auto const token(token_current->expect_ok());
    ++token_current;
    auto const sv(std::get<std::string_view>(token.data));
    return ok(runtime::make_box<runtime::obj::string>(std::string{ sv.data(), sv.size() }));
  }

  processor::iterator processor::begin()
  { return { some(next()), *this }; }
  processor::iterator processor::end()
  { return { some(ok(nullptr)), *this }; }
}

