#pragma once

#include <unordered_map>

#include <jank/runtime/ns.hpp>
#include <jank/runtime/var.hpp>

namespace jank::runtime
{
  struct context
  {
    context() = default;
    context(context const&) = default;
    context(context &&) = default;

    void initialize();
    void dump() const;

    ns_ptr intern_ns(obj::symbol_ptr const &);

    std::unordered_map<obj::symbol_ptr, detail::box_type<ns>> namespaces;
    var_ptr current_ns;
    var_ptr in_ns;
  };
}
