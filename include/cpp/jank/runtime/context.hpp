#pragma once

#include <mutex>
#include <unordered_map>

#include <folly/Synchronized.h>

#include <jank/result.hpp>
#include <jank/runtime/ns.hpp>
#include <jank/runtime/var.hpp>
#include <jank/runtime/obj/keyword.hpp>

namespace jank::jit
{ struct processor; }

namespace jank::runtime
{
  struct context
  {
    context();
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

    object_ptr macroexpand1(object_ptr list);
    object_ptr macroexpand(object_ptr list);

    void eval_prelude(jit::processor const &);
    object_ptr eval_file(native_string_view const &path, jit::processor const &);
    object_ptr eval_string(native_string_view const &code, jit::processor const &);

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

      var_ptr current_ns;
      var_ptr in_ns;
      context &rt_ctx;
    };

    thread_state& get_thread_state();
    thread_state& get_thread_state(option<thread_state> init);

    folly::Synchronized<native_unordered_map<std::thread::id, thread_state>> thread_states;
  };
}
