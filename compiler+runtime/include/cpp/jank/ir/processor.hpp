#pragma once

#include <jtl/ref.hpp>
#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>
#include <jank/ir/instruction.hpp>

namespace jtl
{
  struct immutable_string;
}

namespace jank::analyze::expr
{
  using function_ref = jtl::ref<struct function>;
  struct function_arity;
}

namespace jank::codegen
{
  enum class compilation_target : u8;
}

namespace jank::ir
{
  /* An IR block is a sequence of instructions, which must be terminated with a special
   * terminator instruction (like jump, ret, throw, etc). */
  struct block
  {
    bool has_terminator() const;

    usize index{};
    identifier name;
    native_vector<jtl::ref<instruction>> instructions;
  };

  /* An IR function is a single arity of a jank function, which will be compiled to a C function.
   * It's composed of one or more blocks of instructions. */
  struct function
  {
    usize add_block(identifier const &name);
    void remove_block(usize const index);
    usize find_block(identifier const &name) const;

    jtl::ref<analyze::expr::function_arity> arity;
    identifier name{};
    native_vector<block> blocks{};
  };

  /* An IR module corresponds to a single compilation, which could mean one top-level
   * jank function, in the case of eval, or a whole namespace, in the case of
   * AOT compilation.
   *
   * The `entry_points` are the IR functions which correspond with the primary function we're
   * dealing with. In a normal eval case, the primary function is jank function we're compiling.
   * It may have multiple IR functions for it, though, if it has multiple arities. If there
   * are any nested functions within our primary function, they will end up in the module. In
   * the AOT compilation case, the entry point will be the `jank_load_foo` function and all
   * other functions will be everything else in the whole namespace.
   *
   * IR modules track lifted and deduped constants and vars, but leave it up to codegen for
   * how to handle them. */
  struct module
  {
    jtl::immutable_string name{};
    codegen::compilation_target target{};
    runtime::callable_arity_flags arity_flags{};
    analyze::expr::function_ref root_fn_expr;
    native_vector<jtl::immutable_string> entry_points{};
    native_vector<function> functions{};
    native_unordered_map<identifier, runtime::object_ref> lifted_constants{};

    struct lifted_var
    {
      jtl::immutable_string qualified_var;
      bool owned{};
    };

    native_unordered_map<identifier, lifted_var> lifted_vars{};
  };

  module create(analyze::expr::function_ref,
                jtl::immutable_string const &module_name,
                codegen::compilation_target target);
}
