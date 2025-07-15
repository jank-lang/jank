#include <codecvt>

#include <jank/read/parse.hpp>
#include <jank/error/parse.hpp>
#include <jank/util/escape.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/obj/ratio.hpp>
#include <jank/runtime/behavior/map_like.hpp>
#include <jank/runtime/behavior/set_like.hpp>
#include <jank/runtime/sequence_range.hpp>
#include <jank/util/scope_exit.hpp>
#include <jank/util/fmt.hpp>

/* TODO: Make common symbol boxes once and reuse those. */
namespace jank::read::parse
{
  using namespace jank::runtime;

  jtl::result<jtl::immutable_string, char_parse_error>
  parse_character_in_base(jtl::immutable_string const &char_literal, int const base)
  {
    try
    {
      size_t chars_processed{};
      auto const codepoint(std::stol(char_literal, &chars_processed, base));

      /* `std::stol` will ignore any character that lies outside of
       * the `base` digits range.
       *
       * For base `8`, valid digits are {0, 1, 2, 3, 4, 5, 6, 7}.
       * For base `16`, valid digits are {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, a, b, c, d, e, f, A, B, C, D, E, F}
       *
       * If `std::stol` doesn't process all the
       * characters in `char_literal`, we'll consider it as invalid.
       *
       * Refer: https://en.cppreference.com/w/cpp/string/basic_string/stol
       */
      if(chars_processed != char_literal.size())
      {
        return err("Invalid Unicode digit.");
      }

      /* C++ helpfully deprecated the only standard way of converting Unicode formats.
       * We'll use it while we can. It'll be gone in C++26. */
      std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
      jtl::immutable_string const converted(converter.to_bytes(codepoint));

      if(converter.converted() != 1)
      {
        return err("Unicode character is out of range.");
      }

      return ok(converted);
    }
    catch(std::exception const &)
    {
      return err("Unable to parse Unicode codepoint.");
    }
  }

  jtl::option<char> get_char_from_literal(jtl::immutable_string const &s)
  {
    if(s.size() == 2)
    {
      return s[1];
    }
    else if(s == R"(\newline)")
    {
      return '\n';
    }
    else if(s == R"(\space)")
    {
      return ' ';
    }
    else if(s == R"(\tab)")
    {
      return '\t';
    }
    else if(s == R"(\backspace)")
    {
      return '\b';
    }
    else if(s == R"(\formfeed)")
    {
      return '\f';
    }
    else if(s == R"(\return)")
    {
      return '\r';
    }

    return none;
  }

  bool object_source_info::operator==(object_source_info const &rhs) const
  {
    return !(*this != rhs);
  }

  bool object_source_info::operator!=(object_source_info const &rhs) const
  {
    return ptr != rhs.ptr || start != rhs.start || end != rhs.end;
  }

  processor::iterator::value_type processor::iterator::operator*() const
  {
    return latest.unwrap();
  }

  processor::iterator::pointer processor::iterator::operator->()
  {
    return &latest.unwrap();
  }

  processor::iterator &processor::iterator::operator++()
  {
    latest = some(p->next());
    return *this;
  }

  bool processor::iterator::operator!=(processor::iterator const &rhs) const
  {
    return latest != rhs.latest;
  }

  bool processor::iterator::operator==(processor::iterator const &rhs) const
  {
    return latest == rhs.latest;
  }

  processor::processor(lex::processor::iterator const &b, lex::processor::iterator const &e)
    : token_current{ b }
    , token_end{ e }
    , splicing_allowed_var{ make_box<var>(
                              __rt_ctx->intern_ns(make_box<obj::symbol>("clojure.core")),
                              make_box<obj::symbol>("*splicing-allowed?*"),
                              jank_false)
                              ->set_dynamic(true) }
  {
  }

  processor::object_result processor::next()
  {
    if(token_current == token_end)
    {
      return ok(none);
    }

    while(true)
    {
      if(!pending_forms.empty())
      {
        auto const front(pending_forms.front());
        pending_forms.pop_front();
        /* NOTE: Source info for spliced forms will be incorrect. Right now,
         * that doesn't matter. If it begins to matter, we need to store the
         * source info along with the object in `pending_forms`. */
        return object_source_info{ front, latest_token, latest_token };
      }

      auto token_result(*token_current);
      if(token_result.is_err())
      {
        return token_result.err().unwrap();
      }
      latest_token = token_result.expect_ok();
      switch(latest_token.kind)
      {
        /* We ignore comments, but everything else returns out of the loop. */
        case lex::token_kind::comment:
          ++token_current;
          continue;

        case lex::token_kind::open_paren:
          return parse_list();
        case lex::token_kind::open_square_bracket:
          return parse_vector();
        case lex::token_kind::open_curly_bracket:
          return parse_map();
        case lex::token_kind::close_square_bracket:
        case lex::token_kind::close_paren:
        case lex::token_kind::close_curly_bracket:
          if(expected_closer != latest_token.kind)
          {
            return error::parse_unexpected_closing_character(latest_token);
          }
          ++token_current;
          expected_closer = none;
          return ok(none);
        case lex::token_kind::single_quote:
          return parse_quote();
        case lex::token_kind::character:
          return parse_character();
        case lex::token_kind::meta_hint:
          return parse_meta_hint();
        case lex::token_kind::reader_macro:
          {
            auto res(parse_reader_macro());
            /* Reader macros can skip some items entirely. */
            if(res.is_ok() && res.expect_ok().is_none())
            {
              continue;
            }
            return res;
          }
        case lex::token_kind::reader_macro_comment:
          {
            auto res(parse_reader_macro_comment());
            if(res.is_ok() && res.expect_ok().is_none())
            {
              continue;
            }
            return res;
          }
        case lex::token_kind::reader_macro_conditional:
          {
            auto res(parse_reader_macro_conditional(false));
            if(res.is_ok() && res.expect_ok().is_none())
            {
              continue;
            }
            return res;
          }
        case lex::token_kind::reader_macro_conditional_splice:
          {
            auto res(parse_reader_macro_conditional(true));
            if(res.is_ok() && res.expect_ok().is_none())
            {
              continue;
            }
            return res;
          }
        case lex::token_kind::syntax_quote:
          return parse_syntax_quote();
        case lex::token_kind::unquote:
          return parse_unquote(false);
        case lex::token_kind::unquote_splice:
          return parse_unquote(true);
        case lex::token_kind::deref:
          return parse_deref();
        case lex::token_kind::nil:
          return parse_nil();
        case lex::token_kind::boolean:
          return parse_boolean();
        case lex::token_kind::symbol:
          return parse_symbol();
        case lex::token_kind::keyword:
          return parse_keyword();
        case lex::token_kind::integer:
          return parse_integer();
        case lex::token_kind::real:
          return parse_real();
        case lex::token_kind::ratio:
          return parse_ratio();
        case lex::token_kind::big_integer:
          return parse_big_integer();
        case lex::token_kind::string:
          return parse_string();
        case lex::token_kind::escaped_string:
          return parse_escaped_string();
        case lex::token_kind::eof:
          return ok(none);
        default:
          {
            return error::internal_parse_failure(
              util::format("Unexpected token kind '{}'.", lex::token_kind_str(latest_token.kind)),
              { latest_token.start, latest_token.end });
          }
      }
    }
  }

