#include <iostream>
#include <iomanip>

#include <jank/read/lex.hpp>

using namespace std::string_view_literals;

namespace jank::read
{
  error::error(size_t const s, native_persistent_string const &m)
    : start{ s }
    , end{ s }
    , message{ m }
  {
  }

  error::error(size_t const s, size_t const e, native_persistent_string const &m)
    : start{ s }
    , end{ e }
    , message{ m }
  {
  }

  error::error(native_persistent_string const &m)
    : message{ m }
  {
  }

  native_bool error::operator==(error const &rhs) const
  {
    return !(*this != rhs);
  }

  native_bool error::operator!=(error const &rhs) const
  {
    return start != rhs.start || end != rhs.end || message != rhs.message;
  }

  std::ostream &operator<<(std::ostream &os, error const &e)
  {
    return os << "error(" << e.start << " - " << e.end << ", \"" << e.message << "\")";
  }

  namespace lex
  {
    template <typename... Ts>
    static std::ostream &operator<<(std::ostream &os, std::variant<Ts...> const &v)
    {
      boost::apply_visitor(
        [&](auto &&arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr(std::is_same_v<T, native_persistent_string>
                       || std::is_same_v<T, native_persistent_string_view>)
          {
            os << std::quoted(arg);
          }
          else
          {
            os << arg;
          }
        },
        v);
      return os;
    }

    token::token(token_kind const k)
      : kind{ k }
    {
    }

    token::token(size_t const p, token_kind const k)
      : pos{ p }
      , kind{ k }
    {
    }

    token::token(size_t const p, token_kind const k, native_integer const d)
      : pos{ p }
      , kind{ k }
      , data{ d }
    {
    }

    token::token(size_t const p, token_kind const k, native_real const d)
      : pos{ p }
      , kind{ k }
      , data{ d }
    {
    }

    token::token(size_t const p, token_kind const k, native_persistent_string_view const d)
      : pos{ p }
      , kind{ k }
      , data{ d }
    {
    }

    token::token(size_t const p, token_kind const k, native_bool const d)
      : pos{ p }
      , kind{ k }
      , data{ d }
    {
    }

    token::token(size_t const p, size_t const s, token_kind const k)
      : pos{ p }
      , size{ s }
      , kind{ k }
    {
    }

    token::token(size_t const p, size_t const s, token_kind const k, native_integer const d)
      : pos{ p }
      , size{ s }
      , kind{ k }
      , data{ d }
    {
    }

    token::token(size_t const p, size_t const s, token_kind const k, native_real const d)
      : pos{ p }
      , size{ s }
      , kind{ k }
      , data{ d }
    {
    }

    token::token(size_t const p,
                 size_t const s,
                 token_kind const k,
                 native_persistent_string_view const d)
      : pos{ p }
      , size{ s }
      , kind{ k }
      , data{ d }
    {
    }

    token::token(size_t const p, size_t const s, token_kind const k, char const * const d)
      : pos{ p }
      , size{ s }
      , kind{ k }
      , data{ native_persistent_string_view{ d } }
    {
    }

    token::token(size_t const p, size_t const s, token_kind const k, native_bool const d)
      : pos{ p }
      , size{ s }
      , kind{ k }
      , data{ d }
    {
    }

    token::token(size_t const p, size_t const s, token_kind const k, ratio const d)
      : pos{ p }
      , size{ s }
      , kind{ k }
      , data{ d }
    {
    }

    struct codepoint
    {
      char32_t character{};
      uint8_t len{};
    };

    native_bool ratio::operator==(ratio const &rhs) const
    {
      return numerator == rhs.numerator && denominator == rhs.denominator;
    }

    native_bool ratio::operator!=(ratio const &rhs) const
    {
      return !(*this == rhs);
    }

    native_bool token::no_data::operator==(no_data const &) const
    {
      return true;
    }

    native_bool token::no_data::operator!=(no_data const &) const
    {
      return false;
    }

    native_bool token::operator==(token const &rhs) const
    {
      return !(*this != rhs);
    }

    native_bool token::operator!=(token const &rhs) const
    {
      return (pos != rhs.pos && pos != token::ignore_pos && rhs.pos != token::ignore_pos)
        || size != rhs.size || kind != rhs.kind || data != rhs.data;
    }

