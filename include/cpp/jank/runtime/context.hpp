#pragma once

#include <mutex>
#include <unordered_map>

#include <folly/Synchronized.h>

#include <jank/result.hpp>
#include <jank/detail/cli.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/runtime/module/loader.hpp>
#include <jank/runtime/ns.hpp>
#include <jank/runtime/var.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/jit/processor.hpp>

namespace jank::jit
{ struct processor; }

namespace jank::runtime
{
  struct context
  {
    context();
    context(cli_options const &opts);
    context(context const&);
    context(context &&) = delete;

    void dump() const;

    ns_ptr intern_ns(obj::symbol_ptr const &);
    result<var_ptr, native_string> intern_var(obj::symbol_ptr const &);
    result<var_ptr, native_string> intern_var(native_string const &ns, native_string const &name);
    obj::symbol_ptr qualify_symbol(obj::symbol_ptr const &);
    option<var_ptr> find_var(obj::symbol_ptr const &);
    option<object_ptr> find_local(obj::symbol_ptr const &);

    obj::keyword_ptr intern_keyword(obj::symbol const &sym, bool const resolved);
    obj::keyword_ptr intern_keyword(native_string_view const &ns, native_string_view const &name, bool resolved);

    object_ptr macroexpand1(object_ptr o);
    object_ptr macroexpand(object_ptr o);

    static object_ptr print(object_ptr o);
    static object_ptr print(object_ptr o, object_ptr more);
    static object_ptr println(object_ptr more);

    void eval_prelude();
    object_ptr eval_file(native_string_view const &path);
    object_ptr eval_string(native_string_view const &code);
    native_vector<analyze::expression_ptr> analyze_string(native_string_view const &code, native_bool const eval = true);

    /* Finds the specified module on the class path and loads it. If
     * the module is already loaded, nothing is done. */
    result<void, native_string> load_module(native_string_view const &module);

    /* Does all the same work as load_module, but also writes compiled files to the file system. */
    result<void, native_string> compile_module(native_string_view const &module);

    void write_module(native_string_view const &module, native_string_view const &contents) const;

    /* Generates a unique name for use with anything from codgen structs,
     * lifted vars, to shadowed locals. */
    static native_string unique_string();
    static native_string unique_string(native_string_view const &prefix);
    static obj::symbol unique_symbol();
    static obj::symbol unique_symbol(native_string_view const &prefix);

    folly::Synchronized<native_unordered_map<obj::symbol_ptr, ns_ptr>> namespaces;
    folly::Synchronized<native_unordered_map<obj::symbol, obj::keyword_ptr>> keywords;

    /* TODO: This can be forward declared and moved to the cpp. */
    struct thread_state
    {
      thread_state(thread_state const&) = default;
      thread_state(context &ctx);

      var_ptr current_ns{};
      var_ptr in_ns{};
      context &rt_ctx;
    };

    thread_state& get_thread_state();
    thread_state& get_thread_state(option<thread_state> init);

    folly::Synchronized<native_unordered_map<std::thread::id, thread_state>> thread_states;
    /* The analyze processor is reused across evaluations so we can keep the semantic information
     * of previous code. This is essential for REPL use. */
    /* TODO: This needs to be synchronized. */
    analyze::processor an_prc{ *this };
    jit::processor jit_prc;
    /* TODO: This needs to be a dynamic var. */
    bool compiling{};
    /* TODO: This needs to be a dynamic var. */
    native_string_view current_module;
    native_unordered_map<native_string, native_vector<native_string>> module_dependencies;
    native_string output_dir;
    module::loader module_loader;
  };
}
