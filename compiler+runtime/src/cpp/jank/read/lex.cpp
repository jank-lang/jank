#include <iostream>
#include <iomanip>
#include <fmt/format.h>

#include <jank/read/lex.hpp>
#include <jank/error/lex.hpp>
#include <jank/runtime/object.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/to_string.hpp>

using namespace std::string_view_literals;

namespace jank::read::lex
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
    : start{ k == token_kind::eof ? ignore_pos : 0 }
    , kind{ k }
  {
  }

  token::token(movable_position const &s, token_kind const k)
    : start{ s } /* NOLINT(cppcoreguidelines-slicing) */
    , end{ s } /* NOLINT(cppcoreguidelines-slicing) */
    , kind{ k }
  {
  }

  token::token(movable_position const &s, movable_position const &e, token_kind const k)
    : start{ s } /* NOLINT(cppcoreguidelines-slicing) */
    , end{ e } /* NOLINT(cppcoreguidelines-slicing) */
    , kind{ k }
  {
  }

  token::token(movable_position const &s,
               movable_position const &e,
               token_kind const k,
               native_integer const d)
    : start{ s } /* NOLINT(cppcoreguidelines-slicing) */
    , end{ e } /* NOLINT(cppcoreguidelines-slicing) */
    , kind{ k }
    , data{ d }
  {
  }

  token::token(movable_position const &s,
               movable_position const &e,
               token_kind const k,
               native_real const d)
    : start{ s } /* NOLINT(cppcoreguidelines-slicing) */
    , end{ e } /* NOLINT(cppcoreguidelines-slicing) */
    , kind{ k }
    , data{ d }
  {
  }

  token::token(movable_position const &s,
               movable_position const &e,
               token_kind const k,
               native_persistent_string_view const d)
    : start{ s } /* NOLINT(cppcoreguidelines-slicing) */
    , end{ e } /* NOLINT(cppcoreguidelines-slicing) */
    , kind{ k }
    , data{ d }
  {
  }

  token::token(movable_position const &s,
               movable_position const &e,
               token_kind const k,
               char const * const d)
    : start{ s } /* NOLINT(cppcoreguidelines-slicing) */
    , end{ e } /* NOLINT(cppcoreguidelines-slicing) */
    , kind{ k }
    , data{ native_persistent_string_view{ d } }
  {
  }

  token::token(movable_position const &s,
               movable_position const &e,
               token_kind const k,
               native_bool const d)
    : start{ s } /* NOLINT(cppcoreguidelines-slicing) */
    , end{ e } /* NOLINT(cppcoreguidelines-slicing) */
    , kind{ k }
    , data{ d }
  {
  }

  token::token(movable_position const &s,
               movable_position const &e,
               token_kind const k,
               ratio const d)
    : start{ s } /* NOLINT(cppcoreguidelines-slicing) */
    , end{ e } /* NOLINT(cppcoreguidelines-slicing) */
    , kind{ k }
    , data{ d }
  {
  }

#ifdef JANK_TEST
  token::token(size_t const offset, size_t const width, token_kind const k)
    : start{ offset, 1, offset + 1 }
    , end{ offset + width, 1, offset + width + 1 }
    , kind{ k }
  {
  }

  token::token(size_t const offset, size_t const width, token_kind const k, native_integer const d)
    : start{ offset, 1, offset + 1 }
    , end{ offset + width, 1, offset + width + 1 }
    , kind{ k }
    , data{ d }
  {
  }

  token::token(size_t const offset, size_t const width, token_kind const k, native_real const d)
    : start{ offset, 1, offset + 1 }
    , end{ offset + width, 1, offset + width + 1 }
    , kind{ k }
    , data{ d }
  {
  }

  token::token(size_t const offset,
               size_t const width,
               token_kind const k,
               native_persistent_string_view const d)
    : start{ offset, 1, offset + 1 }
    , end{ offset + width, 1, offset + width + 1 }
    , kind{ k }
    , data{ d }
  {
  }

  token::token(size_t const offset, size_t const width, token_kind const k, char const * const d)
    : start{ offset, 1, offset + 1 }
    , end{ offset + width, 1, offset + width + 1 }
    , kind{ k }
    , data{ native_persistent_string_view{ d } }
  {
  }

  token::token(size_t const offset, size_t const width, token_kind const k, native_bool const d)
    : start{ offset, 1, offset + 1 }
    , end{ offset + width, 1, offset + width + 1 }
    , kind{ k }
    , data{ d }
  {
  }

  token::token(size_t const offset, size_t const width, token_kind const k, ratio const d)
    : start{ offset, 1, offset + 1 }
    , end{ offset + width, 1, offset + width + 1 }
    , kind{ k }
    , data{ d }
  {
  }
