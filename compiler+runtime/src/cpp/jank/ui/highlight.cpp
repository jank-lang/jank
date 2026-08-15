#include <isocline.h>

#include <jtl/terminal.hpp>

#include <jank/ui/highlight.hpp>
#include <jank/read/lex.hpp>
#include <jank/runtime/module/loader.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::ui
{
  using namespace jtl::terminal;

  /* TODO: Also support core fns? */
  static native_set<jtl::immutable_string_view> const specials{
    "def", "defn",  "fn*", "fn",  "let*",  "let",     "loop*", "loop",   "do",
    "if",  "quote", "var", "try", "catch", "finally", "throw", "letfn*", "cpp/raw"
  };

  static text_style symbol_color(jtl::immutable_string_view const &sym)
  {
    if(specials.contains(sym))
    {
      return text_style::bright_cyan | text_style::bold;
    }

    return text_style::reset;
  }

  static text_style token_color(read::lex::token const &token)
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
        return text_style::yellow;
      case token_kind::keyword:
        return text_style::bright_blue;
      case token_kind::comment:
        return text_style::bright_black;
      case token_kind::integer:
      case token_kind::real:
      case token_kind::ratio:
      case token_kind::big_integer:
      case token_kind::big_decimal:
      case token_kind::boolean:
      case token_kind::character:
        return text_style::bright_magenta;
      case token_kind::string:
      case token_kind::escaped_string:
        return text_style::bright_green;
      case token_kind::symbol:
        return symbol_color(std::get<jtl::immutable_string_view>(token.data));
      case token_kind::eof:
        return text_style::reset;
    }
  }

  /* This function will return a map of line numbers to highlighted lines. It gracefully
   * handles lex errors by not highlighting those tokens and skipping to the next token.
   * The map will at least contain lines within the range specified and maybe some others. */
  native_map<usize, jtl::immutable_string>
  highlight(runtime::module::file_view const &code, usize const line_start, usize const line_end)
  {
    read::lex::processor l_prc{ code.view() };
    auto const end{ l_prc.end() };
    usize last_offset{}, last_line{ 1 };
    native_map<usize, jtl::immutable_string> lines;
    jtl::string_builder current_line;
    bool ended_on_error{};

    /* As we progress through the file, we update our offset. When we call `fill_in_lines`, we
     * catch up based on scanning for new line characters between the last offset and the
     * new offset. For any new lines found, we track the line and build it up if it's
     * in our target range. */
    auto const fill_in_lines([&](bool const skip, usize const offset, text_style const style) {
      jtl::immutable_string_view const space{ code.data() + last_offset, offset - last_offset };
      usize last_newline{};
      for(auto it(space.find('\n')); it != decltype(space)::npos; it = space.find('\n', it + 1))
      {
        if(!skip)
        {
          auto const line{ space.substr(last_newline, it - last_newline) };
          current_line(style);
          current_line(line);
          lines.emplace(last_line, current_line.release());
        }
        last_newline = it + 1;
        ++last_line;
      }

      if(!skip && last_newline < space.size())
      {
        current_line(style);
        current_line(space.substr(last_newline));
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
        /* We're at the end of our range. In case we saw an error last, fill in the space
         * until this token. */
        fill_in_lines(false, token.start.offset, text_style::reset);
        break;
      }

      auto const skip(token.end.line < line_start);
      fill_in_lines(skip, token.start.offset, text_style::reset);

      /* TODO: Large tokens can be broken up further, to aid in line wrapping. For example,
       * using `paragraph` for comments. */
      auto const token_size(std::max(token.end.offset - token.start.offset, 1llu));
      jtl::immutable_string_view const code_range{ code.data() + token.start.offset, token_size };
      /* Multi-line tokens can't just be added to the current line. We need to walk through
       * all of the new lines and build things up accordingly. We just use the same
       * `fill_in_lines` fn for this, but we give it the token color.
       *
       * This only adds lines if it finds a new line character, though, so the normal
       * case of a single-line token is handled below. */
      if(code_range.contains('\n'))
      {
        last_offset = token.start.offset;
        fill_in_lines(skip, token.start.offset + token_size, token_color(token));
      }
      else
      {
        current_line(token_color(token));
        current_line(code_range);
      }
      last_offset = token.start.offset + token_size;
    }

    /* If we saw an error last, fill in the space until the end of the file. */
    if(ended_on_error)
    {
      fill_in_lines(false, code.size(), text_style::reset);
    }

    lines.emplace(last_line, current_line.release());

    return lines;
  }

  jtl::immutable_string highlight_str(runtime::module::file_view const &code)
  {
    auto const lines{ std::count(code.data(), code.data() + code.size(), '\n') };
    return highlight_str(code, 0, lines + 1);
  }

  jtl::immutable_string highlight_str(runtime::module::file_view const &code,
                                      usize const line_start,
                                      usize const line_end)
  {
    auto const line_to_elem{ highlight(code, line_start, line_end) };
    jtl::string_builder sb;
    for(usize i{ line_start }; i <= line_end; ++i)
    {
      auto const found{ line_to_elem.find(i + 1) };
      if(found != line_to_elem.end())
      {
        sb(found->second);
        sb('\n');
      }
    }
    return sb.release();
  }

  /* Highlighting for isocline is done in place. We use this for highlighting prompts in jank's
   * terminal REPL client. Note that the colors match our normal highlighting above. */
  void highlight_for_ic(ic_highlight_env_t * const henv, jtl::immutable_string_view const input)
  {
    read::lex::processor l_prc{ input };
    auto const end{ l_prc.end() };

    for(auto it(l_prc.begin()); it != end; ++it)
    {
      if(it.latest.unwrap().is_err())
      {
        continue;
      }

      auto const &token(it.latest.unwrap().expect_ok());
      char const *style{ "none" };
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
          style = "ansi-olive";
          break;
        case read::lex::token_kind::keyword:
          style = "ansi-blue";
          break;
        case read::lex::token_kind::comment:
          style = "ansi-gray";
          break;
        case read::lex::token_kind::integer:
        case read::lex::token_kind::real:
        case read::lex::token_kind::ratio:
        case read::lex::token_kind::big_integer:
        case read::lex::token_kind::big_decimal:
        case read::lex::token_kind::boolean:
        case read::lex::token_kind::character:

        case read::lex::token_kind::string:
        case read::lex::token_kind::escaped_string:
          style = "ansi-fuchsia";
          break;
        case read::lex::token_kind::symbol:
          if(specials.contains(std::get<jtl::immutable_string_view>(token.data)))
          {
            style = "ansi-aqua";
          }
          break;
        case read::lex::token_kind::eof:
          break;
      }

      ic_highlight(henv, token.start.offset, token.end.offset - token.start.offset, style);
    }
  }
}