  processor::object_result processor::parse_list()
  {
    auto const start_token((*token_current).expect_ok());
    ++token_current;
    auto const prev_expected_closer(expected_closer);
    expected_closer = some(lex::token_kind::close_paren);

    __rt_ctx
      ->push_thread_bindings(
        obj::persistent_hash_map::create_unique(std::make_pair(splicing_allowed_var, jank_true)))
      .expect_ok();
    util::scope_exit const finally{ [] { __rt_ctx->pop_thread_bindings().expect_ok(); } };

    runtime::detail::native_transient_vector ret;
    for(auto it(begin()); it != end(); ++it)
    {
      if(it.latest.unwrap().is_err())
      {
        return err(it.latest.unwrap().expect_err());
      }
      ret.push_back(it.latest.unwrap().expect_ok().unwrap().ptr);
    }
    if(expected_closer.is_some())
    {
      return error::parse_unterminated_list({ start_token.start, latest_token.end });
    }

    expected_closer = prev_expected_closer;

    return object_source_info{ make_box<obj::persistent_list>(
                                 source_to_meta(start_token.start, latest_token.end),
                                 std::in_place,
                                 ret.rbegin(),
                                 ret.rend()),
                               start_token,
                               latest_token };
  }

  processor::object_result processor::parse_vector()
  {
    auto const start_token((*token_current).expect_ok());
    ++token_current;
    auto const prev_expected_closer(expected_closer);
    expected_closer = some(lex::token_kind::close_square_bracket);

    __rt_ctx
      ->push_thread_bindings(
        obj::persistent_hash_map::create_unique(std::make_pair(splicing_allowed_var, jank_true)))
      .expect_ok();
    util::scope_exit const finally{ [] { __rt_ctx->pop_thread_bindings().expect_ok(); } };

    runtime::detail::native_transient_vector ret;
    for(auto it(begin()); it != end(); ++it)
    {
      if(it.latest.unwrap().is_err())
      {
        return err(it.latest.unwrap().expect_err());
      }
      ret.push_back(it.latest.unwrap().expect_ok().unwrap().ptr);
    }
    if(expected_closer.is_some())
    {
      return error::parse_unterminated_vector(start_token.start);
    }

    expected_closer = prev_expected_closer;
    return object_source_info{ make_box<obj::persistent_vector>(
                                 source_to_meta(start_token.start, latest_token.end),
                                 ret.persistent()),
                               start_token,
                               latest_token };
  }

