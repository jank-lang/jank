#include <jank/ir/walk.hpp>
#include <jank/ir/processor.hpp>
#include <jank/ir/visit.hpp>

namespace jank::ir
{
  struct state
  {
    void enter_block(identifier const &block)
    {
      block_index = fn->find_block(block);
      instruction_index = 0;
      seen_blocks.emplace(block);
    }

    identifier current_block() const
    {
      return fn->blocks[block_index].name;
    }

    void next_instruction()
    {
      ++instruction_index;
    }

    jtl::ref<function> fn;
    usize block_index{};
    usize instruction_index{};
    native_set<identifier> seen_blocks{};
  };

  void walk(jtl::ref<instruction> const instr, instruction_walk_function const &f, state &s);

  void walk_until_jump(jtl::option<identifier> const &jump_block,
                       instruction_walk_function const &f,
                       state &s)
  {
    while(s.instruction_index < s.fn->blocks[s.block_index].instructions.size())
    {
      auto const current_inst{ s.fn->blocks[s.block_index].instructions[s.instruction_index] };

      if(current_inst->kind == ir::instruction_kind::jump)
      {
        auto const jump{ runtime::static_box_cast<ir::inst::jump>(current_inst) };
        if(s.seen_blocks.contains(jump->block))
        {
          walk(current_inst, f, s);
          break;
        }
      }

      walk(current_inst, f, s);

      if(current_inst->kind == ir::instruction_kind::jump)
      {
        auto const jump{ runtime::static_box_cast<ir::inst::jump>(current_inst) };
        if(jump_block.is_some() && jump->block == jump_block.unwrap())
        {
          break;
        }
      }
      else if(current_inst->kind == ir::instruction_kind::ret
              || current_inst->kind == ir::instruction_kind::throw_)
      {
        break;
      }
    }
  }

  void walk_typed(ir::inst::def_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::var_deref_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::var_ref_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void
  walk_typed(ir::inst::type_erase_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void
  walk_typed(ir::inst::dynamic_call_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::literal_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::persistent_list_ref const instr,
                  instruction_walk_function const &f,
                  state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::persistent_vector_ref const instr,
                  instruction_walk_function const &f,
                  state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::persistent_array_map_ref const instr,
                  instruction_walk_function const &f,
                  state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::persistent_hash_map_ref const instr,
                  instruction_walk_function const &f,
                  state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::persistent_hash_set_ref const instr,
                  instruction_walk_function const &f,
                  state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::function_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::closure_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::nop_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::parameter_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::capture_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::recursion_reference_ref const instr,
                  instruction_walk_function const &f,
                  state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::named_recursion_ref const instr,
                  instruction_walk_function const &f,
                  state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::letfn_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::local_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::set_local_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::jump_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());

    /* If there are any instructions after the jump, like scope closes, we need to handle
     * them, too. */
    while(s.instruction_index < s.fn->blocks[s.block_index].instructions.size())
    {
      walk(s.fn->blocks[s.block_index].instructions[s.instruction_index], f, s);
    }

    if(s.seen_blocks.contains(instr->block))
    {
      return;
    }
    s.enter_block(instr->block);
  }

  void walk_typed(ir::inst::truthy_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void
  walk_typed(ir::inst::branch_get_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void
  walk_typed(ir::inst::branch_set_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::branch_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
    s.enter_block(instr->then_block);
    walk_until_jump(instr->merge_block, f, s);

    s.enter_block(instr->else_block);
    walk_until_jump(instr->merge_block, f, s);

    s.enter_block(instr->merge_block);
  }

  void walk_typed(ir::inst::loop_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());

    s.enter_block(instr->loop_block);
    walk_until_jump(instr->merge_block, f, s);

    if(instr->merge_block.is_some())
    {
      s.enter_block(instr->merge_block.unwrap());
    }
  }

  void walk_typed(ir::inst::throw_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::try_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());

    auto const has_finally{ instr->finally_block.is_some() };

    if(has_finally)
    {
      auto const instruction_index{ s.instruction_index };
      auto const block_index{ s.block_index };
      s.enter_block(instr->finally_block.unwrap());

      walk(s.fn->blocks[s.block_index].instructions[s.instruction_index], f, s);

      s.instruction_index = instruction_index;
      s.block_index = block_index;
    }

    auto const &jump_block{ has_finally ? instr->finally_block : instr->merge_block };

    walk_until_jump(jump_block, f, s);

    for(auto const &catch_details : instr->catches)
    {
      s.enter_block(catch_details.second);
      walk(s.fn->blocks[s.block_index].instructions[s.instruction_index], f, s);
    }

    s.enter_block(instr->merge_block);
  }

  void walk_typed(ir::inst::catch_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());

    auto const &jump_block{ instr->finally_block.is_some() ? instr->finally_block
                                                           : instr->merge_block };
    walk_until_jump(jump_block, f, s);
  }

