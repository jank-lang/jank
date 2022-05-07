#include <iostream>
#include <iomanip>

#include <magic_enum.hpp>

#include <jank/read/lex.hpp>

namespace jank::read::lex
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
  std::ostream& operator <<(std::ostream &os, error const &e)
  { return os << "error(" << e.pos << ", " << std::quoted(e.message) << ")"; }

  token_iterator::value_type token_iterator::operator *() const
  {
    if(!latest)
    /* TODO: panic */
    { std::abort(); }
    return *latest;
  }
  token_iterator& token_iterator::operator ++()
  {
    latest = some(p.next());
    return *this;
  }
  bool token_iterator::operator !=(token_iterator const &rhs) const
  { return latest != rhs.latest; }

  token_iterator processor::begin()
  { return token_iterator{ some(next()), *this }; }
  token_iterator processor::end()
  { return token_iterator{ some(token_kind::eof), *this }; }

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
      { return ok(token{ token_kind::eof }); }

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
        ++pos;
        return ok(token{ token_kind::open_paren });
      case ')':
        ++pos;
        return ok(token{ token_kind::close_paren });
      case '[':
        ++pos;
        return ok(token{ token_kind::open_square_bracket });
      case ']':
        ++pos;
        return ok(token{ token_kind::close_square_bracket });
      case '{':
        ++pos;
        return ok(token{ token_kind::open_curly_bracket });
      case '}':
        ++pos;
        return ok(token{ token_kind::close_curly_bracket });
      /* Numbers. */
      case '0' ... '9':
      {
        if(auto &&e = check_whitespace(found_space))
        { return err(std::move(*e)); }
        while(true)
        {
          auto const oc(peek());
          if(!oc || std::isdigit(*oc) == 0)
          { break; }
          ++pos;
        }
        ++pos;
        require_space = true;
        return ok(token(token_kind::integer, std::strtoll(file.data() + token_start, nullptr, 10)));
      }
      /* Symbols. */
      case 'a' ... 'z':
      case 'A' ... 'Z':
      case '_':
      case '-':
      case '/':
      case '?':
      {
        if(auto &&e = check_whitespace(found_space))
        { return err(std::move(*e)); }
        while(true)
        {
          auto const oc(peek());
          /* TODO: Lift this to a separate fn, since it's used twice. */
          if(!oc || (std::isalnum(static_cast<unsigned char>(*oc)) == 0 && *oc != '_' && *oc != '-' && *oc != '/' && *oc != '?'))
          { break; }
          ++pos;
        }
        ++pos;
        require_space = true;
        return ok(token(token_kind::symbol, std::string_view(file.data() + token_start, pos - token_start)));
      }
      /* Keywords. */
      case ':':
      {
        if(auto &&e = check_whitespace(found_space))
        { return err(std::move(*e)); }
        while(true)
        {
          auto const oc(peek());
          if(!oc || (std::isalnum(static_cast<unsigned char>(*oc)) == 0 && *oc != '_' && *oc != '-' && *oc != '/' && *oc != '?'))
          { break; }
          ++pos;
        }
        ++pos;
        require_space = true;
        /* TODO: Ensure it's not empty and uses / properly. */
        return ok(token(token_kind::keyword, std::string_view(file.data() + token_start + 1, pos - token_start - 1)));
      }
      /* Strings. */
      case '"':
      {
        if(auto &&e = check_whitespace(found_space))
        { return err(std::move(*e)); }
        bool escaped{};
        while(true)
        {
          auto const oc(peek());
          if(!oc)
          { break; }
          else if(!escaped && *oc == '"')
          {
            ++pos;
            break;
          }

          if(escaped)
          {
            switch(*oc)
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
          else if(*oc == '\\')
          { escaped = true; }
          ++pos;
        }
        ++pos;
        require_space = true;
        return ok(token(token_kind::string, std::string_view(file.data() + token_start + 1, pos - token_start - 2)));
      }
      default:
        ++pos;
        return err(error{ token_start, std::string{ "unexpected character: " } + file[token_start] });
    }

    return err(error{});
  }

  option<char> processor::peek() const
  {
    auto const next_pos(pos + 1);
    if(next_pos >= file.size())
    { return none; }
    return some(file[next_pos]);
  }
}