  processor::object_result processor::parse_map()
  {
    auto const start_token((*token_current).expect_ok());
    ++token_current;
    auto const prev_expected_closer(expected_closer);
    expected_closer = some(lex::token_kind::close_curly_bracket);

    __rt_ctx
      ->push_thread_bindings(
        obj::persistent_hash_map::create_unique(std::make_pair(splicing_allowed_var, jank_true)))
      .expect_ok();
    util::scope_exit const finally{ [] { __rt_ctx->pop_thread_bindings().expect_ok(); } };
    native_vector<processor::object_result> const items(begin(), end());

    if(expected_closer.is_some())
    {
      return error::parse_unterminated_map(start_token.start);
    }

    expected_closer = prev_expected_closer;

    native_unordered_map<runtime::object_ref, object_source_info> parsed_keys{};

    auto const build_map([&](auto &map) -> jtl::result<void, error_ref> {
      using T = std::remove_reference_t<decltype(map)>;

      for(auto item(items.begin()); item != items.end(); ++item)
      {
        if(item->is_err())
        {
          return err(item->expect_err());
        }
        auto const key(item->expect_ok().unwrap());

        if(++item == items.end())
        {
          return error::parse_odd_entries_in_map({ start_token.start, latest_token.end },
                                                 { key.start.start, key.end.end });
        }

        if(item->is_err())
        {
          return err(item->expect_err());
        }
        auto const value(item->expect_ok());

        if(auto const parsed_key = parsed_keys.find(key.ptr); parsed_key != parsed_keys.end())
        {
          return error::parse_duplicate_keys_in_map(
            {
              key.start.start,
              key.end.end
          },
            { "Original key.",
              { parsed_key->second.start.start, parsed_key->second.end.end },
              error::note::kind::info });
        }

        parsed_keys.insert({ key.ptr, key });

        if constexpr(std::same_as<T, runtime::detail::native_array_map>)
        {
          map.insert_or_assign(key.ptr, value.unwrap().ptr);
        }
        else
        {
          map.insert(std::make_pair(key.ptr, value.unwrap().ptr));
        }
      }

      return jtl::ok();
    });

    if((items.size() / 2) <= runtime::detail::native_array_map::max_size)
    {
      runtime::detail::native_array_map map{};
      map.reserve(items.size() / 2);
      auto const res{ build_map(map) };

      if(res.is_err())
      {
        return res.expect_err();
      }

      return object_source_info{ make_box<obj::persistent_array_map>(
                                   source_to_meta(start_token.start, latest_token.end),
                                   std::move(map)),
                                 start_token,
                                 latest_token };
    }
    else
    {
      runtime::detail::native_transient_hash_map transient_map{};
      auto const res{ build_map(transient_map) };

      if(res.is_err())
      {
        return res.expect_err();
      }

      auto map{ transient_map.persistent() };

      return object_source_info{ make_box<obj::persistent_hash_map>(
                                   source_to_meta(start_token.start, latest_token.end),
                                   std::move(map)),
                                 start_token,
                                 latest_token };
    }
  }

  processor::object_result processor::parse_quote()
  {
    auto const start_token((*token_current).expect_ok());
    ++token_current;
    auto const old_quoted(quoted);
    quoted = !syntax_quoted;
    auto val_result(next());
    quoted = old_quoted;
    if(val_result.is_err())
    {
      return val_result;
    }
    else if(val_result.expect_ok().is_none())
    {
      return error::parse_invalid_quote({ start_token.start, latest_token.end },
                                        "Quote form is missing its value.");
    }

    return object_source_info{ make_box<obj::persistent_list>(
                                 source_to_meta(start_token.start, latest_token.end),
                                 std::in_place,
                                 make_box<obj::symbol>("quote"),
                                 val_result.expect_ok().unwrap().ptr)
                                 .erase(),
                               start_token,
                               latest_token };
  }

  processor::object_result processor::parse_character()
  {
    auto const start_token((*token_current).expect_ok());
    ++token_current;
    auto const sv(std::get<native_persistent_string_view>(start_token.data));
    auto const character(get_char_from_literal(sv));

    if(character.is_none())
    {
      /* Hexadecimal unicode */
      if(sv[0] == '\\' && (sv[1] == 'u' || sv[1] == 'o'))
      {
        auto const base{ (sv[1] == 'u' ? 16 : 8) };
        auto const char_bytes(parse_character_in_base(sv.substr(2), base));

        if(char_bytes.is_err())
        {
          return error::parse_invalid_unicode({ start_token.start, latest_token.end },
                                              char_bytes.expect_err().error);
        }

        return object_source_info{ make_box<obj::character>(char_bytes.expect_ok()),
                                   start_token,
                                   start_token };
      }

      return error::parse_invalid_character(start_token);
    }

    return object_source_info{ make_box<obj::character>(character.unwrap()),
                               start_token,
                               start_token };
  }

  processor::object_result processor::parse_meta_hint()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;
    auto meta_val_result(next());
    if(meta_val_result.is_err())
    {
      return meta_val_result;
    }
    else if(meta_val_result.expect_ok().is_none())
    {
      return error::parse_invalid_meta_hint_value({ start_token.start, latest_token.end });
    }

    auto meta_result(visit_object(
      [&](auto const typed_val) -> processor::object_result {
        using T = typename decltype(typed_val)::value_type;
        if constexpr(std::same_as<T, obj::keyword>)
        {
          return object_source_info{ obj::persistent_array_map::create_unique(typed_val, jank_true),
                                     start_token,
                                     latest_token };
        }
        if constexpr(behavior::map_like<T>)
        {
          return object_source_info{ typed_val, start_token, latest_token };
        }
        else
        {
          return error::parse_invalid_meta_hint_value({ start_token.start, latest_token.end });
        }
      },
      meta_val_result.expect_ok().unwrap().ptr));
    if(meta_result.is_err())
    {
      return meta_result;
    }

    auto target_val_result(next());
    if(target_val_result.is_err())
    {
      return target_val_result;
    }
    else if(target_val_result.expect_ok().is_none())
    {
      return error::parse_invalid_meta_hint_target({ start_token.start, latest_token.end },
                                                   "Value missing after hint.");
    }

    return visit_object(
      [&](auto const typed_val) -> processor::object_result {
        using T = typename decltype(typed_val)::value_type;
        if constexpr(behavior::metadatable<T>)
        {
          if(typed_val->meta.is_none())
          {
            return object_source_info{ typed_val->with_meta(meta_result.expect_ok().unwrap().ptr),
                                       start_token,
                                       latest_token };
          }
          else
          {
            return object_source_info{ typed_val->with_meta(
                                         merge(typed_val->meta.unwrap(),
                                               meta_result.expect_ok().unwrap().ptr)),
                                       start_token,
                                       latest_token };
          }
        }
        else
        {
          return error::parse_invalid_meta_hint_target(
            { start_token.start, latest_token.end },
            "Target value for meta hint must accept metadata.");
        }
      },
      target_val_result.expect_ok().unwrap().ptr);
  }

  processor::object_result processor::parse_reader_macro()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;

    auto next_token_result(*token_current);
    if(next_token_result.is_err())
    {
      return next_token_result.err().unwrap();
    }
    auto next_token(next_token_result.expect_ok());

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
    switch(next_token.kind)
    {
      case lex::token_kind::open_curly_bracket:
        return parse_reader_macro_set();
      case lex::token_kind::open_paren:
        return parse_reader_macro_fn();
      case lex::token_kind::single_quote:
        return parse_reader_macro_var_quote();
      case lex::token_kind::reader_macro:
        return parse_reader_macro_symbolic_values();
      default:
        return error::parse_unsupported_reader_macro({ start_token.start, latest_token.end });
    }
