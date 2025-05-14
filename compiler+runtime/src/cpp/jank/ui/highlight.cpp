#include <ftxui/dom/elements.hpp>

#include <jank/ui/highlight.hpp>
#include <jank/read/lex.hpp>
#include <jank/runtime/module/loader.hpp>

namespace jank::ui
{
  using namespace ftxui;

  /* TODO: Also support core fns? */
  static std::set<native_persistent_string_view> const specials{
    "def", "fn*",   "fn",  "let*", "let",   "loop*",   "loop",  "do",
    "if",  "quote", "var", "try",  "catch", "finally", "throw", "letfn*",
  };

  static Element symbol_color(Element const &e, native_persistent_string_view const &sym)
  {
    if(specials.contains(sym))
    {
      return e | color(Color::CyanLight) | bold;
    }

    return e | color(Color::Default);
  }

  static Element token_color(Element const &e, read::lex::token const &token)
  {
    using namespace jank::read::lex;
    switch(token.kind)
    {
      case token_kind::open_paren:
      case token_kind::close_paren:
      case token_kind::open_square_bracket:
      case token_kind::close_square_bracket:
      case token_kind::open_curly_bracket:
      case token_kind::close_curly_bracket:
      case token_kind::single_quote:
      case token_kind::meta_hint:
      case token_kind::reader_macro:
      case token_kind::reader_macro_comment:
      case token_kind::reader_macro_conditional:
      case token_kind::reader_macro_conditional_splice:
      case token_kind::syntax_quote:
      case token_kind::unquote:
      case token_kind::unquote_splice:
      case token_kind::deref:
      case token_kind::nil:
        return e | color(Color::DarkOrange);
      case token_kind::keyword:
        return e | color(Color::BlueLight);
      case token_kind::comment:
        return e | color(Color::GrayLight);
      case token_kind::integer:
      case token_kind::real:
      case token_kind::ratio:
      case token_kind::big_integer:
      case token_kind::boolean:
      case token_kind::character:
        return e | color(Color::MagentaLight);
      case token_kind::string:
      case token_kind::escaped_string:
        return e | color(Color::GreenLight);
      case token_kind::symbol:
        return symbol_color(e, std::get<native_persistent_string_view>(token.data));
      case token_kind::eof:
        return e | color(Color::Default);
        break;
    }
  }

  /* This function will return a map of line numbers to highlighted lines. It gracefully
   * handles lex errors by not highlighting those tokens and skipping to the next token.
   * The map will at least contain lines within the range specified and maybe some others. */
  std::map<usize, Element>
  highlight(runtime::module::file_view const &code, usize const line_start, usize const line_end)
  {
    read::lex::processor l_prc{ code.view() };
    auto const end{ l_prc.end() };
    usize last_offset{}, last_line{ 1 };
    std::map<usize, Element> lines;
    std::vector<Element> current_line;
    bool ended_on_error{};
    static auto const config{ FlexboxConfig().SetGap(0, 0) };

    auto const fill_space([&](bool const skip, usize const offset) {
      std::string_view const space{ code.data() + last_offset, offset - last_offset };
      usize last_newline{};
      for(auto it(space.find('\n')); it != decltype(space)::npos; it = space.find('\n', it + 1))
      {
        /* We add a space since this could be an empty line and ftxui will only make it take
         * space if it's non-empty. */
        if(!skip && last_line >= line_start)
        {
          std::string line{ space.substr(last_newline, it - last_newline) };
          current_line.emplace_back(text(std::move(line)));
          lines.emplace(last_line, flexbox(std::move(current_line), config));
          current_line.clear();
        }
        last_newline = it + 1;
        ++last_line;
      }
      if(!skip && last_newline < space.size())
      {
        current_line.emplace_back(text(std::string{ space.substr(last_newline) }));
      }
    });

    for(auto it(l_prc.begin()); it != end; ++it)
    {
      if(it.latest.unwrap().is_err())
      {
        ended_on_error = true;
        continue;
      }
      ended_on_error = false;

      auto const &token(it.latest.unwrap().expect_ok());
      if(token.start.line > line_end)
      {
        /* If we saw an error last, fill in the space until this token. */
        fill_space(false, token.start.offset);
        break;
      }

      auto const skip(token.start.line < line_start);
      fill_space(skip, token.start.offset);

      /* TODO: Large tokens can be broken up further, to aid in line wrapping. For example,
       * using `paragraph` for comments. */
      auto const token_size(std::max(token.end.offset - token.start.offset, 1llu));
      if(!skip)
      {
        current_line.emplace_back(
          token_color(text(std::string{ code.data() + token.start.offset, token_size }), token));
      }
      last_offset = token.start.offset + token_size;
    }

    /* If we saw an error last, fill in the space until the end of the file. */
    if(ended_on_error)
    {
      fill_space(false, code.size());
    }

    lines.emplace(last_line, flexbox(std::move(current_line), config));

    return lines;
  }
}
