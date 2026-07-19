#pragma once

#include <functional>

#include <jtl/ref.hpp>

namespace jank::ir
{
  struct function;
  struct instruction;
  using identifier = jtl::immutable_string;

  using instruction_walk_function
    = std::function<void(jtl::ref<instruction>, identifier const &block)>;
  using reference_walk_function = std::function<void(identifier const &ref)>;

  void walk(ir::function const &fn, instruction_walk_function const &f);
  void walk_references(jtl::ref<instruction> const instr, reference_walk_function const &f);
}
