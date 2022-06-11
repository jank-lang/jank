#include <iostream>
#include <iomanip>

#include <magic_enum.hpp>

#include <jank/read/lex.hpp>

namespace jank::read
{
  std::ostream& operator <<(std::ostream &os, error const &e)
  { return os << "error(" << e.start << " - " << e.end << ", " << std::quoted(e.message) << ")"; }

  namespace lex
  {
    template <typename ... Ts>
    std::ostream& operator<<(std::ostream &os, std::variant<Ts...> const &v)
    {
      std::visit
      (
       [&](auto &&arg)
       {
         using T = std::decay_t<decltype(arg)>;
         if constexpr(std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>)
         { os << std::quoted(arg); }
         else
         { os << arg; }
       },
       v
      );
      return os;
    }

    bool token::no_data::operator ==(no_data const &) const
    { return true; }
    bool token::no_data::operator !=(no_data const &) const
    { return false; }
    bool token::operator ==(token const &rhs) const
    { return kind == rhs.kind && data == rhs.data; }
    bool token::operator !=(token const &rhs) const
    { return kind != rhs.kind || data != rhs.data; }
    std::ostream& operator <<(std::ostream &os, token const &t)
    { return os << "token(" << magic_enum::enum_name(t.kind) << ", " << t.data << ")"; }
    std::ostream& operator <<(std::ostream &os, token::no_data const &)
    { return os << "<no data>"; }

    processor::iterator::value_type processor::iterator::operator *() const
    { return latest.unwrap(); }
    processor::iterator& processor::iterator::operator ++()
    {
      latest = some(p.next());
      return *this;
    }
    bool processor::iterator::operator !=(processor::iterator const &rhs) const
    { return latest != rhs.latest; }
    bool processor::iterator::operator ==(processor::iterator const &rhs) const
    { return latest == rhs.latest; }

    processor::iterator processor::begin()
    { return { some(next()), *this }; }
    processor::iterator processor::end()
    { return { some(token_kind::eof), *this }; }

    option<error> processor::check_whitespace(bool const found_space)
    {
      if(require_space && !found_space)
      {
        require_space = false;
        return some(error{ pos, "expected whitespace before next token" });
      }
      return none;
    }

    result<token, error> processor::next()
    {
      /* Skip whitespace. */
      bool found_space{};
      while(true)
      {
        if(pos >= file.size())
        { return ok(token{ pos, token_kind::eof }); }

        auto const c(file[pos]);
        if(std::isspace(c) == 0 && c != ',')
        { break; }

        found_space = true;
        ++pos;
      }

      auto const token_start(pos);
      switch(file[token_start])
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
        /* Numbers. */
        case '0' ... '9':
        {
          auto &&e = check_whitespace(found_space);
          if(e.is_some())
          { return err(std::move(e.unwrap())); }
          while(true)
          {
            auto const oc(peek());
            if(oc.is_none() || std::isdigit(oc.unwrap()) == 0)
            { break; }
            ++pos;
          }
          require_space = true;
          return ok(token{ pos++, token_kind::integer, std::strtoll(file.data() + token_start, nullptr, 10) });
        }
        /* Symbols. */
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_':
        case '-':
        case '/':
        case '?':
        case '+':
        {
          auto &&e = check_whitespace(found_space);
          if(e.is_some())
          { return err(std::move(e.unwrap())); }
          while(true)
          {
            auto const oc(peek());
            if(oc.is_none())
            { break; }
            auto const c(oc.unwrap());
            /* TODO: Lift this to a separate fn, since it's used twice. */
            if(std::isalnum(static_cast<unsigned char>(c)) == 0 && c != '_' && c != '-' && c != '/' && c != '?' && c != '+' && c != '.')
            { break; }
            ++pos;
          }
          require_space = true;
          /* TODO: Ensure it uses / correctly. */
          return ok(token{ pos++, token_kind::symbol, std::string_view(file.data() + token_start, pos - token_start) });
        }
        /* Keywords. */
        case ':':
        /* TODO: Support for ::foo */
        {
          auto &&e = check_whitespace(found_space);
          if(e.is_some())
          { return err(std::move(e.unwrap())); }
          while(true)
          {
            auto const oc(peek());
            if(oc.is_none())
            { break; }
            auto const c(oc.unwrap());
            /* TODO: Lift this to a separate fn, since it's used twice. */
            if(std::isalnum(static_cast<unsigned char>(c)) == 0 && c != '_' && c != '-' && c != '/' && c != '?' && c != '+' && c != '.')
            { break; }
            ++pos;
          }
          require_space = true;
          /* TODO: Ensure it's not empty and uses / properly. */
          return ok(token{ pos++, token_kind::keyword, std::string_view(file.data() + token_start + 1, pos - token_start - 1) });
        }
        /* Strings. */
        case '"':
        {
          auto &&e = check_whitespace(found_space);
          if(e.is_some())
          { return err(std::move(e.unwrap())); }
          auto const token_start(pos);
          bool escaped{};
          while(true)
          {
            auto const oc(peek());
            if(oc.is_none())
            {
              ++pos;
              return err(error{ token_start, "unterminated string" });
            }
            else if(!escaped && oc.unwrap() == '"')
            {
              ++pos;
              break;
            }

            if(escaped)
            {
              switch(oc.unwrap())
              {
                case 'n':
                case 't':
                case '"':
                  break;
                default:
                  return err(error{ pos, "unsupported escape character" });
              }
              escaped = false;
            }
            else if(oc.unwrap() == '\\')
            { escaped = true; }
            ++pos;
          }
          require_space = true;
          return ok(token{ pos++, token_kind::string, std::string_view(file.data() + token_start + 1, pos - token_start - 2) });
        }
        default:
          ++pos;
          return err(error{ token_start, std::string{ "unexpected character: " } + file[token_start] });
      }
    }

    option<char> processor::peek() const
    {
      auto const next_pos(pos + 1);
      if(next_pos >= file.size())
      { return none; }
      return some(file[next_pos]);
    }
  }
}
