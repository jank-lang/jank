#include <fmt/format.h>
#include <fmt/ostream.h>

#include <ftxui/dom/elements.hpp>

#include <jank/read/lex.hpp>

namespace jank::ui
{
  using namespace ftxui;

  static std::set<native_persistent_string_view> const specials{
    "def", "fn*",   "fn",  "let*", "let",   "loop*",   "loop",  "do",
    "if",  "quote", "var", "try",  "catch", "finally", "throw",
  };

  /* TODO: Also support core fns? */

  Element symbol_color(Element const &e, native_persistent_string_view const &sym)
  {
    if(specials.find(sym) != specials.end())
    {
      return e | color(Color::CyanLight) | bold;
    }

    return e | color(Color::Default);
  }

  Element token_color(Element const &e, read::lex::token const &token)
  {
    switch(token.kind)
    {
      case read::lex::token_kind::open_paren:
      case read::lex::token_kind::close_paren:
      case read::lex::token_kind::open_square_bracket:
      case read::lex::token_kind::close_square_bracket:
      case read::lex::token_kind::open_curly_bracket:
      case read::lex::token_kind::close_curly_bracket:
      case read::lex::token_kind::single_quote:
      case read::lex::token_kind::meta_hint:
      case read::lex::token_kind::reader_macro:
      case read::lex::token_kind::reader_macro_comment:
      case read::lex::token_kind::reader_macro_conditional:
      case read::lex::token_kind::reader_macro_conditional_splice:
      case read::lex::token_kind::syntax_quote:
      case read::lex::token_kind::unquote:
      case read::lex::token_kind::unquote_splice:
      case read::lex::token_kind::deref:
      case read::lex::token_kind::nil:
        return e | color(Color::DarkOrange);
      case read::lex::token_kind::keyword:
        return e | color(Color::BlueLight);
      case read::lex::token_kind::comment:
        return e | color(Color::GrayLight);
      case read::lex::token_kind::integer:
      case read::lex::token_kind::real:
      case read::lex::token_kind::ratio:
      case read::lex::token_kind::boolean:
      case read::lex::token_kind::character:
        return e | color(Color::MagentaLight);
      case read::lex::token_kind::string:
      case read::lex::token_kind::escaped_string:
        return e | color(Color::GreenLight);
      case read::lex::token_kind::symbol:
        return symbol_color(e, boost::get<native_persistent_string_view>(token.data));
      case read::lex::token_kind::eof:
        return e | color(Color::Default);
        break;
    }
  }

  /* TODO: Center horizontally if the line is too long. */
  Element
  highlight(native_persistent_string const &code, size_t const line_start, size_t const line_end)
  {
    read::lex::processor l_prc{ code };
    size_t last_offset{}, last_line{ 1 };
    std::vector<Element> lines;
    std::vector<Element> current_line;

    for(auto it(l_prc.begin()); it != l_prc.end(); ++it)
    {
      /* TODO: Handle lex errors by showing the rest of the lines without highlighting. */

      auto const &token(it.latest.unwrap().expect_ok());
      if(token.start.line > line_end)
      {
        break;
      }

      auto const token_size(std::max(token.end.offset - token.start.offset, 1zu));
      auto const skip(token.start.line < line_start);
      std::string_view space{ code.data() + last_offset, token.start.offset - last_offset };
      size_t last_newline{};
      for(auto it(space.find('\n')); it != decltype(space)::npos; it = space.find('\n', it + 1))
      {
        /* We add a space since this could be an empty line and ftxui will only make it take
         * space if it's non-empty. */
        if(!skip && last_line >= line_start)
        {
          current_line.emplace_back(text(" "));
          //current_line.emplace_back(text("<empty>"));
          lines.emplace_back(hbox(std::move(current_line)));
          current_line.clear();
        }
        last_newline = it + 1;
        ++last_line;
      }
      //fmt::println("space '{}' size {} last_newline {}", space, space.size(), last_newline);
      if(!skip && last_newline < space.size())
      {
        current_line.emplace_back(text(std::string{ space.substr(last_newline) }));
        //current_line.emplace_back(
        //  text(fmt::format("|{}|", std::string{ space.substr(last_newline) })));
      }

      if(!skip)
      {
        current_line.emplace_back(
          token_color(text(std::string{ code.data() + token.start.offset, token_size }), token));
        //current_line.emplace_back(token_color(
        //  text(fmt::format("'{}'", std::string{ code.data() + token.start.offset, token_size })),
        //  token));
      }
      last_offset = token.start.offset + token_size;
      //fmt::println("last_offset {}", last_offset);
    }

    lines.emplace_back(hbox(std::move(current_line)));

    return vbox(std::move(lines));
  }
}
