#pragma once

#include <mutex>
#include <unordered_map>

#include <libguarded/shared_guarded.hpp>

#include <jank/result.hpp>
#include <jank/runtime/ns.hpp>
#include <jank/runtime/var.hpp>
#include <jank/runtime/obj/keyword.hpp>

namespace jank::analyze
{ struct context; }

namespace jank::jit
{ struct processor; }

namespace jank::runtime
{
  struct context
  {
    context();
    context(context const&) = delete;
    context(context &&) = delete;

    void dump() const;

    ns_ptr intern_ns(obj::symbol_ptr const &);
    result<var_ptr, std::string> intern_var(obj::symbol_ptr const &);
    result<var_ptr, std::string> intern_var(detail::string_type const &ns, detail::string_type const &name);
    option<var_ptr> find_var(obj::symbol_ptr const &);
    option<object_ptr> find_local(obj::symbol_ptr const &);

    obj::keyword_ptr intern_keyword(obj::symbol const &sym, bool const resolved);
    obj::keyword_ptr intern_keyword(std::string_view const &ns, std::string_view const &name, bool resolved);

    void eval_prelude(analyze::context &, jit::processor const &);
    object_ptr eval_file(std::string_view const &path, analyze::context &, jit::processor const &);
    object_ptr eval_string(std::string_view const &code, analyze::context &, jit::processor const &);

    libguarded::shared_guarded<std::unordered_map<obj::symbol_ptr, detail::box_type<ns>>> namespaces;
    libguarded::shared_guarded<std::unordered_map<obj::symbol, obj::keyword_ptr>> keywords;

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

    libguarded::shared_guarded<std::unordered_map<std::thread::id, thread_state>> thread_states;
  };
}
