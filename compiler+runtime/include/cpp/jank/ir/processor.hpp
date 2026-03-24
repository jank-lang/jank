#pragma once

#include <jtl/ref.hpp>
#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>
#include <jank/ir/instruction.hpp>

namespace jtl
{
  struct immutable_string;
  struct string_builder;
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
  struct block
  {
    bool has_terminator() const;

    usize index{};
    identifier name;
    native_vector<jtl::ref<instruction>> instructions;
  };

  struct function
  {
    usize add_block(identifier const &name);
    void remove_block(usize const index);

    jtl::ref<analyze::expr::function_arity> arity;
    identifier name{};
    native_vector<block> blocks{};
  };

  native_vector<function> create(analyze::expr::function_ref,
                                 jtl::immutable_string const &module,
                                 codegen::compilation_target target);
}
