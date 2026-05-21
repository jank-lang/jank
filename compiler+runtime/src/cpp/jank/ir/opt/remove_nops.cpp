#include <jank/ir/processor.hpp>

namespace jank::ir
{
  void remove_nops(function &fn)
  {
    for(auto &block : fn.blocks)
    {
      for(usize i{}; i < block.instructions.size();)
      {
        auto const inst{ block.instructions[i] };
        if(inst->kind == instruction_kind::nop)
        {
          block.instructions.erase(block.instructions.begin() + i);
          continue;
        }
        ++i;
      }
    }
  }
}