    std::ostream &operator<<(std::ostream &os, token const &t)
    {
      return os << "token(" << t.pos << ", " << t.size << ", " << token_kind_str(t.kind) << ", "
                << t.data << ")";
    }

    std::ostream &operator<<(std::ostream &os, token::no_data const &)
    {
      return os << "<no data>";
    }

    std::ostream &operator<<(std::ostream &os, ratio const &r)
    {
      return os << r.numerator << "/" << r.denominator;
    }

    processor::processor(native_persistent_string_view const &f)
      : file{ f }
    {
    }

    processor::iterator::value_type const &processor::iterator::operator*() const
    {
      return latest.unwrap();
    }

    processor::iterator::value_type const *processor::iterator::operator->() const
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

    processor::iterator processor::begin()
    {
      return { some(next()), *this };
    }

    processor::iterator processor::end()
    {
      return { some(token_kind::eof), *this };
    }

    option<error> processor::check_whitespace(native_bool const found_space)
    {
      if(require_space && !found_space)
      {
        require_space = false;
        return some(error{ pos, "expected whitespace before next token" });
      }
      return none;
    }

    static result<codepoint, error>
    convert_to_codepoint(native_persistent_string_view const sv, size_t const pos)
    {
      std::mbstate_t state{};
      wchar_t wc{};
      auto const len{ std::mbrtowc(&wc, sv.data(), sv.size(), &state) };

      if(len == static_cast<size_t>(-1))
      {
        return err(error{ pos, "Unfinished Character" });
      }
      else if(len == static_cast<size_t>(-2))
      {
        return err(error{ pos, "Invalid character" });
      }
      return ok(codepoint{ static_cast<char32_t>(wc), static_cast<uint8_t>(len) });
    }

    static native_bool is_utf8_char(char32_t const c)
    {
      /* Checks if the codepoint is within Unicode scalar ranges */
      if(c <= 0x7FF)
      /* NOLINTNEXTLINE(bugprone-branch-clone) */
      {
        /* ASCII (U+0000 - U+007F) and 2-byte range (U+0080 - U+07FF) */
        return true;
      }
      else if(c <= 0xFFFF)
      {
        /* 3-byte range (U+0800 - U+FFFF) */
        return c < 0xD800 || c > 0xDFFF;
      }
      else if(c <= 0x10FFFF)
      {
        /* 4-byte range (U+10000 - U+10FFFF) */
        return true;
      }
      /* Outside the range */
      return false;
    }

    static native_bool is_special_char(char32_t const c)
    {
      return c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' || c == '"'
        || c == '^' || c == '\\' || c == '`' || c == '~';
    }

    static native_bool is_symbol_char(char32_t const c)
    {
      return !std::iswspace(c) && !is_special_char(c)
        && (std::iswalnum(static_cast<wint_t>(c)) != 0 || c == '_' || c == '-' || c == '/'
            || c == '?' || c == '!' || c == '+' || c == '*' || c == '=' || c == '.' || c == '&'
            || c == '<' || c == '>' || c == '#' || c == '%' || is_utf8_char(c));
    }

