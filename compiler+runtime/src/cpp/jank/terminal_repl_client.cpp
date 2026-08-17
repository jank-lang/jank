#include <cctype>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>

#include <isocline.h>

#include <jtl/terminal.hpp>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/call.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/persistent_vector_sequence.hpp>
#include <jank/runtime/detail/type.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/c_api.h>
#include <jank/jit/processor.hpp>
#include <jank/aot/processor.hpp>
#include <jank/profile/time.hpp>
#include <jank/util/scope_exit.hpp>
#include <jank/util/string.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/try.hpp>
#include <jank/ui/highlight.hpp>

namespace jank::terminal_repl
{
  static bool is_clj_word_char(char const * const s, long const len)
  {
    if(s == nullptr || len <= 0)
    {
      return false;
    }

    auto const c{ static_cast<unsigned char>(*s) };
    /* Beyond ASCII implies Unicode. */
    if(c >= 0x80)
    {
      return true;
    }

    switch(c)
    {
      case '/':
      case '.':
      case '-':
      case '_':
      case '*':
      case '+':
      case '!':
      case '?':
      case '<':
      case '>':
      case '=':
      case ':':
      case '#':
        return true;
      default:
        return std::isalnum(c) != 0;
    }
  }

  static void add_repl_completions(ic_completion_env_t * const cenv, std::string const &prefix)
  {
    using namespace jank::runtime;

    std::set<std::string> seen;
    auto const add_candidate{ [&](std::string const &candidate) {
      if(candidate.empty())
      {
        return;
      }

      if(!candidate.starts_with(prefix))
      {
        return;
      }

      if(seen.insert(candidate).second)
      {
        ic_add_completion(cenv, candidate.c_str());
      }
    } };

    auto const current_ns{ __rt_ctx->current_ns() };
    auto const current_mappings{ current_ns->get_mappings() };
    for(auto const &entry : current_mappings->data)
    {
      auto const sym{ expect_object<obj::symbol>(entry.first) };
      add_candidate(sym->name);
    }

    for(auto const &ns : __rt_ctx->all_ns())
    {
      add_candidate(ns->name->to_string());
    }
  }

  static void add_qualified_repl_completions(ic_completion_env_t * const cenv,
                                             std::string const &token,
                                             std::string const &ns_name,
                                             std::string const &var_prefix)
  {
    using namespace jank::runtime;

    obj::symbol const ns_sym{ ns_name };
    auto const ns{ __rt_ctx->resolve_ns(runtime::detail::untagged(&ns_sym)) };
    if(ns.is_nil())
    {
      return;
    }

    std::set<std::string> seen;
    auto const mappings{ ns->get_mappings() };
    for(auto const &entry : mappings->data)
    {
      auto const sym{ expect_object<obj::symbol>(entry.first) };
      auto const value{ expect_object<var>(entry.second) };

      /* Namespaces can have referred vars, but we only want to show completions for
       * owned vars. */
      if(value->n != ns)
      {
        continue;
      }

      auto const &name{ sym->name };
      if(!var_prefix.empty() && !name.starts_with(var_prefix))
      {
        continue;
      }

      std::string const candidate{ ns->name->to_string() + "/" + name };
      if(!candidate.starts_with(token))
      {
        continue;
      }

      if(seen.insert(candidate).second)
      {
        ic_add_completion(cenv, candidate.c_str());
      }
    }
  }

  static void repl_word_completer(ic_completion_env_t * const cenv, char const * const prefix)
  {
    if(prefix == nullptr)
    {
      return;
    }

    std::string const token{ prefix };
    if(token.find('/') == std::string::npos)
    {
      add_repl_completions(cenv, token);
      return;
    }

    auto const slash_pos{ token.rfind('/') };
    auto const ns_name{ token.substr(0, slash_pos) };
    auto const var_prefix{ token.substr(slash_pos + 1) };
    add_qualified_repl_completions(cenv, token, ns_name, var_prefix);
  }

  static void repl_completer(ic_completion_env_t * const cenv, char const * const input)
  {
    if(input == nullptr)
    {
      return;
    }

    ic_complete_word(cenv, input, repl_word_completer, &is_clj_word_char);
  }

  static void repl_highlighter(ic_highlight_env_t * const henv, char const * const input, void *)
  {
    jank::ui::highlight_for_ic(henv, input);
  }

  static jtl::option<std::string> read_line(std::string const &prompt)
  {
    if(!jtl::terminal::is_stdin_interactive())
    {
      std::string input;
      if(!std::getline(std::cin, input))
      {
        return jank::none;
      }
      return input;
    }

    auto * const line{ ic_readline(prompt.c_str()) };
    if(!line)
    {
      return jank::none;
    }

    std::string input{ line };
    ic_free(line);
    return input;
  }

  void init_isocline()
  {
    ic_set_prompt_marker("=> ", "> ");
    ic_enable_color(true);

    ic_set_insertion_braces("()[]{}");
    ic_set_matching_braces("()[]{}");
    ic_enable_brace_insertion(true);
    ic_enable_brace_matching(true);

    ic_enable_hint(true);
    ic_set_hint_delay(0);
    ic_enable_inline_help(true);

    ic_enable_completion_preview(true);
    ic_enable_auto_tab(true);

    ic_enable_multiline(true);
    ic_enable_multiline_indent(true);
  }