#endif

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
    if(start.offset == token::ignore_pos || rhs.start.offset == token::ignore_pos)
    {
      return kind == rhs.kind && data == rhs.data;
    }
    else
    {
      return start == rhs.start && end == rhs.end && kind == rhs.kind && data == rhs.data;
    }
  }

  native_bool token::operator!=(token const &rhs) const
  {
    return !(*this == rhs);
  }

  std::ostream &operator<<(std::ostream &os, movable_position const &p)
  {
    return os << "movable_position(" << p.offset << ", " << p.line << ", " << p.col << ")";
  }

  std::ostream &operator<<(std::ostream &os, token const &t)
  {
    return os << "token(" << t.start << ", " << t.end << ", " << token_kind_str(t.kind) << ", "
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
    : pos{ .proc = this }
    , file{ f }
  {
  }

  movable_position &movable_position::operator++()
  {
    assert(offset < proc->file.size());

    if(proc->file[offset] == '\n')
    {
      col = 1;
      ++line;
    }
    else
    {
      ++col;
    }

    ++offset;
    return *this;
  }

  movable_position movable_position::operator++(int)
  {
    movable_position ret{ *this };
    ++(*this);
    return ret;
  }

  movable_position &movable_position::operator+=(size_t const count)
  {
    for(size_t i{}; i < count; ++i)
    {
      ++(*this);
    }
    return *this;
  }

  native_bool movable_position::operator==(movable_position const &rhs) const
  {
    return offset == rhs.offset;
  }

  native_bool movable_position::operator!=(movable_position const &rhs) const
  {
    return offset != rhs.offset;
  }

  movable_position movable_position::operator+(size_t const count) const
  {
    movable_position ret{ *this };
    for(size_t i{}; i < count; ++i)
    {
      ++ret;
    }
    return ret;
  }

  movable_position::operator size_t() const
  {
    return offset;
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

  option<error_ptr> processor::check_whitespace(native_bool const found_space)
  {
    if(require_space && !found_space)
    {
      require_space = false;
      return error::lex_expecting_whitespace(pos);
    }
    return none;
  }

  static result<codepoint, error_ptr>
  convert_to_codepoint(native_persistent_string_view const sv, movable_position const &pos)
  {
    std::mbstate_t state{};
    wchar_t wc{};
    auto const len{ std::mbrtowc(&wc, sv.data(), sv.size(), &state) };

    if(len == static_cast<size_t>(-1))
    {
      return error::lex_invalid_unicode("Unfinished character", pos);
    }
    else if(len == static_cast<size_t>(-2))
    {
      return error::lex_invalid_unicode("Invalid Unicode character", pos);
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
      || c == '^' || c == '\\' || c == '`' || c == '~' || c == ',' || c == ';';
  }

  static native_bool is_symbol_char(char32_t const c)
  {
    return !std::iswspace(static_cast<wint_t>(c)) && !is_special_char(c)
      && (std::iswalnum(static_cast<wint_t>(c)) != 0 || c == '_' || c == '-' || c == '/' || c == '?'
          || c == '!' || c == '+' || c == '*' || c == '=' || c == '.' || c == '&' || c == '<'
          || c == '>' || c == '#' || c == '%' || is_utf8_char(c));
  }

  static native_bool is_lower_letter(char32_t const c)
  {
    return c >= 'a' && c <= 'z';
  }

  static native_bool is_upper_letter(char32_t const c)
  {
    return c >= 'A' && c <= 'Z';
  }

  static native_bool is_letter(char32_t const c)
  {
    return is_lower_letter(c) || is_upper_letter(c);
  }

  static native_bool is_valid_num_char(char32_t const c, native_integer const radix)
  {
    if(c == '-' || c == '+' || c == '.')
    {
      return radix == 10;
    }
    if(radix == 10 && (c == 'e' || c == 'E'))
    {
      return true;
    }
    if(radix <= 10)
    {
      return c >= '0' && c < '0' + radix;
    }
    if(is_upper_letter(c))
    {
      return c < 'A' + radix - 10;
    }
    if(is_lower_letter(c))
    {
      return c < 'a' + radix - 10;
    }
    return c >= '0' && c <= '9';
  }

  result<token, error_ptr> processor::next()
  {
    /* Skip whitespace. */
    native_bool found_space{};
    while(true)
    {
      if(pos.offset >= file.size())
      {
        return ok(token{ pos, token_kind::eof });
      }

      if(auto const c(file[pos.offset]); std::isspace(c) == 0 && c != ',')
      {
        break;
      }

      found_space = true;
      ++pos;
    }

    /* Whether or not we've found the r in radix-specific integers such as 2r01010. */
    native_bool found_r{};
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
        return ok(token{ token_start, ++pos, token_kind::open_paren });
      case ')':
        require_space = false;
        return ok(token{ token_start, ++pos, token_kind::close_paren });
      case '[':
        require_space = false;
        return ok(token{ token_start, ++pos, token_kind::open_square_bracket });
      case ']':
        require_space = false;
        return ok(token{ token_start, ++pos, token_kind::close_square_bracket });
      case '{':
        require_space = false;
        return ok(token{ token_start, ++pos, token_kind::open_curly_bracket });
      case '}':
        require_space = false;
        return ok(token{ token_start, ++pos, token_kind::close_curly_bracket });
      case '\'':
        require_space = false;
        return ok(token{ token_start, ++pos, token_kind::single_quote });
      case '\\':
        {
          require_space = false;

          auto const ch(peek());
          ++pos;

          if(ch.is_err())
          {
            return ch.expect_err();
          }
          else if(std::iswspace(static_cast<int>(ch.expect_ok().character)))
          {
            return error::lex_incomplete_character("A \\ must be followed by a character value",
                                                   { token_start, pos });
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
          return ok(token{ token_start, pos, token_kind::character, data });
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
            return ok(token{ token_start, ++pos, token_kind::comment, ""sv });
          }
          else
          {
            ++pos;
            native_persistent_string_view const comment{ file.data() + token_start + leading_semis,
                                                         pos - token_start - leading_semis };
            return ok(token{ token_start, pos, token_kind::comment, comment });
          }
        }
        /* Numbers. */
      case '-':
      case '0' ... '9':
        {
          if(auto &&e(check_whitespace(found_space)); e.is_some())
          {
            return err(std::move(e.unwrap()));
          }

          native_bool contains_leading_digit{ file[token_start] != '-' };
          native_bool contains_dot{};
          native_bool is_scientific{};
          native_bool found_exponent_sign{};
          native_bool expecting_exponent{};
          native_bool expecting_more_digits{};
          int radix{ 10 };
          auto r_pos{ pos }; /* records the 'r' position if one is found */
          native_bool found_beginning_negative{};

          if(auto const [c, l]{ peek().unwrap_or({ ' ', 1 }) };
             file[token_start] == '-' && c == '0' && !found_slash_after_number)
          {
            contains_leading_digit = true;
            if(auto const [f, l]{ peek(2).unwrap_or({ ' ', 1 }) }; f != ' ')
            {
              if(f != 'x' && f != 'X')
              {
                radix = 8;
                ++pos;
              }
              else if(f == 'x' || f == 'X')
              {
                radix = 16;
                pos += 2;
              }
            }
          }
          else if(file[token_start] == '0' && !found_slash_after_number)
          {
            contains_leading_digit = true;
            if(auto const [f, l]{ peek().unwrap_or({ ' ', 1 }) }; f != ' ')
            {
              if(f != 'x' && f != 'X')
              {
                radix = 8;
              }
              if(f == 'x' || f == 'X')
              {
                radix = 16;
                ++pos;
              }
            }
          }
          if(radix == 8 || radix == 16)
          {
            expecting_more_digits = true;
          }

          while(true)
          {
            auto const result(peek());
            if(result.is_err())
            {
              break;
            }
            if(auto const [c, len](result.unwrap_or(codepoint{ ' ', 1 })); c == '.')
            {
              if(contains_dot || is_scientific || !contains_leading_digit)
              {
                ++pos;
                return error::lex_invalid_number("Unexpected '.' found in number",
                                                 { token_start, pos },
                                                 error::note{ "Found the '.' here", pos });
              }
              if(found_r)
              {
                ++pos;
                return error::lex_invalid_number(
                  "An arbitrary radix number can only be an integer, so it may not contain a '.'",
                  { token_start, pos },
                  error::note{ "Found a '.' here", pos });
              }
              contains_dot = true;
              if(radix != 10 && radix != 8)
              {
                ++pos;
                continue;
              }
              radix = 10; /* numbers like 02.3 should be parsed as decimal numbers. */
            }
            else if(c == 'e' || c == 'E')
            {
              if(found_r)
              {
                ++pos;
                continue;
              }
              if(radix < 15)
              {
                /* Numbers containing 'e' and radix < 15, then it must be a decimal number. */
                radix = 10;
                if(is_scientific || !contains_leading_digit)
                {
                  ++pos;
                  return error::lex_invalid_number("Extraneous 'e' found in number",
                                                   { token_start, pos },
                                                   error::note{ "Found 'e' here", pos });
                }
                if(found_slash_after_number)
                {
                  ++pos;
                  found_slash_after_number = false;
                  return error::lex_invalid_ratio("Ratio cannot have scientific notation",
                                                  { token_start, pos },
                                                  error::note{ "Found 'e' here", pos });
                }
                is_scientific = true;
                expecting_exponent = true;
              }
            }
            else if(c == '+' || c == '-')
            {
              if(radix == 10 && !found_r)
              {
                if(found_exponent_sign || !is_scientific || !expecting_exponent)
                {
                  ++pos;
                  return error::lex_invalid_number(
                    fmt::format("Unexpected '{}' in number", static_cast<char>(c)),
                    { token_start, pos },
                    error::note{ fmt::format("Found '{}' here", static_cast<char>(c)), pos });
                }
                found_exponent_sign = true;
              }
            }
            else if(c == 'r' || c == 'R')
            {
              ++pos;
              if(found_r)
              {
                continue;
              }
              found_r = true;
              expecting_more_digits = true;
              r_pos = pos;
              if(found_slash_after_number || contains_dot || found_exponent_sign
                 || expecting_exponent)
              {
                /* TODO: Add a note for why we're determining this isn't an integer. */
                return error::lex_invalid_number("Arbitrary radix numbers can only be integers",
                                                 { token_start, pos });
              }
              continue;
            }
            else if(c == '/')
            {
              require_space = false;
              ++pos;
              if(found_exponent_sign || is_scientific || expecting_exponent || contains_dot
                 || found_slash_after_number || (radix != 10 && radix != 8))
              {
                /* TODO: Add a note for why it's not an integer. */
                return error::lex_invalid_ratio("A ratio numerator must be an integer",
                                                { token_start, pos });
              }
              found_slash_after_number = true;
              /* Skip the '/' char and look for the denominator number. */
              ++pos;
              auto const denominator(next());
              found_slash_after_number = false;
              if(denominator.is_ok())
              {
                if(denominator.expect_ok().kind != token_kind::integer)
                {
                  return error::lex_invalid_ratio(
                    "A ratio denominator must be an integer",
                    {
                      token_start,
                      pos
                  },
                    error::note{ "Denominator is here",
                                 { denominator.expect_ok().start, denominator.expect_ok().end } });
                }
                auto const &denominator_token(denominator.expect_ok());
                return ok(
                  token(token_start,
                        pos,
                        token_kind::ratio,
                        { .numerator = std::strtoll(file.data() + token_start, nullptr, 10),
                          .denominator = boost::get<native_integer>(denominator_token.data) }));
              }
              return denominator.expect_err();
            }
            else if(std::iswdigit(static_cast<wint_t>(c)) == 0)
            {
              if(expecting_exponent)
              {
                ++pos;
                return error::lex_invalid_number("Missing exponent from end of number",
                                                 { token_start, pos });
              }
              if(contains_dot)
              {
                /* If we have a dot, then we are parsing a decimal real number. */
                break;
              }
              if(!contains_leading_digit)
              {
                /* If we don't have a leading digit, then we are parsing a symbol. */
                break;
              }
              /* When parsing decimal numbers only, we would break if we see a non-digit char. */
              /* But to support other kinds of numbers (octal, hex, etc.), we only break if
               * c is also not a letter. */
              if(!is_letter(c))
              {
                if(found_r && expecting_more_digits)
                {
                  ++pos;
                  return error::lex_invalid_number("Unexpected end of integer",
                                                   { token_start, pos });
                }
                if(pos == token_start && file[token_start] == '0')
                {
                  /* handle the case of a single digit 0 */
                  radix = 10;
                  expecting_more_digits = false;
                }
                break;
              }
            }
            else if(expecting_exponent)
            {
              expecting_exponent = false;
            }
            contains_leading_digit = true;
            expecting_more_digits = false;
            ++pos;
          }

          /* TODO: This looks unreachable. */
          if(expecting_exponent)
          {
            ++pos;
            return error::lex_invalid_number("Unexpected end of decimal number",
                                             { token_start, pos });
          }

          if(expecting_more_digits)
          {
            ++pos;
            if(found_r)
            {
              return error::lex_invalid_number("Unexpected end of integer", { token_start, pos });
            }
            return error::lex_invalid_number(fmt::format("Unexpected end of base {} number", radix),
                                             { token_start, pos });
          }
          /* Tokens beginning with - are ambiguous; it's a negative number only if
           * followed by a number. Otherwise, it's a symbol. */
          /* TODO: Handle numbers starting with `+` */
          if(file[token_start] != '-' || (pos - token_start) >= 1)
          {
            require_space = true;
            ++pos;
            auto number_start{ token_start.offset };
            if(file[token_start] == '-' || file[token_start] == '+')
            {
              number_start = token_start + static_cast<size_t>(1);
              found_beginning_negative = file[token_start] == '-';
            }
            if(found_r)
            {
              radix = std::stoi(file.data() + token_start, nullptr);
              if(radix < 0)
              {
                radix = -radix;
                found_beginning_negative = true;
              }
              if(radix < 2 || radix > 36)
              {
                return error::lex_invalid_number(
                  fmt::format(
                    "Base {} is out of range. The supported bases are 2 through 36, inclusive.",
                    radix),
                  { token_start, pos });
              }
              number_start = r_pos + static_cast<size_t>(1);
            }
            else if(radix == 16)
            {
              number_start += 2;
            }

            /* Check for invalid digits. */
            native_vector<char> invalid_digits{};
            for(auto i{ number_start }; i < pos; i++)
            {
              if(!is_valid_num_char(file[i], radix))
              {
                invalid_digits.emplace_back(file[i]);
              }
            }
            if(!invalid_digits.empty())
            {
              return error::lex_invalid_number(
                fmt::format("Characters '{}' are invalid for a base {} number",
                            std::string(invalid_digits.begin(), invalid_digits.end()),
                            radix),
                { token_start, pos });
            }
            /* Real numbers. */
            if(contains_dot || is_scientific || found_exponent_sign)
            {
              return ok(token{ token_start,
                               pos,
                               token_kind::real,
                               std::strtod(file.data() + token_start, nullptr) });
            }

            /* Integers */
            errno = 0;
            auto const parsed_int{ std::strtoll(file.data() + number_start, nullptr, radix)
                                   * (found_beginning_negative ? -1 : 1) };
            if(errno == ERANGE)
            {
              static constexpr auto const max(std::numeric_limits<native_integer>::max());
              /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
              auto const max_width{ static_cast<size_t>(snprintf(nullptr, 0, "%lld", max)) };
              return error::lex_invalid_number(
                "Number is too large to be stored",
                {
                  token_start,
                  pos
              },
                error::note{ "I can fit about this much",
                             { token_start, token_start + max_width } });
            }
            return ok(token{ token_start, pos, token_kind::integer, parsed_int });
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
      case '.':
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
            return error::lex_invalid_symbol("A symbol may not start with a '/'",
                                             { token_start, pos });
          }
          else if(name == "nil")
          {
            return ok(token{ token_start, pos, token_kind::nil });
          }
          else if(name == "true")
          {
            return ok(token{ token_start, pos, token_kind::boolean, true });
          }
          else if(name == "false")
          {
            return ok(token{ token_start, pos, token_kind::boolean, false });
          }
          return ok(token{ token_start, pos, token_kind::symbol, name });
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
          if(oc.is_err() || std::iswspace(static_cast<wint_t>(c)) || is_special_char(c))
          {
            ++pos;
            return error::lex_invalid_keyword("A keyword must contain a valid symbol after the ':'",
                                              { token_start, pos });
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
            return error::lex_invalid_keyword("A keyword may not start with ':/'",
                                              { token_start, pos });
          }
          else if(name[0] == ':' && name[1] == ':')
          {
            return error::lex_invalid_keyword("Too many ':' for a valid keyword",
                                              { token_start, pos },
                                              "Only one or two ':' is valid");
          }

          return ok(token{ token_start, pos, token_kind::keyword, name });
        }
        /* Strings. */
      case '"':
        {
          if(auto &&e(check_whitespace(found_space)); e.is_some())
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
              return error::lex_unterminated_string({ token_start, pos });
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
                  util::string_builder sb;
                  return error::lex_invalid_string_escape(
                    fmt::format("Unsupported string escape character '{}'",
                                sb(oc.expect_ok().character).view()),
                    { pos, pos + 2zu });
              }
              escaped = false;
            }
            else if(oc.expect_ok().character == '\\')
            {
              escaped = contains_escape = true;
            }
            pos += oc.expect_ok().len;
          }
          require_space = true;
          ++pos;

          /* Unescaped strings can be read right from memory, but escaped strings require
           * some processing first, to turn the escape sequences into the necessary characters.
           * We use distinct token types for these so we can optimize for the typical case. */
          auto const kind(contains_escape ? token_kind::escaped_string : token_kind::string);
          return ok(token{
            token_start,
            pos,
            kind,
            native_persistent_string_view(file.data() + token_start + 1, pos - token_start - 2) });
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

          return ok(token{ token_start, pos, token_kind::meta_hint });
        }
        /* Reader macros. */
      case '#':
        {
          if(auto &&e(check_whitespace(found_space)); e.is_some())
          {
            return err(std::move(e.unwrap()));
          }
          require_space = false;
          auto const [character, len](peek().unwrap_or(codepoint{ ' ', 1 }));
          pos += len;

          switch(character)
          {
            case '_':
              ++pos;
              return ok(token{ token_start, pos, token_kind::reader_macro_comment });
            case '?':
              {
                auto const maybe_splice(peek());

                auto const [character, len](maybe_splice.unwrap_or(codepoint{ ' ', 1 }));
                pos += len;

                if(character == '@')
                {
                  ++pos;
                  return ok(token{ token_start, pos, token_kind::reader_macro_conditional_splice });
                }
                else
                {
                  return ok(token{ token_start, pos, token_kind::reader_macro_conditional });
                }
              }
            case '!':
              {
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

                  ++pos;
                }

                ++pos;
                if(pos == token_start + 2zu)
                {
                  return ok(token{ token_start, pos, token_kind::comment, ""sv });
                }
                else
                {
                  auto const length{ pos - token_start - 2 };
                  native_persistent_string_view const comment{ file.data() + token_start + 2,
                                                               length };
                  return ok(token{ token_start, pos, token_kind::comment, comment });
                }
              }
            default:
              break;
          }
          return ok(token{ token_start, pos, token_kind::reader_macro });
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

          return ok(token{ token_start, pos, token_kind::syntax_quote });
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
          auto const result(peek());
          auto const [character, len](result.unwrap_or(codepoint{ ' ', 1 }));
          pos += len;
          switch(character)
          {
            case '@':
              {
                ++pos;
                return ok(token{ token_start, pos, token_kind::unquote_splice });
              }
            default:
              return ok(token{ token_start, pos, token_kind::unquote });
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

          return ok(token{ token_start, pos, token_kind::deref });
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
            auto const result(convert_to_codepoint(file.substr(pos), pos));
            if(result.is_err())
            {
              break;
            }

            if(!is_symbol_char(result.expect_ok().character))
            {
              break;
            }
            pos += result.expect_ok().len;
          }
          require_space = true;
          native_persistent_string_view const name{ file.data() + token_start, pos - token_start };

          return ok(token{ token_start, pos, token_kind::symbol, name });
        }
        ++pos;
        return error::lex_unexpected_character(
          fmt::format("Unexpected character '{}'", file[token_start]),
          { token_start, pos });
    }
  }

  result<codepoint, error_ptr> processor::peek(size_t const ahead) const
  {
    auto const peek_pos{ pos + ahead };
    if(peek_pos >= file.size())
    {
      return error::lex_unexpected_eof(pos);
    }
    auto const oc{ convert_to_codepoint(file.substr(peek_pos), peek_pos) };
    return oc;
  }
}