  void walk_typed(ir::inst::finally_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
    walk_until_jump(instr->merge_block, f, s);
  }

  void walk_typed(ir::inst::case_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());

    for(auto const &case_block : instr->case_blocks)
    {
      s.enter_block(case_block.second);
      walk_until_jump(instr->merge_block, f, s);
    }

    s.enter_block(instr->default_block);
    walk_until_jump(instr->merge_block, f, s);

    s.enter_block(instr->merge_block);
  }

  void walk_typed(ir::inst::ret_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void
  walk_typed(ir::inst::cpp_scope_open_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::cpp_scope_close_ref const instr,
                  instruction_walk_function const &f,
                  state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::cpp_raw_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::cpp_value_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::cpp_into_object_ref const instr,
                  instruction_walk_function const &f,
                  state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::cpp_from_object_ref const instr,
                  instruction_walk_function const &f,
                  state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::cpp_unsafe_cast_ref const instr,
                  instruction_walk_function const &f,
                  state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::cpp_call_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::cpp_constructor_call_ref const instr,
                  instruction_walk_function const &f,
                  state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::cpp_member_call_ref const instr,
                  instruction_walk_function const &f,
                  state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::cpp_member_access_ref const instr,
                  instruction_walk_function const &f,
                  state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::cpp_builtin_operator_call_ref const instr,
                  instruction_walk_function const &f,
                  state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::cpp_box_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::cpp_unbox_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk_typed(ir::inst::cpp_new_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void
  walk_typed(ir::inst::cpp_delete_ref const instr, instruction_walk_function const &f, state &s)
  {
    s.next_instruction();
    f(instr, s.current_block());
  }

  void walk(jtl::ref<instruction> const instr, instruction_walk_function const &f, state &s)
  {
    ir::visit_inst([&](auto const typed_inst) { walk_typed(typed_inst, f, s); }, instr);
  }

  void walk(ir::function const &fn, instruction_walk_function const &f, state &s)
  {
    while(s.instruction_index < fn.blocks[s.block_index].instructions.size())
    {
      walk(fn.blocks[s.block_index].instructions[s.instruction_index], f, s);
    }
  }

  void walk(ir::function const &fn, instruction_walk_function const &f)
  {
    state s{ &fn };
    walk(fn, f, s);
  }

  void walk_references_typed(ir::inst::def_ref const instr, reference_walk_function const &f)
  {
    if(instr->value.is_some())
    {
      f(instr->value.unwrap());
    }
  }

  void walk_references_typed(ir::inst::var_deref_ref const, reference_walk_function const &)
  {
  }

  void walk_references_typed(ir::inst::var_ref_ref const, reference_walk_function const &)
  {
  }

  void walk_references_typed(ir::inst::type_erase_ref const instr, reference_walk_function const &f)
  {
    f(instr->value);
  }

  void
  walk_references_typed(ir::inst::dynamic_call_ref const instr, reference_walk_function const &f)
  {
    f(instr->fn);
    for(auto const &arg : instr->args)
    {
      f(arg);
    }
  }

  void walk_references_typed(ir::inst::literal_ref const, reference_walk_function const &)
  {
  }

  void
  walk_references_typed(ir::inst::persistent_list_ref const instr, reference_walk_function const &f)
  {
    for(auto const &arg : instr->values)
    {
      f(arg);
    }
  }

  void walk_references_typed(ir::inst::persistent_vector_ref const instr,
                             reference_walk_function const &f)
  {
    for(auto const &arg : instr->values)
    {
      f(arg);
    }
  }

  void walk_references_typed(ir::inst::persistent_array_map_ref const instr,
                             reference_walk_function const &f)
  {
    for(auto const &arg : instr->values)
    {
      f(arg.first);
      f(arg.second);
    }
  }

  void walk_references_typed(ir::inst::persistent_hash_map_ref const instr,
                             reference_walk_function const &f)
  {
    for(auto const &arg : instr->values)
    {
      f(arg.first);
      f(arg.second);
    }
  }

  void walk_references_typed(ir::inst::persistent_hash_set_ref const instr,
                             reference_walk_function const &f)
  {
    for(auto const &arg : instr->values)
    {
      f(arg);
    }
  }

  void walk_references_typed(ir::inst::function_ref const, reference_walk_function const &)
  {
  }

  void walk_references_typed(ir::inst::closure_ref const, reference_walk_function const &)
  {
  }

  void walk_references_typed(ir::inst::nop_ref const, reference_walk_function const &)
  {
  }

  void walk_references_typed(ir::inst::parameter_ref const, reference_walk_function const &)
  {
  }

  void walk_references_typed(ir::inst::capture_ref const, reference_walk_function const &)
  {
  }

  void
  walk_references_typed(ir::inst::recursion_reference_ref const, reference_walk_function const &)
  {
  }

  void walk_references_typed(ir::inst::named_recursion_ref const, reference_walk_function const &)
  {
  }

  void walk_references_typed(ir::inst::letfn_ref const, reference_walk_function const &)
  {
  }

  void walk_references_typed(ir::inst::local_ref const, reference_walk_function const &)
  {
  }

  void walk_references_typed(ir::inst::set_local_ref const instr, reference_walk_function const &f)
  {
    f(instr->value);
  }

  void walk_references_typed(ir::inst::jump_ref const, reference_walk_function const &)
  {
  }

  void walk_references_typed(ir::inst::truthy_ref const instr, reference_walk_function const &f)
  {
    f(instr->value);
  }

  void walk_references_typed(ir::inst::branch_get_ref const instr, reference_walk_function const &f)
  {
    f(instr->name);
  }

  void walk_references_typed(ir::inst::branch_set_ref const instr, reference_walk_function const &f)
  {
    f(instr->value);
  }

  void walk_references_typed(ir::inst::branch_ref const instr, reference_walk_function const &f)
  {
    f(instr->condition);
  }

  void walk_references_typed(ir::inst::loop_ref const instr, reference_walk_function const &f)
  {
    for(auto const &shadow : instr->binding_shadows)
    {
      f(shadow.value);
    }
  }

  void walk_references_typed(ir::inst::throw_ref const instr, reference_walk_function const &f)
  {
    f(instr->value);
  }

  void walk_references_typed(ir::inst::try_ref const, reference_walk_function const &)
  {
  }

  void walk_references_typed(ir::inst::catch_ref const, reference_walk_function const &)
  {
  }

  void walk_references_typed(ir::inst::finally_ref const, reference_walk_function const &)
  {
  }

  void walk_references_typed(ir::inst::case_ref const instr, reference_walk_function const &f)
  {
    f(instr->value);
  }

  void walk_references_typed(ir::inst::ret_ref const instr, reference_walk_function const &f)
  {
    f(instr->value);
  }

  void walk_references_typed(ir::inst::cpp_scope_open_ref const, reference_walk_function const &)
  {
  }

  void walk_references_typed(ir::inst::cpp_scope_close_ref const, reference_walk_function const &)
  {
  }

  void walk_references_typed(ir::inst::cpp_raw_ref const, reference_walk_function const &)
  {
  }

  void walk_references_typed(ir::inst::cpp_value_ref const, reference_walk_function const &)
  {
  }

  void
  walk_references_typed(ir::inst::cpp_into_object_ref const instr, reference_walk_function const &f)
  {
    f(instr->value);
  }

  void
  walk_references_typed(ir::inst::cpp_from_object_ref const instr, reference_walk_function const &f)
  {
    f(instr->value);
  }

  void
  walk_references_typed(ir::inst::cpp_unsafe_cast_ref const instr, reference_walk_function const &f)
  {
    f(instr->value);
  }

  void walk_references_typed(ir::inst::cpp_call_ref const instr, reference_walk_function const &f)
  {
    if(instr->value.is_some())
    {
      f(instr->value.unwrap());
    }
    for(auto const &arg : instr->args)
    {
      f(arg);
    }
  }

  void walk_references_typed(ir::inst::cpp_constructor_call_ref const instr,
                             reference_walk_function const &f)
  {
    for(auto const &arg : instr->args)
    {
      f(arg);
    }
  }

  void
  walk_references_typed(ir::inst::cpp_member_call_ref const instr, reference_walk_function const &f)
  {
    for(auto const &arg : instr->args)
    {
      f(arg);
    }
  }

  void walk_references_typed(ir::inst::cpp_member_access_ref const instr,
                             reference_walk_function const &f)
  {
    f(instr->value);
  }

  void walk_references_typed(ir::inst::cpp_builtin_operator_call_ref const instr,
                             reference_walk_function const &f)
  {
    for(auto const &arg : instr->args)
    {
      f(arg);
    }
  }

  void walk_references_typed(ir::inst::cpp_box_ref const instr, reference_walk_function const &f)
  {
    f(instr->value);
  }

  void walk_references_typed(ir::inst::cpp_unbox_ref const instr, reference_walk_function const &f)
  {
    f(instr->value);
  }

  void walk_references_typed(ir::inst::cpp_new_ref const instr, reference_walk_function const &f)
  {
    f(instr->value);
  }

  void walk_references_typed(ir::inst::cpp_delete_ref const instr, reference_walk_function const &f)
  {
    f(instr->value);
  }

  void walk_references(jtl::ref<instruction> const instr, reference_walk_function const &f)
  {
    ir::visit_inst([&](auto const typed_inst) { walk_references_typed(typed_inst, f); }, instr);
  }
}