#pragma clang diagnostic pop
  }

  processor::object_result processor::parse_reader_macro_set()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;

    auto const prev_expected_closer(expected_closer);
    expected_closer = some(lex::token_kind::close_curly_bracket);

    __rt_ctx
      ->push_thread_bindings(
        obj::persistent_hash_map::create_unique(std::make_pair(splicing_allowed_var, jank_true)))
      .expect_ok();
    util::scope_exit const finally{ [] { __rt_ctx->pop_thread_bindings().expect_ok(); } };

    native_unordered_map<runtime::object_ref, object_source_info> parsed_items{};
    runtime::detail::native_transient_hash_set ret;
    for(auto it(begin()); it != end(); ++it)
    {
      if(it.latest.unwrap().is_err())
      {
        return err(it.latest.unwrap().expect_err());
      }

      auto const item(it.latest.unwrap().expect_ok().unwrap());

      if(auto const parsed_item = parsed_items.find(item.ptr); parsed_item != parsed_items.end())
      {
        return error::parse_duplicate_items_in_set(
          {
            item.start.start,
            item.end.end
        },
          { "Original item.",
            { parsed_item->second.start.start, parsed_item->second.end.end },
            error::note::kind::info });
      }

      parsed_items.insert({ item.ptr, item });
      ret.insert(item.ptr);
    }
    if(expected_closer.is_some())
    {
      return error::parse_unterminated_set({ start_token.start, latest_token.end });
    }

    expected_closer = prev_expected_closer;
    return object_source_info{ make_box<obj::persistent_hash_set>(
                                 source_to_meta(start_token.start, latest_token.end),
                                 std::move(ret).persistent()),
                               start_token,
                               latest_token };
  }

  processor::object_result processor::parse_reader_macro_fn()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());

    if(shorthand.is_some())
    {
      return error::parse_nested_shorthand_function(
        start_token.start,
        { "Outer #() form starts here.", shorthand.unwrap().source, error::note::kind::info });
    }

    shorthand = shorthand_function_details{ {}, {}, start_token.start };

    auto list_result(next());
    if(list_result.is_err())
    {
      return list_result;
    }
    else if(list_result.expect_ok().is_none()
            || list_result.expect_ok().unwrap().ptr->type != object_type::persistent_list)
    {
      return error::internal_parse_failure("Value after #( must be present.",
                                           { start_token.start, latest_token.end });
    }

    auto const call(expect_object<obj::persistent_list>(list_result.expect_ok().unwrap().ptr));
    auto const call_end(list_result.expect_ok().unwrap().end);

    runtime::detail::native_transient_vector arg_trans;
    if(shorthand.unwrap().max_fixed_arity.is_some())
    {
      for(u8 i{}; i < shorthand.unwrap().max_fixed_arity.unwrap(); ++i)
      {
        arg_trans.push_back(make_box<obj::symbol>(util::format("%{}#", i + 1)));
      }
    }
    if(shorthand.unwrap().variadic)
    {
      arg_trans.push_back(make_box<obj::symbol>("&"));
      arg_trans.push_back(make_box<obj::symbol>("%&#"));
    }

    auto const args(make_box<obj::persistent_vector>(arg_trans.persistent()));
    auto const wrapped(
      make_box<obj::persistent_list>(std::in_place, make_box<obj::symbol>("fn*"), args, call));

    shorthand = none;

    return object_source_info{ wrapped, start_token, call_end };
  }

  processor::object_result processor::parse_reader_macro_var_quote()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;

    auto sym_result(next());
    if(sym_result.is_err())
    {
      return sym_result;
    }
    else if(sym_result.expect_ok().is_none())
    {
      return error::parse_invalid_reader_var({ start_token.start, latest_token.end },
                                             "Value after #' must be present.");
    }
    else if(sym_result.expect_ok().unwrap().ptr->type != object_type::symbol)
    {
      return error::parse_invalid_reader_var({ start_token.start, latest_token.end },
                                             "Value after #' must be a symbol.");
    }

    auto const sym(expect_object<obj::symbol>(sym_result.expect_ok().unwrap().ptr));
    auto const sym_end(sym_result.expect_ok().unwrap().end);

    auto const wrapped(
      make_box<obj::persistent_list>(std::in_place, make_box<obj::symbol>("var"), sym));

    return object_source_info{ wrapped, start_token, sym_end };
  }

  processor::object_result processor::parse_reader_macro_symbolic_values()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;

    auto sym_result(next());

    if(sym_result.is_err())
    {
      return sym_result;
    }
    else if(sym_result.expect_ok().is_none())
    {
      return error::parse_invalid_reader_symbolic_value("Value after ## must be present.",
                                                        { start_token.start, latest_token.end });
    }
    else if(sym_result.expect_ok().unwrap().ptr->type != object_type::symbol)
    {
      return error::parse_invalid_reader_symbolic_value("Value after ## must be a symbol.",
                                                        { start_token.start, latest_token.end });
    }

    auto const sym(expect_object<obj::symbol>(sym_result.expect_ok().unwrap().ptr));
    auto const sym_end(sym_result.expect_ok().unwrap().end);

    f64 n{};
    if(sym->name == "Inf")
    {
      n = std::numeric_limits<f64>::infinity();
    }
    else if(sym->name == "-Inf")
    {
      n = -std::numeric_limits<f64>::infinity();
    }
    else if(sym->name == "NaN")
    {
      n = std::numeric_limits<f64>::quiet_NaN();
    }
    else
    {
      return error::parse_invalid_reader_symbolic_value("Invalid symbolic value.",
                                                        { start_token.start, latest_token.end });
    }

    auto const wrapped(make_box<obj::real>(n));
    return object_source_info{ wrapped, start_token, sym_end };
  }

  processor::object_result processor::parse_reader_macro_comment()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;

    auto ignored_result(next());
    if(ignored_result.is_err())
    {
      return ignored_result;
    }
    else if(ignored_result.expect_ok().is_none())
    {
      return error::parse_invalid_reader_comment({ start_token.start, latest_token.end },
                                                 "Value after #_ must be present.");
    }

    return ok(none);
  }

  processor::object_result processor::parse_reader_macro_conditional(bool const splice)
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;

    auto list_result(next());
    if(list_result.is_err())
    {
      return list_result;
    }
    else if(list_result.expect_ok().is_none())
    {
      return error::parse_invalid_reader_conditional({ start_token.start, latest_token.end },
                                                     "Value after #? must be present.");
    }
    else if(list_result.expect_ok().unwrap().ptr->type != object_type::persistent_list)
    {
      return error::parse_invalid_reader_conditional({ start_token.start, latest_token.end },
                                                     "Value after #? must be a list.");
    }

    auto const list(expect_object<obj::persistent_list>(list_result.expect_ok().unwrap().ptr));
    auto const list_end(list_result.expect_ok().unwrap().end);

    if(list->count() % 2 == 1)
    {
      return error::parse_invalid_reader_conditional({ start_token.start, latest_token.end },
                                                     "#? expects an even number of forms.");
    }

    auto const jank_keyword(__rt_ctx->intern_keyword("", "jank").expect_ok());
    auto const default_keyword(__rt_ctx->intern_keyword("", "default").expect_ok());

    auto const r{ make_sequence_range(list) };
    for(auto it(r.begin()); it != r.end(); ++it, ++it)
    {
      auto const kw(*it);
      /* We take the first match, checking for :jank first. If there are duplicates, it doesn't
       * matter. If :default comes first, we'll always take it. In short, order is important. This
       * matches Clojure's behavior. */
      if(equal(kw, jank_keyword) || equal(kw, default_keyword))
      {
        if(splice)
        {
          if(!truthy(splicing_allowed_var->deref()))
          {
            return error::parse_invalid_reader_splice({ start_token.start, latest_token.end },
                                                      "Top-level #?@ usage is not allowed.");
          }

          auto const s(*(++it));
          return visit_seqable(
            [&](auto const typed_s) -> processor::object_result {
              auto const r{ make_sequence_range(typed_s) };
              if(r.begin() == r.end())
              {
                return ok(none);
              }
              auto const first(*r.begin());

              auto const front(pending_forms.begin());
              for(auto it(++r.begin()); it != r.end(); ++it)
              {
                pending_forms.insert(front, *it);
              }

              return object_source_info{ first, start_token, list_end };
            },
            [&]() -> processor::object_result {
              /* TODO: Get the source of just this form. */
              return error::parse_invalid_reader_splice({ start_token.start, latest_token.end },
                                                        "#?@ must be used on a sequence.");
            },
            s);
        }
        else
        {
          return object_source_info{ *(++it), start_token, list_end };
        }
      }
    }

    return ok(none);
  }

  jtl::result<object_ref, error_ref> processor::syntax_quote_expand_seq(object_ref const seq)
  {
    if(seq.is_nil())
    {
      return seq;
    }

    return visit_seqable(
      [this](auto const typed_seq) -> jtl::result<object_ref, error_ref> {
        runtime::detail::native_transient_vector ret;
        for(auto const item : make_sequence_range(typed_seq))
        {
          if(syntax_quote_is_unquote(item, false))
          {
            ret.push_back(
              make_box<obj::persistent_list>(std::in_place,
                                             make_box<obj::symbol>("clojure.core", "list"),
                                             second(item)));
          }
          else if(syntax_quote_is_unquote(item, true))
          {
            ret.push_back(second(item));
          }
          else
          {
            auto quoted_item(syntax_quote(item));
            if(quoted_item.is_err())
            {
              return quoted_item;
            }
            ret.push_back(
              make_box<obj::persistent_list>(std::in_place,
                                             make_box<obj::symbol>("clojure.core", "list"),
                                             quoted_item.expect_ok()));
          }
        }
        auto const vec(make_box<obj::persistent_vector>(ret.persistent())->seq());
        return vec;
      },
      []() -> jtl::result<object_ref, error_ref> {
        return err(error::internal_parse_failure("syntax_quote_expand_seq arg not seqable."));
      },
      seq);
  }

  jtl::result<object_ref, error_ref> processor::syntax_quote_flatten_map(object_ref const seq)
  {
    if(seq.is_nil())
    {
      return seq;
    }

    return visit_seqable(
      [](auto const typed_seq) -> jtl::result<object_ref, error_ref> {
        runtime::detail::native_transient_vector ret;
        for(auto const item : make_sequence_range(typed_seq))
        {
          ret.push_back(first(item));
          ret.push_back(second(item));
        }
        auto const vec(make_box<obj::persistent_vector>(ret.persistent())->seq());
        return vec;
      },
      []() -> jtl::result<object_ref, error_ref> {
        return err(error::internal_parse_failure("syntax_quote_flatten_map arg is not a seq."));
      },
      seq);
  }

  bool processor::syntax_quote_is_unquote(object_ref const form, bool const splice)
  {
    return visit_seqable(
      [splice](auto const typed_form) {
        auto const s(typed_form->seq());
        object_ref const item{ s.is_some() ? first(s).erase() : s.erase() };

        return make_box<obj::symbol>("clojure.core", (splice ? "unquote-splicing" : "unquote"))
          ->equal(*item);
      },
      [] { return false; },
      form);
  }

  jtl::result<object_ref, error_ref> processor::syntax_quote(object_ref const form)
  {
    object_ref ret{};

    /* Specials, such as fn*, let*, try, etc. just get left alone. We can't qualify them more. */
    if(__rt_ctx->an_prc.is_special(form))
    {
      ret = make_box<obj::persistent_list>(std::in_place, make_box<obj::symbol>("quote"), form);
    }
    /* By default, all symbols get qualified. However, any symbol ending in # does not get
     * qualified, but instead gets a gensym (a unique name). The unique names are kept in
     * a bound map for reproducibility. */
    else if(form->type == object_type::symbol)
    {
      auto sym(expect_object<obj::symbol>(form));
      if(sym->ns.empty() && sym->name.ends_with('#'))
      {
        auto const env(__rt_ctx->gensym_env_var->deref());
        if(env->type == object_type::nil)
        {
          return err(error::internal_parse_failure("Missing gensym env."));
        }

        auto gensym(get(env, sym));
        if(gensym->type == object_type::nil)
        {
          gensym = make_box<obj::symbol>(__rt_ctx->unique_symbol(sym->name));
          __rt_ctx->gensym_env_var->set(assoc(env, sym, gensym)).expect_ok();
        }
        sym = expect_object<obj::symbol>(gensym);
      }
      else if(sym->ns.empty() && sym->name != "&")
      {
        auto var(__rt_ctx->find_var(sym));
        if(var.is_nil())
        {
          sym = make_box<obj::symbol>(__rt_ctx->current_ns()->name->name, sym->name);
        }
        else
        {
          sym = make_box<obj::symbol>(var->n->name->name, sym->name);
        }
      }

      ret = make_box<obj::persistent_list>(std::in_place, make_box<obj::symbol>("quote"), sym);
    }
    else if(syntax_quote_is_unquote(form, false))
    {
      return second(form);
    }
    else if(syntax_quote_is_unquote(form, true))
    {
      return err(error::parse_invalid_syntax_unquote_splice(meta_source(form)));
    }
    /* Clojure treats these specially, perhaps as a small optimization, by not quoting. We can
     * do the same for now, but quoting all of these has no effect. */
    else if(form->type == object_type::keyword || form->type == object_type::persistent_string
            || form->type == object_type::integer || form->type == object_type::real
            || form->type == object_type::character || form->type == object_type::nil)
    {
      ret = form;
    }
    else
    {
      /* Handle all sorts of sequences. We do this by recursively walking through them,
       * flattening them, qualifying the symbols, and then building up code which will
       * reassemble them. */
      auto const res{ visit_seqable(
        [&](auto const typed_form) -> jtl::result<object_ref, error_ref> {
          using T = typename decltype(typed_form)::value_type;

          if constexpr(std::same_as<T, obj::persistent_vector>)
          {
            auto const seq(typed_form->seq());
            if(seq.is_nil())
            {
              return make_box<obj::persistent_list>(
                std::in_place,
                make_box<obj::symbol>("clojure.core", "vector"));
            }

            auto expanded(syntax_quote_expand_seq(seq));
            if(expanded.is_err())
            {
              return expanded;
            }

            return make_box<obj::persistent_list>(
              std::in_place,
              make_box<obj::symbol>("clojure.core", "apply*"),
              make_box<obj::symbol>("clojure.core", "vector"),
              make_box<obj::persistent_list>(
                std::in_place,
                make_box<obj::symbol>("clojure.core", "seq"),
                cons(make_box<obj::symbol>("clojure.core", "concat*"), expanded.expect_ok())));
          }
          if constexpr(behavior::map_like<T>)
          {
            auto flattened(syntax_quote_flatten_map(typed_form->seq()));
            if(flattened.is_err())
            {
              return flattened;
            }

            auto expanded(syntax_quote_expand_seq(flattened.expect_ok()));
            if(expanded.is_err())
            {
              return expanded;
            }

            return make_box<obj::persistent_list>(
              std::in_place,
              make_box<obj::symbol>("clojure.core", "apply*"),
              make_box<obj::symbol>("clojure.core", "hash-map"),
              make_box<obj::persistent_list>(
                std::in_place,
                make_box<obj::symbol>("clojure.core", "seq"),
                cons(make_box<obj::symbol>("clojure.core", "concat*"), expanded.expect_ok())));
          }
          if constexpr(behavior::set_like<T>)
          {
            return err(error::internal_parse_failure("nyi: set"));
          }
          if constexpr(behavior::sequenceable<T>)
          {
            auto const seq(typed_form->seq());
            if(seq.is_nil())
            {
              return make_box<obj::persistent_list>(std::in_place,
                                                    make_box<obj::symbol>("clojure.core", "list"));
            }
            auto expanded(syntax_quote_expand_seq(seq));
            if(expanded.is_err())
            {
              return expanded;
            }

            /* TODO: Need seq? */
            return make_box<obj::persistent_list>(
              std::in_place,
              make_box<obj::symbol>("clojure.core", "seq"),
              cons(make_box<obj::symbol>("clojure.core", "concat*"), expanded.expect_ok()));
          }
          else
          {
            return err(
              error::internal_parse_failure(util::format("Unsupported collection: {} [{}]",
                                                         typed_form->to_code_string(),
                                                         object_type_str(typed_form->base.type))));
          }
        },
        /* For anything else, do nothing special aside from quoting. Hopefully that works. */
        [=]() -> jtl::result<object_ref, error_ref> {
          return make_box<obj::persistent_list>(std::in_place,
                                                make_box<obj::symbol>("quote"),
                                                form);
        },
        form) };
      if(res.is_err())
      {
        return res;
      }
      ret = res.expect_ok();
    }

    auto const meta{ runtime::meta(form) };
    if(meta != jank_nil)
    {
      /* We quote the meta as well, to ensure it doesn't get evaluated.
       * Note that Clojure removes the source info from the meta here. We're keeping it
       * so that we can provide improved macro expansion errors. This comes as a performance
       * cost during compilation and during startup, as well as higher memory usage for keeping
       * all of the meta around. */
      /* TODO: Support a flag to strip this out. */
      auto const quoted_meta{ syntax_quote(meta) };
      if(quoted_meta.is_err())
      {
        return err(quoted_meta.expect_err());
      }

      return make_box<obj::persistent_list>(std::in_place,
                                            make_box<obj::symbol>("clojure.core", "with-meta"),
                                            ret,
                                            quoted_meta.expect_ok());
    }

    return ret;
  }

  processor::object_result processor::parse_syntax_quote()
  {
    auto const start_token(token_current.latest.unwrap().expect_ok());
    ++token_current;

    context::binding_scope const scope{ *__rt_ctx,
                                        obj::persistent_hash_map::create_unique(
                                          std::make_pair(__rt_ctx->gensym_env_var,
                                                         obj::persistent_hash_map::empty())) };

    auto const old_quoted(quoted);
    quoted = false;
    auto const old_syntax_quoted(syntax_quoted);
    syntax_quoted = true;
    quoted = false;
    auto quoted_form(next());
    quoted = old_quoted;
    syntax_quoted = old_syntax_quoted;
    if(quoted_form.is_err())
    {
      return quoted_form;
    }
    else if(quoted_form.expect_ok().is_none())
    {
      return error::parse_invalid_syntax_quote({ start_token.start, latest_token.end },
                                               "Value after ` must be present.");
    }

    auto const res(syntax_quote(quoted_form.expect_ok().unwrap().ptr));
    if(res.is_err())
    {
      return res.expect_err();
    }

    return object_source_info{ res.expect_ok(), start_token, quoted_form.expect_ok().unwrap().end };
  }

  processor::object_result processor::parse_unquote(bool const splice)
  {
    auto const start_token((*token_current).expect_ok());
    ++token_current;
    auto val_result(next());
    if(val_result.is_err())
    {
      return val_result;
    }
    else if(val_result.expect_ok().is_none())
    {
      return error::parse_invalid_syntax_unquote({ start_token.start, latest_token.end });
    }

    return object_source_info{ make_box<obj::persistent_list>(
                                 std::in_place,
                                 make_box<obj::symbol>("clojure.core",
                                                       (splice ? "unquote-splicing" : "unquote")),
                                 val_result.expect_ok().unwrap().ptr)
                                 .erase(),
                               start_token,
                               latest_token };
  }

  processor::object_result processor::parse_deref()
  {
    auto const start_token((*token_current).expect_ok());
    ++token_current;
    auto val_result(next());
    if(val_result.is_err())
    {
      return val_result;
    }
    else if(val_result.expect_ok().is_none())
    {
      return error::parse_invalid_reader_deref({ start_token.start, latest_token.end });
    }

    return object_source_info{ make_box<obj::persistent_list>(std::in_place,
                                                              make_box<obj::symbol>("deref"),
                                                              val_result.expect_ok().unwrap().ptr)
                                 .erase(),
                               start_token,
                               latest_token };
  }

  processor::object_result processor::parse_nil()
  {
    ++token_current;
    return object_source_info{ jank_nil, latest_token, latest_token };
  }

  processor::object_result processor::parse_boolean()
  {
    auto const token((*token_current).expect_ok());
    ++token_current;
    auto const b(std::get<bool>(token.data));
    return object_source_info{ make_box<obj::boolean>(b), token, token };
  }

  processor::object_result processor::parse_symbol()
  {
    auto const start_token((*token_current).expect_ok());
    ++token_current;
    auto const sv(std::get<native_persistent_string_view>(start_token.data));
    auto const slash(sv.find('/'));
    jtl::immutable_string ns, name;
    if(slash != jtl::immutable_string::npos)
    {
      /* If it's only a slash, it's a name. Otherwise, it's a ns/name separator. */
      if(sv.size() == 1)
      {
        name = sv;
      }
      else
      {
        auto const ns_portion(sv.substr(0, slash));
        /* Quoted symbols can have any ns and it doesn't need to exist. */
        if(quoted)
        {
          ns = ns_portion;
        }
        /* Normal symbols will have the ns resolved immediately if resolvable. */
        else
        {
          auto const resolved_ns(__rt_ctx->resolve_ns(make_box<obj::symbol>(ns_portion)));
          if(resolved_ns.is_nil())
          {
            ns = ns_portion;
          }
          else
          {
            ns = resolved_ns->name->name;
          }
        }
        name = sv.substr(slash + 1);
      }
    }
    else
    {
      name = sv;

      /* When we're parsing a shorthand anonymous function (i.e. #() reader macro), symbols
       * beginning with % have a special meaning which affect the shorthand function. We track
       * that state here. */
      if(shorthand.is_some() && name.starts_with('%'))
      {
        auto const after_percent(name.substr(1));
        bool all_digits{ true };
        for(auto const c : after_percent)
        {
          all_digits &= (std::isdigit(c) != 0) && (c != '0');
        }
        if(all_digits)
        {
          /* We support just % alone, which means %1. */
          u8 num{ 1 };
          if(after_percent.empty())
          {
            /* We rename it so that the generated param name matches. */
            name = "%1";
          }

          num = std::strtol(name.c_str() + 1, nullptr, 10);
          auto &max_fixed_arity(shorthand.unwrap().max_fixed_arity);
          if(max_fixed_arity.is_none() || max_fixed_arity.unwrap() < num)
          {
            max_fixed_arity = num;
          }
        }
        else if(after_percent == "&")
        {
          shorthand.unwrap().variadic = true;
        }
        else
        {
          return error::parse_invalid_shorthand_function_parameter(
            { start_token.start, latest_token.end });
        }

        /* This is a hack. We're building up an anonymous function, but that form could be within
         * a syntax quote. We don't want our %1 and %& symbols to be qualified, so we tack a #
         * at the end, so they're like foo# and get treated as gensyms. However, when not inside
         * a syntax quote, # is fine to have in a symbol anyway. This saves us from having to
         * do custom logic within the syntax quoting to check for these symbols. */
        name = name + "#";
      }
    }
    return object_source_info{
      make_box<obj::symbol>(source_to_meta(start_token.start, latest_token.end), ns, name),
      start_token,
      start_token
    };
  }

  processor::object_result processor::parse_keyword()
  {
    auto const start_token((*token_current).expect_ok());
    ++token_current;
    auto const sv(std::get<native_persistent_string_view>(start_token.data));
    /* A :: keyword either resolves to the current ns or an alias, depending on
     * whether or not it's qualified. */
    bool const resolved{ sv[0] != ':' };

    auto const slash(sv.find('/'));
    jtl::immutable_string ns, name;
    if(slash != jtl::immutable_string::npos)
    {
      if(resolved)
      {
        ns = sv.substr(0, slash);
      }
      else
      {
        ns = sv.substr(1, slash - 1);
      }
      name = sv.substr(slash + 1);
    }
    else
    {
      name = sv.substr(resolved ? 0 : 1);
    }

    auto const intern_res(__rt_ctx->intern_keyword(ns, name, resolved));
    if(intern_res.is_err())
    {
      return error::parse_invalid_keyword(intern_res.expect_err(),
                                          { start_token.start, latest_token.end });
    }
    return object_source_info{ intern_res.expect_ok(), start_token, start_token };
  }

  processor::object_result processor::parse_integer()
  {
    auto const token(token_current->expect_ok());
    ++token_current;
    return object_source_info{ make_box<obj::integer>(std::get<i64>(token.data)), token, token };
  }

  processor::object_result processor::parse_big_integer()
  {
    auto const token{ token_current->expect_ok() };
    ++token_current;
    auto const &[number_literal, radix, is_negative](std::get<lex::big_integer>(token.data));
    auto const bi(obj::big_integer::create(number_literal, radix, is_negative));
    return object_source_info{ bi, token, token };
  }

  processor::object_result processor::parse_ratio()
  {
    auto const token(token_current->expect_ok());
    ++token_current;
    auto const &ratio_data(std::get<lex::ratio>(token.data));
    if(ratio_data.denominator == 0)
    {
      return error::parse_invalid_ratio({ token.start, latest_token.end },
                                        "A ratio may not have a denominator of zero.");
    }
    if(auto const ratio{ obj::ratio::create(ratio_data.numerator, ratio_data.denominator) };
       ratio->type == object_type::ratio)
    {
      return object_source_info{ expect_object<obj::ratio>(ratio), token, token };
    }
    else if(ratio->type == object_type::integer)
    {
      return object_source_info{ expect_object<obj::integer>(ratio), token, token };
    }
    else if(ratio->type == object_type::big_integer)
    {
      return object_source_info{ expect_object<obj::big_integer>(ratio), token, token };
    }
    else
    {
      return error::internal_parse_failure("Unexpected token ratio data.",
                                           { token.start, latest_token.end });
    }
  }

  processor::object_result processor::parse_real()
  {
    auto const token(token_current->expect_ok());
    ++token_current;
    return object_source_info{ make_box<obj::real>(std::get<f64>(token.data)), token, token };
  }

  processor::object_result processor::parse_string()
  {
    auto const token(token_current->expect_ok());
    ++token_current;
    auto const sv(std::get<native_persistent_string_view>(token.data));
    return object_source_info{ make_box<obj::persistent_string>(
                                 jtl::immutable_string{ sv.data(), sv.size() }),
                               token,
                               token };
  }

  processor::object_result processor::parse_escaped_string()
  {
    auto const token(token_current->expect_ok());
    ++token_current;
    auto const sv(std::get<native_persistent_string_view>(token.data));
    auto res(util::unescape({ sv.data(), sv.size() }));
    if(res.is_err())
    {
      return error::internal_parse_failure(
        util::format("Unable to unescape string: {}", res.expect_err().message),
        { token.start, latest_token.end });
    }
    return object_source_info{ make_box<obj::persistent_string>(res.expect_ok_move()),
                               token,
                               token };
  }

  processor::iterator processor::begin()
  {
    return { some(next()), this };
  }

  processor::iterator processor::end()
  {
    return { some(ok(none)), this };
  }
}
