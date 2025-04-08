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
    using keyword_ptr = native_box<struct keyword>;
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
    context(util::cli::options const &opts);
    context(context &&) = delete;
    ~context();

    void dump() const;

    ns_ptr intern_ns(jtl::immutable_string const &);
    ns_ptr intern_ns(obj::symbol_ptr const &);
    jtl::option<ns_ptr> remove_ns(obj::symbol_ptr const &);
    /* Looks up a ns by its symbol. Does not resolve aliases. Does not intern. */
    jtl::option<ns_ptr> find_ns(obj::symbol_ptr const &);
    /* Resolves a symbol which could be an alias to its ns, based on the aliases
     * in the current ns. Does not intern. */
    jtl::option<ns_ptr> resolve_ns(obj::symbol_ptr const &);
    ns_ptr current_ns();

    /* Adds the current ns to unqualified symbols and resolves the ns of qualified symbols.
     * Does not intern. */
    obj::symbol_ptr qualify_symbol(obj::symbol_ptr const &) const;
    jtl::option<object_ptr> find_local(obj::symbol_ptr const &);

    jtl::result<var_ptr, jtl::immutable_string> intern_var(obj::symbol_ptr const &);
    jtl::result<var_ptr, jtl::immutable_string>
    intern_var(jtl::immutable_string const &ns, jtl::immutable_string const &name);
    jtl::option<var_ptr> find_var(obj::symbol_ptr const &);
    jtl::option<var_ptr>
    find_var(jtl::immutable_string const &ns, jtl::immutable_string const &name);

    jtl::result<obj::keyword_ptr, jtl::immutable_string>
    intern_keyword(jtl::immutable_string const &ns,
                   jtl::immutable_string const &name,
                   native_bool resolved = true);
    jtl::result<obj::keyword_ptr, jtl::immutable_string>
    intern_keyword(jtl::immutable_string const &s);

    object_ptr macroexpand1(object_ptr o);
    object_ptr macroexpand(object_ptr o);

    object_ptr eval_file(jtl::immutable_string const &path);
    object_ptr eval_string(native_persistent_string_view const &code);
    jtl::string_result<void> eval_cpp_string(native_persistent_string_view const &code) const;
    object_ptr read_string(native_persistent_string_view const &code);
    native_vector<analyze::expression_ref>
    analyze_string(native_persistent_string_view const &code, native_bool const eval = true);

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
    jtl::result<void, jtl::immutable_string>
    load_module(native_persistent_string_view const &module, module::origin ori);

    /* Does all the same work as load_module, but also writes compiled files to the file system. */
    jtl::result<void, jtl::immutable_string>
    compile_module(native_persistent_string_view const &module);

    object_ptr eval(object_ptr const o);

    jtl::string_result<void> write_module(jtl::immutable_string const &module_name,
                                          std::unique_ptr<llvm::Module> const &module) const;

    /* Generates a unique name for use with anything from codgen structs,
     * lifted vars, to shadowed locals. */
    jtl::immutable_string unique_string();
    jtl::immutable_string unique_string(native_persistent_string_view const &prefix);
    obj::symbol unique_symbol();
    obj::symbol unique_symbol(native_persistent_string_view const &prefix);

    folly::Synchronized<native_unordered_map<obj::symbol_ptr, ns_ptr>> namespaces;
    folly::Synchronized<native_unordered_map<jtl::immutable_string, obj::keyword_ptr>> keywords;

    struct binding_scope
    {
      binding_scope(context &rt_ctx);
      binding_scope(context &rt_ctx, obj::persistent_hash_map_ptr const bindings);
      ~binding_scope();

      context &rt_ctx;
    };

    jtl::string_result<void> push_thread_bindings();
    jtl::string_result<void> push_thread_bindings(object_ptr const bindings);
    jtl::string_result<void> push_thread_bindings(obj::persistent_hash_map_ptr const bindings);
    jtl::string_result<void> pop_thread_bindings();
    obj::persistent_hash_map_ptr get_thread_bindings() const;
    jtl::option<thread_binding_frame> current_thread_binding_frame();

    /* The analyze processor is reused across evaluations so we can keep the semantic information
     * of previous code. This is essential for REPL use. */
    /* TODO: This needs to be synchronized. */
    analyze::processor an_prc{ *this };
    jit::processor jit_prc;
    /* TODO: This needs to be a dynamic var. */
    native_unordered_map<jtl::immutable_string, native_vector<jtl::immutable_string>>
      module_dependencies;
    jtl::immutable_string binary_cache_dir;
    module::loader module_loader;

    var_ptr current_file_var{};
    var_ptr current_ns_var{};
    var_ptr in_ns_var{};
    var_ptr compile_files_var{};
    var_ptr loaded_libs_var{};
    var_ptr current_module_var{};
    var_ptr assert_var{};
    var_ptr no_recur_var{};
    var_ptr gensym_env_var{};

    static thread_local native_unordered_map<context const *, std::list<thread_binding_frame>>
      thread_binding_frames;
  };

  /* NOLINTNEXTLINE */
  extern context *__rt_ctx;
}