  void repl()
  {
    using namespace jank;
    using namespace jank::runtime;

    {
      profile::timer const timer{ "require clojure.core" };
      __rt_ctx->load_module("clojure.core", module::origin::latest).expect_ok();
    }

    {
      profile::timer const timer{ "require jank.nrepl.server.core" };
      __rt_ctx->load_module("jank.nrepl.server.core", module::origin::latest).expect_ok();
    }

    /* This will start the nREPL server on a background thread. Meanwhile, we'll set up
     * some stuff for this thread and then we'll later block on an nREPL promise to ensure
     * that it's completely running before we prompt the user. */
    auto const repl_main{
      __rt_ctx->intern_var("jank.nrepl.server.core", "background-main").expect_ok()
    };
    repl_main.call();

    __rt_ctx->in_ns_var->deref().call(make_box<obj::symbol>("user"));
    __rt_ctx->intern_var("clojure.core", "refer")
      .expect_ok()
      .call(make_box<obj::symbol>("clojure.core"));

    if(!util::cli::opts.target_module.empty())
    {
      profile::timer const timer{ "load main" };
      __rt_ctx->load_module(util::cli::opts.target_module, module::origin::latest).expect_ok();
      __rt_ctx->in_ns_var->deref().call(make_box<obj::symbol>(util::cli::opts.target_module));
    }

    auto const get_prompt([](std::string const &suffix) {
      return __rt_ctx->current_ns()->name->to_code_string() + suffix;
    });

    if(jtl::terminal::is_stdin_interactive())
    {
      ic_set_history(".jank-repl-history", 200);
      init_isocline();
      ic_set_default_completer(repl_completer, nullptr);
      ic_set_default_highlighter(repl_highlighter, nullptr);
    }

    auto const tmp{ std::filesystem::temp_directory_path() };
    std::string path_tmp{ (tmp / "jank-repl-XXXXXX").string() };
    int const fd{ mkstemp(path_tmp.data()) };
    close(fd);

    auto const first_res_var{ __rt_ctx->find_var("clojure.core", "*1") };
    auto const second_res_var{ __rt_ctx->find_var("clojure.core", "*2") };
    auto const third_res_var{ __rt_ctx->find_var("clojure.core", "*3") };
    auto const error_var{ __rt_ctx->find_var("clojure.core", "*e") };

    context::binding_scope const scope{ obj::persistent_hash_map::create_unique(
      std::make_pair(first_res_var, jank_nil),
      std::make_pair(second_res_var, jank_nil),
      std::make_pair(third_res_var, jank_nil),
      std::make_pair(error_var, jank_nil)) };

    /* We want to wait until the nREPL server is completely initialized, so we'll block on
     * its promise. */
    auto const repl_ready{ __rt_ctx->intern_var("jank.nrepl.server.core", "ready?").expect_ok() };
    repl_ready->deref();

    while(true)
    {
      auto const line{ read_line(get_prompt("")) };
      if(line.is_none())
      {
        break;
      }

      std::string input{ line.unwrap() };
      util::trim(input);

      if(input.empty())
      {
        util::println("");
        continue;
      }

      util::scope_exit const finally{ [&] { std::filesystem::remove(path_tmp); } };
      cpptrace::try_catch(
        [&] {
          {
            std::ofstream ofs{ path_tmp };
            ofs << input;
          }

          auto const res(__rt_ctx->eval_file(path_tmp));

          if(res.is_some())
          {
            third_res_var->set(second_res_var->deref()).expect_ok();
            second_res_var->set(first_res_var->deref()).expect_ok();
            first_res_var->set(res.unwrap()).expect_ok();

            util::print("{}", jank::ui::highlight_str({ path_tmp, res.unwrap().to_code_string() }));
          }
        },
        [&](std::exception const &e) { jank::util::print_exception(e); },
        [&](jank::runtime::object_ref const e) { jank::util::print_exception(e); },
        [&](jank::error_ref const e) { jank::util::print_exception(e); },
        [&]() { jank::util::print_current_exception(); });

      util::println("");
    }
  }

  void cpp_repl()
  {
    using namespace jank;
    using namespace jank::runtime;

    {
      profile::timer const timer{ "require clojure.core" };
      __rt_ctx->load_module("clojure.core", module::origin::latest).expect_ok();
    }

    if(!util::cli::opts.target_module.empty())
    {
      profile::timer const timer{ "load main" };
      __rt_ctx->load_module(util::cli::opts.target_module, module::origin::latest).expect_ok();
      __rt_ctx->in_ns_var->deref().call(make_box<obj::symbol>(util::cli::opts.target_module));
    }

    if(jtl::terminal::is_stdin_interactive())
    {
      ic_set_history(".jank-native-repl-history", 200);
      init_isocline();
    }

    while(true)
    {
      auto const line{ read_line("cpp") };
      if(line.is_none())
      {
        break;
      }

      auto input{ line.unwrap() };
      util::trim(input);

      if(input.empty())
      {
        continue;
      }

      cpptrace::try_catch(
        [&] { __rt_ctx->jit_prc.eval_string(input); },
        [&](std::exception const &e) { jank::util::print_exception(e); },
        [&](jank::runtime::object_ref const e) { jank::util::print_exception(e); },
        [&](jank::error_ref const e) { jank::util::print_exception(e); },
        [&]() { jank::util::print_current_exception(); });
    }
  }
}
