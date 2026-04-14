#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/var.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/obj/persistent_string.hpp>

jank::runtime::var_ref _jank_var(char const *sym);
jank::runtime::var_ref _jank_var_owned(char const *sym);
jank::runtime::obj::keyword_ref _jank_keyword(char const * const ns, char const * const name);
jank::runtime::obj::symbol_ref _jank_symbol(char const * const ns, char const * const name);
jank::runtime::obj::symbol_ref
_jank_symbol(jank::runtime::object_ref const meta, char const * const ns, char const * const name);
jank::runtime::obj::persistent_string_ref _jank_string(char const * const s);
jank::runtime::object_ref _jank_read(char const *edn);
