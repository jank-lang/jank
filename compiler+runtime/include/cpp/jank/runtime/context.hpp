#pragma once

#include <list>

#include <folly/Synchronized.h>

#include <jtl/result.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/runtime/module/loader.hpp>
#include <jank/runtime/ns.hpp>
#include <jank/runtime/var.hpp>
#include <jank/jit/processor.hpp>
#include <jank/util/cli.hpp>

namespace jank
{
  namespace jit
  {
    struct processor;
  }

  namespace codegen
  {
    struct reusable_context;
  }
}

namespace jank::runtime
{
  namespace obj
  {
    using keyword_ref = oref<struct keyword>;
  }

  /* This is a singleton, as much as I fought it for actual years. Trying to have multiple
   * contexts is limited firstly by there being a single, global JIT compilation context
   * and process in which global memory exists. Secondly, by the fact that interned keywords
   * from one context will not play nicely with others. Thirdly that we want the context
   * anywhere and everywhere, without passing it around, so we can do things like intern
   * keywords. So this guy is always initialized in main and you can always use it. */
  struct context
  {
    context();
    context(context const &) = delete;
    context(context &&) noexcept = delete;

    ns_ref intern_ns(jtl::immutable_string const &);
    ns_ref intern_ns(obj::symbol_ref const);
    ns_ref remove_ns(obj::symbol_ref const);
    /* Looks up a ns by its symbol. Does not resolve aliases. Does not intern. */
    ns_ref find_ns(obj::symbol_ref const);
    /* Resolves a symbol which could be an alias to its ns, based on the aliases
     * in the current ns. Does not intern. */
    ns_ref resolve_ns(obj::symbol_ref const);
    ns_ref current_ns() const;

    /* Adds the current ns to unqualified symbols and resolves the ns of qualified symbols.
     * Does not intern. */
    obj::symbol_ref qualify_symbol(obj::symbol_ref const) const;
    jtl::option<object_ref> find_local(obj::symbol_ref const);

    jtl::result<var_ref, jtl::immutable_string> intern_var(obj::symbol_ref const qualified_name);
    jtl::result<var_ref, jtl::immutable_string> intern_var(jtl::immutable_string const &);
    jtl::result<var_ref, jtl::immutable_string>
    intern_var(jtl::immutable_string const &ns, jtl::immutable_string const &name);
    jtl::result<var_ref, jtl::immutable_string> intern_owned_var(obj::symbol_ref const);
    jtl::result<var_ref, jtl::immutable_string> intern_owned_var(jtl::immutable_string const &);
    jtl::result<var_ref, jtl::immutable_string>
    intern_owned_var(jtl::immutable_string const &ns, jtl::immutable_string const &name);
    var_ref find_var(obj::symbol_ref const);
    var_ref find_var(jtl::immutable_string const &ns, jtl::immutable_string const &name);

    jtl::result<obj::keyword_ref, jtl::immutable_string>
    intern_keyword(jtl::immutable_string const &ns,
                   jtl::immutable_string const &name,
                   bool resolved = true);
    jtl::result<obj::keyword_ref, jtl::immutable_string>
    intern_keyword(jtl::immutable_string const &s);

    object_ref macroexpand1(object_ref const o);
    object_ref macroexpand(object_ref const o);

    jtl::option<object_ref> eval_file(jtl::immutable_string const &path);
    jtl::option<object_ref> eval_string(jtl::immutable_string const &code);
    jtl::result<void, error_ref> eval_cpp_string(jtl::immutable_string const &code) const;
    object_ref read_string(jtl::immutable_string const &code);
    native_vector<analyze::expression_ref>
    analyze_string(jtl::immutable_string const &code, bool const eval = true);

    /* Finds the specified module on the module path and loads it. If
     * the module is already loaded, nothing is done.
     *
     * Modules are considered absolute if they begin with a forward slash. Otherwise, they
     * are considered relative to the current namespace.
     *
     * For example, if the current namespace is foo.bar, then:
     *
     * Module /meow.cat refers to module meow.cat
     * Module meow.cat refers to foo.bar$meow.cat
     */
    jtl::result<void, error_ref>
    load_module(jtl::immutable_string const &module, module::origin ori);

    /* Does all the same work as load_module, but also writes compiled files to the file system. */
    jtl::result<void, error_ref> compile_module(jtl::immutable_string const &module);

    object_ref eval(object_ref const o);

    jtl::immutable_string get_output_module_name(jtl::immutable_string const &module_name) const;
    jtl::string_result<void> write_module(jtl::immutable_string const &module_name,
                                          jtl::immutable_string const &cpp_code,
                                          jtl::ref<llvm::Module> const &module) const;

    /* Generates a unique name for use with anything from codgen structs,
     * lifted vars, to shadowed locals. Prefixes with current namespace. */
    jtl::immutable_string unique_namespaced_string() const;
    jtl::immutable_string unique_namespaced_string(jtl::immutable_string const &prefix) const;
    jtl::immutable_string unique_string() const;
    jtl::immutable_string unique_string(jtl::immutable_string const &prefix) const;
    obj::symbol_ref unique_symbol() const;
    obj::symbol_ref unique_symbol(jtl::immutable_string const &prefix) const;

    folly::Synchronized<native_unordered_map<obj::symbol_ref, ns_ref>> namespaces;
    folly::Synchronized<native_unordered_map<jtl::immutable_string, obj::keyword_ref>> keywords;

    struct binding_scope
    {
      binding_scope();
      binding_scope(obj::persistent_hash_map_ref const bindings);
      ~binding_scope();
    };

    jtl::string_result<void> push_thread_bindings();
    jtl::string_result<void> push_thread_bindings(object_ref const bindings);
    jtl::string_result<void> push_thread_bindings(obj::persistent_hash_map_ref const bindings);
    jtl::string_result<void> pop_thread_bindings();
    obj::persistent_hash_map_ref get_thread_bindings() const;
    jtl::option<thread_binding_frame> current_thread_binding_frame();

    /* The analyze processor is reused across evaluations so we can keep the semantic information
     * of previous code. This is essential for REPL use.
     *
     * TODO: Is it? I think we can remove this. */
    /* TODO: This needs to be synchronized, if it's kept. */
    analyze::processor an_prc;
    jtl::immutable_string binary_version;
    /* TODO: This needs to be a dynamic var. */
    native_unordered_map<jtl::immutable_string, native_vector<jtl::immutable_string>>
      module_dependencies;
    folly::Synchronized<native_deque<jtl::immutable_string>> loaded_modules_in_order;
    jtl::immutable_string binary_cache_dir;
    module::loader module_loader;

    var_ref current_file_var;
    var_ref current_ns_var;
    var_ref in_ns_var;
    var_ref compile_files_var;
    var_ref loaded_libs_var;
    var_ref current_module_var;
    var_ref assert_var;
    var_ref no_recur_var;
    var_ref gensym_env_var;

    static thread_local native_list<thread_binding_frame> thread_binding_frames;

    /* This must go last, since it'll try to access other bits in the runtime context during
     * its initialization and we need them to be ready. */
    jit::processor jit_prc;
  };

  /* NOLINTNEXTLINE */
  extern context *__rt_ctx;
}