    result<token, error> processor::next()
    {
      /* Skip whitespace. */
      native_bool found_space{};
      while(true)
      {
        if(pos >= file.size())
        {
          return ok(token{ pos, token_kind::eof });
        }

        auto const c(file[pos]);
        if(std::isspace(c) == 0 && c != ',')
        {
          break;
        }

        found_space = true;
        ++pos;
      }

      auto const token_start(pos);
      auto const oc{ convert_to_codepoint(file.substr(token_start), token_start) };
      if(oc.is_err())
      {
        ++pos;
        return oc.expect_err();
      }
      switch(oc.expect_ok().character)
      {
        case '(':
          require_space = false;
          return ok(token{ pos++, token_kind::open_paren });
        case ')':
          require_space = false;
          return ok(token{ pos++, token_kind::close_paren });
        case '[':
          require_space = false;
          return ok(token{ pos++, token_kind::open_square_bracket });
        case ']':
          require_space = false;
          return ok(token{ pos++, token_kind::close_square_bracket });
        case '{':
          require_space = false;
          return ok(token{ pos++, token_kind::open_curly_bracket });
        case '}':
          require_space = false;
          return ok(token{ pos++, token_kind::close_curly_bracket });
        case '\'':
          require_space = false;
          return ok(token{ pos++, token_kind::single_quote });
        case '\\':
          {
            require_space = false;

            auto const ch(peek());
            pos++;

            if(ch.is_err() || std::iswspace(ch.expect_ok().character))
            {
              return err(error{ token_start, "Expecting a valid character literal after \\" });
            }

            while(pos <= file.size())
            {
              auto const pt(peek());
              if(pt.is_err() || !is_symbol_char(pt.expect_ok().character))
              {
                break;
              }
              pos += pt.expect_ok().len;
            }

            native_persistent_string_view const data{ file.data() + token_start,
                                                      ++pos - token_start };
            return ok(token{ token_start, pos - token_start, token_kind::character, data });
          }
        case ';':
          {
            size_t leading_semis{ 1 };
            native_bool hit_non_semi{};
            while(true)
            {
              auto const oc(peek());
              if(oc.is_err())
              {
                break;
              }
              auto const c(oc.expect_ok().character);
              if(c == '\n')
              {
                break;
              }
              else if(c == ';' && !hit_non_semi)
              {
                ++leading_semis;
              }
              else
              {
                hit_non_semi = true;
              }

              ++pos;
            }
            if(pos == token_start)
            {
              return ok(token{ pos++, 1, token_kind::comment, ""sv });
            }
            else
            {
              ++pos;
              native_persistent_string_view const comment{ file.data() + token_start
                                                             + leading_semis,
                                                           pos - token_start - leading_semis };
              return ok(token{ token_start, pos - 1 - token_start, token_kind::comment, comment });
            }
          }
        /* Numbers. */
        case '-':
        case '0' ... '9':
          {
            auto &&e(check_whitespace(found_space));
            if(e.is_some())
            {
              return err(std::move(e.unwrap()));
            }
            native_bool contains_leading_digit{ file[token_start] != '-' };
            native_bool contains_dot{};
            native_bool is_scientific{};
            native_bool found_exponent_sign{};
            native_bool expecting_exponent{};
            while(true)
            {
              auto const oc(peek());
              if(oc.is_err())
              {
                break;
              }

              auto const c(oc.expect_ok().character);
              if(c == '.')
              {
                if(contains_dot || is_scientific || !contains_leading_digit)
                {
                  ++pos;
                  return err(error{ token_start, pos, "invalid number" });
                }
                contains_dot = true;
              }
              else if(c == 'e' || c == 'E')
              {
                if(is_scientific || !contains_leading_digit)
                {
                  ++pos;
                  return err(error{ token_start, pos, "invalid number" });
                }
                is_scientific = true;
                expecting_exponent = true;
              }
              else if(c == '+' || c == '-')
              {
                if(found_exponent_sign || !is_scientific || !expecting_exponent)
                {
                  ++pos;
                  return err(error{ token_start, pos, "invalid number" });
                }
                found_exponent_sign = true;
              }
              else if(c == '/')
              {
                require_space = false;
                ++pos;
                if(found_exponent_sign || is_scientific || expecting_exponent || contains_dot
                   || found_slash_after_number)
                {
                  return err(error{ token_start, pos, "invalid ratio" });
                }
                found_slash_after_number = true;
                /* skip the '/' char and look for the denominator number. */
                ++pos;
                auto const denominator(next());
                if(denominator.is_ok() && denominator.expect_ok().kind == token_kind::integer)
                {
                  auto const &denominator_token(denominator.expect_ok());
                  found_slash_after_number = false;
                  return ok(
                    token(token_start,
                          pos - token_start,
                          token_kind::ratio,
                          { .numerator = std::strtoll(file.data() + token_start, nullptr, 10),
                            .denominator = boost::get<native_integer>(denominator_token.data) }));
                }
                return err(
                  error{ token_start, pos, "invalid ratio: expecting an integer denominator" });
              }
              else if(std::iswdigit(c) == 0)
              {
                if(expecting_exponent)
                {
                  ++pos;
                  return err(
                    error{ token_start, pos, "unexpected end of real, expecting exponent" });
                }
                break;
              }
              else if(expecting_exponent)
              {
                expecting_exponent = false;
              }

              contains_leading_digit = true;

              ++pos;
            }

            if(expecting_exponent)
            {
              ++pos;
              return err(error{ token_start, pos, "unexpected end of real, expecting exponent" });
            }

            /* Tokens beginning with - are ambiguous; it's only a negative number if it has numbers
             * to follow.
             * TODO: handle numbers starting with `+` */
            if(file[token_start] != '-' || (pos - token_start) >= 1)
            {
              require_space = true;
              ++pos;
              if(contains_dot || is_scientific)
              {
                return ok(token{ token_start,
                                 pos - token_start,
                                 token_kind::real,
                                 std::strtod(file.data() + token_start, nullptr) });
              }

              {
                return ok(token{ token_start,
                                 pos - token_start,
                                 token_kind::integer,
                                 std::strtoll(file.data() + token_start, nullptr, 10) });
              }
            }
            /* XXX: Fall through to symbol starting with - */
          }
        /* Symbols. */
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_':
        case '/':
        case '?':
        case '+':
        case '=':
        case '*':
        case '!':
        case '&':
        case '<':
        case '>':
        case '%':
          {
            auto &&e(check_whitespace(found_space));
            if(e.is_some())
            {
              return err(std::move(e.unwrap()));
            }
            while(true)
            {
              auto const oc(peek());
              if(oc.is_err())
              {
                break;
              }
              auto const c(oc.expect_ok().character);
              auto const size(oc.expect_ok().len);
              if(!is_symbol_char(c))
              {
                break;
              }
              pos += size;
            }
            require_space = true;
            native_persistent_string_view const name{ file.data() + token_start,
                                                      ++pos - token_start };
            if(name[0] == '/' && name.size() > 1)
            {
              return err(error{ token_start, "invalid symbol" });
            }
            else if(name == "nil")
            {
              return ok(token{ token_start, pos - token_start, token_kind::nil });
            }
            else if(name == "true")
            {
              return ok(token{ token_start, pos - token_start, token_kind::boolean, true });
            }
            else if(name == "false")
            {
              return ok(token{ token_start, pos - token_start, token_kind::boolean, false });
            }
            return ok(token{ token_start, pos - token_start, token_kind::symbol, name });
          }
        /* Keywords. */
        case ':':
          {
            auto &&e(check_whitespace(found_space));
            if(e.is_some())
            {
              return err(std::move(e.unwrap()));
            }

            auto const oc(peek());
            auto const c(oc.expect_ok().character);
            if(oc.is_err() || std::iswspace(c))
            {
              ++pos;
              return err(
                error{ token_start, "invalid keyword: expected non-whitespace character after :" });
            }

            /* Support auto-resolved qualified keywords. */
            if(c == ':')
            {
              ++pos;
            }

            while(true)
            {
              auto const oc(peek());
              if(oc.is_err())
              {
                break;
              }

              auto const codepoint(oc.expect_ok());
              if(!is_symbol_char(codepoint.character))
              {
                break;
              }
              pos += codepoint.len;
            }
            require_space = true;
            native_persistent_string_view const name{ file.data() + token_start + 1,
                                                      ++pos - token_start - 1 };

            if(name[0] == '/' && name.size() > 1)
            {
              return err(error{ token_start, "invalid keyword: starts with /" });
            }
            else if(name[0] == ':' && name[1] == ':')
            {
              return err(error{ token_start, "invalid keyword: incorrect number of :" });
            }

            return ok(token{ token_start, pos - token_start, token_kind::keyword, name });
          }
        /* Strings. */
        case '"':
          {
            auto &&e(check_whitespace(found_space));
            if(e.is_some())
            {
              return err(std::move(e.unwrap()));
            }
            auto const token_start(pos);
            native_bool escaped{}, contains_escape{};
            while(true)
            {
              auto const oc(peek());
              if(oc.is_err())
              {
                ++pos;
                return err(error{ token_start, "unterminated string" });
              }
              else if(!escaped && oc.expect_ok().character == '"')
              {
                ++pos;
                break;
              }

              if(escaped)
              {
                switch(oc.expect_ok().character)
                {
                  case '"':
                  case '?':
                  case '\'':
                  case '\\':
                  case 'a':
                  case 'b':
                  case 'f':
                  case 'n':
                  case 'r':
                  case 't':
                  case 'v':
                    break;
                  default:
                    return err(error{ pos, "unsupported escape character" });
                }
                escaped = false;
              }
              else if(oc.expect_ok().character == '\\')
              {
                escaped = contains_escape = true;
              }
              ++pos;
            }
            require_space = true;
            pos++;

            /* Unescaped strings can be read right from memory, but escaped strings require
             * some processing first, to turn the escape sequences into the necessary characters.
             * We use distinct token types for these so we can optimize for the typical case. */
            auto const kind(contains_escape ? token_kind::escaped_string : token_kind::string);
            return ok(token{ token_start,
                             pos - token_start,
                             kind,
                             native_persistent_string_view(file.data() + token_start + 1,
                                                           pos - token_start - 2) });
          }
        /* Meta hints. */
        case '^':
          {
            auto &&e(check_whitespace(found_space));
            if(e.is_some())
            {
              return err(std::move(e.unwrap()));
            }
            ++pos;
            require_space = false;

            return ok(token{ token_start, pos - token_start, token_kind::meta_hint });
          }
        /* Reader macros. */
        case '#':
          {
            auto &&e(check_whitespace(found_space));
            if(e.is_some())
            {
              return err(std::move(e.unwrap()));
            }
            require_space = false;
            auto const oc(peek());
            auto const cp(oc.unwrap_or(codepoint{ ' ', 1 }));
            pos += cp.len;

            switch(cp.character)
            {
              case '_':
                ++pos;
                return ok(
                  token{ token_start, pos - token_start, token_kind::reader_macro_comment });
              case '?':
                {
                  auto const maybe_splice(peek());

                  auto const cp(maybe_splice.unwrap_or(codepoint{ ' ', 1 }));
                  pos += cp.len;

                  if(cp.character == '@')
                  {
                    ++pos;
                    return ok(token{ token_start,
                                     pos - token_start,
                                     token_kind::reader_macro_conditional_splice });
                  }
                  else
                  {
                    return ok(token{ token_start,
                                     pos - token_start,
                                     token_kind::reader_macro_conditional });
                  }
                }
              default:
                break;
            }
            return ok(token{ token_start, pos - token_start, token_kind::reader_macro });
          }
        /* Syntax quoting. */
        case '`':
          {
            auto &&e(check_whitespace(found_space));
            if(e.is_some())
            {
              return err(std::move(e.unwrap()));
            }
            ++pos;
            require_space = false;

            return ok(token{ token_start, pos - token_start, token_kind::syntax_quote });
          }
        /* Syntax unquoting. */
        case '~':
          {
            auto &&e(check_whitespace(found_space));
            if(e.is_some())
            {
              return err(std::move(e.unwrap()));
            }
            require_space = false;
            auto const oc(peek());

            auto const cp(oc.unwrap_or(codepoint{ ' ', 1 }));
            pos += cp.len;
            switch(cp.character)
            {
              case '@':
                {
                  ++pos;
                  return ok(token{ token_start, pos - token_start, token_kind::unquote_splice });
                }
              default:
                return ok(token{ token_start, pos - token_start, token_kind::unquote });
            }
          }
        /* Deref macro. */
        case '@':
          {
            auto &&e(check_whitespace(found_space));
            if(e.is_some())
            {
              return err(std::move(e.unwrap()));
            }
            ++pos;
            require_space = false;

            return ok(token{ token_start, pos - token_start, token_kind::deref });
          }
        default:
          /* To handle all UTF-8 Characters that could be the beginning of a symbol (e.g an emoji) */
          if(oc.expect_ok().character != '.' && is_utf8_char(oc.expect_ok().character))
          {
            auto &&e(check_whitespace(found_space));
            if(e.is_some())
            {
              return err(std::move(e.unwrap()));
            }
            pos += oc.expect_ok().len;
            while(pos <= file.size())
            {
              auto const oc(convert_to_codepoint(file.substr(pos), pos));
              if(oc.is_err())
              {
                break;
              }

              if(!is_symbol_char(oc.expect_ok().character))
              {
                break;
              }
              pos += oc.expect_ok().len;
            }
            require_space = true;
            native_persistent_string_view const name{ file.data() + token_start,
                                                      pos - token_start };

            return ok(token{ token_start, pos - token_start, token_kind::symbol, name });
          }
          ++pos;
          return err(
            error{ token_start,
                   native_persistent_string{ "unexpected character: " } + file[token_start] });
      }
    }

    result<codepoint, error> processor::peek() const
    {
      auto const next_pos(pos + 1);
      if(next_pos >= file.size())
      {
        return err(error{ pos, "No more characters to peek." });
      }
      auto const oc{ convert_to_codepoint(file.substr(next_pos), next_pos) };
      return oc;
    }
  }
}
