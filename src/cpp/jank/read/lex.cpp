#include <iostream>
#include <iomanip>

#include <magic_enum.hpp>

#include <jank/read/lex.hpp>

using namespace std::string_view_literals;

namespace jank::read
{
  error::error(size_t const s, std::string const &m)
    : start{ s }, end{ s }, message{ m }
  { }
  error::error(size_t const s, size_t const e, std::string const &m)
    : start{ s }, end{ e }, message{ m }
  { }
  error::error(std::string const &m)
    : message{ m }
  { }

  bool error::operator ==(error const &rhs) const
  { return !(*this != rhs); }
  bool error::operator !=(error const &rhs) const
  { return start != rhs.start || end != rhs.end || message != rhs.message; }

  std::ostream& operator <<(std::ostream &os, error const &e)
  { return os << "error(" << e.start << " - " << e.end << ", " << std::quoted(e.message) << ")"; }

  namespace lex
  {
    template <typename ... Ts>
    std::ostream& operator<<(std::ostream &os, std::variant<Ts...> const &v)
    {
      boost::apply_visitor
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

    token::token(token_kind const k) : kind{ k }
    { }
    token::token(size_t const p, token_kind const k) : pos{ p }, kind{ k }
    { }
    token::token(size_t const p, token_kind const k, runtime::detail::integer_type const d)
      : pos{ p }, kind{ k }, data{ d }
    { }
    token::token(size_t const p, token_kind const k, runtime::detail::real_type const d)
      : pos{ p }, kind{ k }, data{ d }
    { }
    token::token(size_t const p, token_kind const k, std::string_view const d)
      : pos{ p }, kind{ k }, data{ d }
    { }
    token::token(size_t const p, token_kind const k, bool const d)
      : pos{ p }, kind{ k }, data{ d }
    { }

    token::token(size_t const p, size_t const s, token_kind const k)
      : pos{ p }, size{ s }, kind{ k }
    { }
    token::token(size_t const p, size_t const s, token_kind const k, runtime::detail::integer_type const d)
      : pos{ p }, size{ s }, kind{ k }, data{ d }
    { }
    token::token(size_t const p, size_t const s, token_kind const k, runtime::detail::real_type const d)
      : pos{ p }, size{ s }, kind{ k }, data{ d }
    { }
    token::token(size_t const p, size_t const s, token_kind const k, std::string_view const d)
      : pos{ p }, size{ s }, kind{ k }, data{ d }
    { }
    token::token(size_t const p, size_t const s, token_kind const k, bool const d)
      : pos{ p }, size{ s }, kind{ k }, data{ d }
    { }

    bool token::no_data::operator ==(no_data const &) const
    { return true; }
    bool token::no_data::operator !=(no_data const &) const
    { return false; }
    bool token::operator ==(token const &rhs) const
    { return !(*this != rhs); }
    bool token::operator !=(token const &rhs) const
    { return (pos != rhs.pos && pos != token::ignore_pos && rhs.pos != token::ignore_pos) || size != rhs.size || kind != rhs.kind || data != rhs.data; }
    std::ostream& operator <<(std::ostream &os, token const &t)
    { return os << "token(" << t.pos << ", " << t.size << ", " << magic_enum::enum_name(t.kind) << ", " << t.data << ")"; }
    std::ostream& operator <<(std::ostream &os, token::no_data const &)
    { return os << "<no data>"; }

    processor::processor(std::string_view const &f) : file{ f }
    { }

    processor::iterator::value_type const& processor::iterator::operator *() const
    { return latest.unwrap(); }
    processor::iterator::value_type const* processor::iterator::operator ->() const
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
        case ';':
        {
          size_t leading_semis{ 1 };
          bool hit_non_semi{};
          while(true)
          {
            auto const oc(peek());
            if(oc.is_none())
            { break; }
            auto const c(oc.unwrap());
            if(c == '\n')
            { break; }
            else if(c == ';' && !hit_non_semi)
            { ++leading_semis; }
            else
            { hit_non_semi = true; }

            ++pos;
          }
          if(pos == token_start)
          { return ok(token{ pos++, 1, token_kind::comment, ""sv }); }
          else
          {
            ++pos;
            std::string_view const comment
            {
              file.data() + token_start + leading_semis,
              pos - token_start - leading_semis
            };
            return ok(token{ token_start, pos - 1 - token_start, token_kind::comment, comment });
          }
        }
        /* Numbers. */
        case '-':
        case '0' ... '9':
        {
          auto &&e(check_whitespace(found_space));
          if(e.is_some())
          { return err(std::move(e.unwrap())); }
          bool contains_leading_digit{ file[token_start] != '-' };
          bool contains_dot{};
          while(true)
          {
            auto const oc(peek());
            if(oc.is_none())
            { break; }

            auto const c(oc.unwrap());
            if(c == '.')
            {
              if(contains_dot || !contains_leading_digit)
              {
                ++pos;
                return err(error{ token_start, pos, "invalid number" });
              }
              contains_dot = true;
            }
            else if(std::isdigit(c) == 0)
            { break; }

            contains_leading_digit = true;

            ++pos;
          }

          /* Tokens beginning with - are ambiguous; it's only a negative number if it has numbers
           * to follow. */
          if(file[token_start] != '-' || (pos - token_start) >= 1)
          {
            require_space = true;
            ++pos;
            if(contains_dot)
            { return ok(token{ token_start, pos - token_start, token_kind::real, std::strtold(file.data() + token_start, nullptr) }); }
            else
            { return ok(token{ token_start, pos - token_start, token_kind::integer, std::strtoll(file.data() + token_start, nullptr, 10) }); }
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
        {
          auto &&e(check_whitespace(found_space));
          if(e.is_some())
          { return err(std::move(e.unwrap())); }
          while(true)
          {
            auto const oc(peek());
            if(oc.is_none())
            { break; }
            auto const c(oc.unwrap());
            /* TODO: Lift this to a separate fn, since it's used twice. */
            if(std::isalnum(static_cast<unsigned char>(c)) == 0 && c != '_' && c != '-' && c != '/' && c != '?' && c != '+' && c != '*' && c != '=' && c != '.')
            { break; }
            ++pos;
          }
          require_space = true;
          std::string_view const name{ file.data() + token_start, ++pos - token_start };
          if(name[0] == '/' && name.size() > 1)
          { return err(error{ token_start, "invalid symbol" }); }
          else if(name == "nil")
          { return ok(token{ token_start, pos - token_start, token_kind::nil }); }
          else if(name == "true")
          { return ok(token{ token_start, pos - token_start, token_kind::boolean, true }); }
          else if(name == "false")
          { return ok(token{ token_start, pos - token_start, token_kind::boolean, false }); }

          return ok(token{ token_start, pos - token_start, token_kind::symbol, name });
        }
        /* Keywords. */
        case ':':
        {
          auto &&e(check_whitespace(found_space));
          if(e.is_some())
          { return err(std::move(e.unwrap())); }

          /* Support auto-resolved qualified keywords. */
          auto const oc(peek());
          if(oc.is_some() && oc.unwrap() == ':')
          { ++pos; }

          while(true)
          {
            auto const oc(peek());
            if(oc.is_none())
            { break; }
            auto const c(oc.unwrap());
            /* TODO: Lift this to a separate fn, since it's used twice. */
            if(std::isalnum(static_cast<unsigned char>(c)) == 0 && c != '_' && c != '-' && c != '/' && c != '?' && c != '+' && c != '*' && c != '=' && c != '.')
            { break; }
            ++pos;
          }
          require_space = true;
          std::string_view const name{ file.data() + token_start + 1, ++pos - token_start - 1 };
          if(name[0] == '/' && name.size() > 1)
          { return err(error{ token_start, "invalid keyword: starts with /" }); }
          else if(name[0] == ':' && name.size() == 1)
          { return err(error{ token_start, "invalid keyword: incorrect number of :" }); }

          return ok(token{ token_start, pos - token_start, token_kind::keyword, name });
        }
        /* Strings. */
        case '"':
        {
          auto &&e(check_whitespace(found_space));
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
          pos++;
          return ok(token{ token_start, pos - token_start, token_kind::string, std::string_view(file.data() + token_start + 1, pos - token_start - 2) });
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
