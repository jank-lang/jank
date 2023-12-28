#include <iostream>
#include <filesystem>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <cling/Interpreter/Interpreter.h>
#include <cling/Interpreter/Value.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Preprocessor.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <folly/FBString.h>

#include <jank/util/mapped_file.hpp>
#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/codegen/processor.hpp>
#include <jank/evaluate.hpp>
#include <jank/jit/processor.hpp>
#include <jank/native_persistent_string.hpp>

namespace jank
{
  void run(util::cli::options const &opts, runtime::context &rt_ctx)
  {
    static_cast<void>(opts);
    static_cast<void>(rt_ctx);
    //{
    //  profile::timer timer{ "require clojure.core" };
    //  rt_ctx.load_module("/clojure.core").expect_ok();
    //}

    //{
    //  profile::timer timer{ "eval user code" };
    //  std::cout << runtime::detail::to_string(rt_ctx.eval_file(opts.target_file)) << std::endl;
    //}

    {
      ankerl::nanobench::Config config;
      config.mMinEpochIterations = 10000000;
      config.mOut = &std::cout;
      config.mWarmup = 10000;

      auto small("foo");
      auto medium("p0aeoka13scfq4ufg27xlse0y07gjg9v29nonktptjd36jnmlfzpze4qaxztkewq8v36hivq7ieuecvjhp9myn52ubvplrq7ip62oj7qo0n2s8xqgaxc38n70jo3cwdq");

      using std_string = std::basic_string<char, std::char_traits<char>, native_allocator<char>>;

      std_string small_std{ small };
      native_persistent_string small_persistent{ small };
      folly::fbstring small_folly{ small };
      std_string medium_std{ medium };
      native_persistent_string medium_persistent{ medium };
      folly::fbstring medium_folly{ medium };

      ankerl::nanobench::Bench().config(config).run
      (
        "native_persistent_string small allocation",
        [&]
        {
          native_persistent_string ret{ small };
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "std_string small allocation",
        [&]
        {
          std_string ret{ small };
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "folly::string small allocation",
        [&]
        {
          folly::fbstring ret{ small };
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "native_persistent_string medium allocation",
        [&]
        {
          native_persistent_string ret{ medium };
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "std_string medium allocation",
        [&]
        {
          std_string ret{ medium };
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "folly::string medium allocation",
        [&]
        {
          folly::fbstring ret{ medium };
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      /*** Copy ctor ***/

      ankerl::nanobench::Bench().config(config).run
      (
        "native_persistent_string small copy ctor",
        [&]
        {
          native_persistent_string ret{ small_persistent };
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "std_string small copy ctor",
        [&]
        {
          std_string ret{ small_std };
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "folly::string small copy ctor",
        [&]
        {
          folly::fbstring ret{ small_folly };
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "native_persistent_string medium copy ctor",
        [&]
        {
          native_persistent_string ret{ medium_persistent };
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "std_string medium copy ctor",
        [&]
        {
          std_string ret{ medium_std };
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "folly::string medium copy ctor",
        [&]
        {
          folly::fbstring ret{ medium_folly };
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      /*** Find ***/

      ankerl::nanobench::Bench().config(config).run
      (
        "native_persistent_string small find",
        [&]
        {
          auto const ret(small_persistent.find("fo"));
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "std_string small find",
        [&]
        {
          auto const ret(small_std.find("fo"));
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "folly::string small find",
        [&]
        {
          auto const ret(small_folly.find("fo"));
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "native_persistent_string medium find",
        [&]
        {
          auto const ret(medium_persistent.find("kewq"));
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "std_string medium find",
        [&]
        {
          auto const ret(medium_std.find("kewq"));
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "folly::string medium find",
        [&]
        {
          auto const ret(medium_folly.find("kewq"));
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      /*** Substrings. ***/

      ankerl::nanobench::Bench().config(config).run
      (
        "native_persistent_string small substr",
        [&]
        {
          auto const ret(small_persistent.substr(1));
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "std_string small substr",
        [&]
        {
          auto const ret(small_std.substr(1));
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "folly_string small substr",
        [&]
        {
          auto const ret(small_folly.substr(1));
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "native_persistent_string medium substr",
        [&]
        {
          auto const ret(medium_persistent.substr(4, 100));
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "std_string medium substr",
        [&]
        {
          auto const ret(medium_std.substr(4, 100));
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "folly_string medium substr",
        [&]
        {
          auto const ret(medium_folly.substr(4, 100));
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      /*** Comparisons. ***/

      std_string another_medium_std{ medium };
      native_persistent_string another_medium_persistent{ medium };
      folly::fbstring another_medium_folly{ medium };

      auto different_medium("p0aeoka13scfq4ufg27xlse0y07gjg9v29nonktptjd36jnmlfzpze4qaxztkewq8v36hivq7ieuecvjhp9myn52ubvplrq7ip62oj7qo0n2s8xqgaxc38nXXXXXXXXX");
      std_string different_medium_std{ different_medium };
      native_persistent_string different_medium_persistent{ different_medium };
      folly::fbstring different_medium_folly{ different_medium };

      ankerl::nanobench::Bench().config(config).run
      (
        "native_persistent_string medium compare same",
        [&]
        {
          auto const ret(medium_persistent == another_medium_persistent);
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "std_string medium compare same",
        [&]
        {
          auto const ret(medium_std == another_medium_std);
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "folly_string medium compare same",
        [&]
        {
          auto const ret(medium_folly == another_medium_folly);
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "native_persistent_string medium compare different",
        [&]
        {
          auto const ret(medium_persistent == different_medium_persistent);
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "std_string medium compare different",
        [&]
        {
          auto const ret(medium_std == different_medium_std);
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );

      ankerl::nanobench::Bench().config(config).run
      (
        "folly_string medium compare different",
        [&]
        {
          auto const ret(medium_folly == different_medium_folly);
          ankerl::nanobench::doNotOptimizeAway(ret);
        }
      );
    }
  }

  void compile(util::cli::options const &opts, runtime::context &rt_ctx)
  {
    //rt_ctx.load_module("/clojure.core").expect_ok();
    rt_ctx.compile_module(opts.target_ns).expect_ok();
  }

  void repl(util::cli::options const &opts, runtime::context &rt_ctx)
  {
    /* TODO: REPL server. */
    if(opts.repl_server)
    { throw std::runtime_error{ "Not yet implemented: REPL server" }; }

    {
      profile::timer timer{ "require clojure.core" };
      rt_ctx.load_module("/clojure.core").expect_ok();
    }

    /* By default, RL will do tab completion for files. We disable that here. */
    rl_bind_key('\t', rl_insert);

    /* TODO: Completion. */
    /* TODO: Syntax highlighting. */
    /* TODO: Multi-line input. */
    while(auto const buf = readline("> "))
    {
      std::string line{ buf };
      boost::trim(line);
      if(line.empty())
      { continue; }

      /* TODO: Persist history to disk, á la .lein-repl-history. */
      /* History is persisted for this session only. */
      add_history(line.data());
      try
      {
        auto const res(rt_ctx.eval_string(line));
        fmt::println("{}", runtime::detail::to_string(res));
      }
      /* TODO: Unify error handling. JEEZE! */
      catch(std::exception const &e)
      { fmt::println("Exception: {}", e.what()); }
      catch(jank::runtime::object_ptr const o)
      { fmt::println("Exception: {}", jank::runtime::detail::to_string(o)); }
      catch(jank::native_persistent_string const &s)
      { fmt::println("Exception: {}", s); }
      catch(jank::read::error const &e)
      { fmt::println("Read error: {}", e.message); }
    }
  }
}

// NOLINTNEXTLINE(bugprone-exception-escape): This can only happen if we fail to report an error.
int main(int const argc, char const **argv)
try
{
  using namespace jank;

  /* The GC needs to enabled even before arg parsing, since our native types,
   * like strings, use the GC for allocations. It can still be configured later. */
  GC_set_all_interior_pointers(1);
  GC_enable();

  auto const parse_result(util::cli::parse(argc, argv));
  if(parse_result.is_err())
  { return parse_result.expect_err(); }
  auto const &opts(parse_result.expect_ok());

  if(opts.gc_incremental)
  { GC_enable_incremental(); }

  profile::configure(opts);
  profile::timer timer{ "main" };

  runtime::context rt_ctx{ opts };

  switch(opts.command)
  {
    case util::cli::command::run:
      run(opts, rt_ctx);
      break;
    case util::cli::command::compile:
      compile(opts, rt_ctx);
      break;
    case util::cli::command::repl:
      repl(opts, rt_ctx);
      break;
  }
}
/* TODO: Unify error handling. JEEZE! */
catch(std::exception const &e)
{ fmt::println("Exception: {}", e.what()); }
catch(jank::runtime::object_ptr const o)
{ fmt::println("Exception: {}", jank::runtime::detail::to_string(o)); }
catch(jank::native_persistent_string const &s)
{ fmt::println("Exception: {}", s); }
catch(jank::read::error const &e)
{ fmt::println("Read error: {}", e.message); }
