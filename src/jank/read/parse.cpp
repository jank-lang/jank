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
  {
    if(!latest)
    /* TODO: panic */
    { std::abort(); }
    return *latest;
  }
  processor::iterator::pointer processor::iterator::operator ->()
  {
    if(!latest)
    /* TODO: panic */
    { std::abort(); }
    return &(*latest);
  }
  processor::iterator& processor::iterator::operator ++()
  {
    latest = some(p.next());
    return *this;
  }
  bool processor::iterator::operator !=(processor::iterator const &rhs) const
  { return latest != rhs.latest; }
  bool processor::iterator::operator ==(processor::iterator const &rhs) const
  { return latest == rhs.latest; }

  result<runtime::object_ptr, error> processor::next()
  {
    if(token_current == token_end)
    { return ok(nullptr); }

    auto token_result(*token_current);
    if(token_result.is_err())
    { return token_result.err().value(); }
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

  result<runtime::object_ptr, error> processor::parse_list()
  {
    auto const start_token(token_current.latest->expect_ok());
    ++token_current;
    auto const prev_expected_closer(expected_closer);
    expected_closer = some(lex::token_kind::close_paren);

    runtime::detail::vector_transient_type ret;
    for(auto it(begin()); it != end(); ++it)
    {
      if(it.latest->is_err())
      { return err(it.latest->expect_err()); }
      ret.push_back(it.latest->expect_ok());
    }
    if(expected_closer)
    { return err(error{ start_token.pos, "Unterminated list" }); }

    expected_closer = prev_expected_closer;
    return runtime::make_box<runtime::obj::list>(runtime::detail::list_type{ ret.rbegin(), ret.rend() });
  }

  result<runtime::object_ptr, error> processor::parse_vector()
  {
    auto const start_token(token_current.latest->expect_ok());
    ++token_current;
    auto const prev_expected_closer(expected_closer);
    expected_closer = some(lex::token_kind::close_square_bracket);

    runtime::detail::vector_transient_type ret;
    for(auto it(begin()); it != end(); ++it)
    {
      if(it.latest->is_err())
      { return err(it.latest->expect_err()); }
      ret.push_back(it.latest->expect_ok());
    }
    if(expected_closer)
    { return err(error{ start_token.pos, "Unterminated vector" }); }

    expected_closer = prev_expected_closer;
    return runtime::make_box<runtime::obj::vector>(ret.persistent());
  }

  result<runtime::object_ptr, error> processor::parse_map()
  {
    ++token_current;
    auto const prev_expected_closer(expected_closer);
    expected_closer = some(lex::token_kind::close_curly_bracket);



    expected_closer = prev_expected_closer;
    throw "unimplemented";
  }

  result<runtime::object_ptr, error> processor::parse_symbol()
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

  result<runtime::object_ptr, error> processor::parse_keyword()
  {
    throw "unimplemented";
  }

  result<runtime::object_ptr, error> processor::parse_integer()
  {
    // TODO: -> operator
    auto token((*token_current).expect_ok());
    ++token_current;
    return ok(runtime::make_box<runtime::obj::integer>(std::get<runtime::detail::integer_type>(token.data)));
  }

  result<runtime::object_ptr, error> processor::parse_string()
  {
    auto token((*token_current).expect_ok());
    ++token_current;
    auto const sv(std::get<std::string_view>(token.data));
    return ok(runtime::make_box<runtime::obj::string>(std::string{ sv.data(), sv.size() }));
  }

  processor::iterator processor::begin()
  { return { some(next()), *this }; }
  processor::iterator processor::end()
  { return { some(nullptr), *this }; }
}

