#include <jank/ir/processor.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::ir
{
  void rewrite(identifier &name, identifier const &old_name, identifier const &new_name)
  {
    if(name == old_name)
    {
      name = new_name;
    }
  }

  void
  rewrite(jtl::option<identifier> &name, identifier const &old_name, identifier const &new_name)
  {
    if(name.is_some() && name.unwrap() == old_name)
    {
      name = new_name;
    }
  }

  void rewrite_uses(jtl::ref<instruction> const inst,
                    identifier const &old_name,
                    identifier const &new_name)
  {
    switch(inst->kind)
    {
      case instruction_kind::nop:
      case instruction_kind::parameter:
      case instruction_kind::capture:
      case instruction_kind::literal:
      case instruction_kind::function:
      case instruction_kind::local:
      case instruction_kind::letfn:
      case instruction_kind::var_deref:
      case instruction_kind::var_ref:
      case instruction_kind::recursion_reference:
      case instruction_kind::branch_get:
      case instruction_kind::cpp_scope_open:
      case instruction_kind::cpp_scope_close:
      case instruction_kind::cpp_raw:
      case instruction_kind::cpp_value:
        break;
      case instruction_kind::persistent_list:
        {
          auto &i{ static_cast<inst::persistent_list &>(*inst.data) };
          for(auto &v : i.values)
          {
            rewrite(v, old_name, new_name);
          }
        }
        break;
      case instruction_kind::persistent_vector:
        {
          auto &i{ static_cast<inst::persistent_vector &>(*inst.data) };
          for(auto &v : i.values)
          {
            rewrite(v, old_name, new_name);
          }
        }
        break;
      case instruction_kind::persistent_array_map:
        {
          auto &i{ static_cast<inst::persistent_array_map &>(*inst.data) };
          for(auto &v : i.values)
          {
            rewrite(v.first, old_name, new_name);
            rewrite(v.second, old_name, new_name);
          }
        }
        break;
      case instruction_kind::persistent_hash_map:
        {
          auto &i{ static_cast<inst::persistent_hash_map &>(*inst.data) };
          for(auto &v : i.values)
          {
            rewrite(v.first, old_name, new_name);
            rewrite(v.second, old_name, new_name);
          }
        }
        break;
      case instruction_kind::persistent_hash_set:
        {
          auto &i{ static_cast<inst::persistent_hash_set &>(*inst.data) };
          for(auto &v : i.values)
          {
            rewrite(v, old_name, new_name);
          }
        }
        break;
      case instruction_kind::closure:
        {
          auto &i{ static_cast<inst::closure &>(*inst.data) };
          for(auto &c : i.captures)
          {
            rewrite(c.second.name, old_name, new_name);
          }
        }
        break;
      case instruction_kind::def:
        {
          auto &i{ static_cast<inst::def &>(*inst.data) };
          rewrite(i.value, old_name, new_name);
        }
        break;
      case instruction_kind::type_erase:
        {
          auto &i{ static_cast<inst::type_erase &>(*inst.data) };
          rewrite(i.value, old_name, new_name);
        }
        break;
      case instruction_kind::dynamic_call:
        {
          auto &i{ static_cast<inst::dynamic_call &>(*inst.data) };
          rewrite(i.fn, old_name, new_name);
          for(auto &arg : i.args)
          {
            rewrite(arg, old_name, new_name);
          }
        }
        break;
      case instruction_kind::named_recursion:
        {
          auto &i{ static_cast<inst::named_recursion &>(*inst.data) };
          rewrite(i.fn, old_name, new_name);
          for(auto &arg : i.args)
          {
            rewrite(arg, old_name, new_name);
          }
        }
        break;
      case instruction_kind::truthy:
        {
          auto &i{ static_cast<inst::truthy &>(*inst.data) };
          rewrite(i.value, old_name, new_name);
        }
        break;
      case instruction_kind::jump:
        {
          auto &i{ static_cast<inst::jump &>(*inst.data) };
          rewrite(i.block, old_name, new_name);
        }
        break;
      case instruction_kind::branch_set:
        {
          auto &i{ static_cast<inst::branch_set &>(*inst.data) };
          rewrite(i.shadow, old_name, new_name);
          rewrite(i.value, old_name, new_name);
        }
        break;
      case instruction_kind::branch:
        {
          auto &i{ static_cast<inst::branch &>(*inst.data) };
          if(i.shadow.is_some())
          {
            rewrite(i.shadow.unwrap().name, old_name, new_name);
          }
          rewrite(i.condition, old_name, new_name);
          rewrite(i.then_block, old_name, new_name);
          rewrite(i.else_block, old_name, new_name);
          rewrite(i.merge_block, old_name, new_name);
        }
        break;
      case instruction_kind::loop:
        {
          auto &i{ static_cast<inst::loop &>(*inst.data) };
          if(i.shadow.is_some())
          {
            rewrite(i.shadow.unwrap().name, old_name, new_name);
          }
          rewrite(i.loop_block, old_name, new_name);
          rewrite(i.merge_block, old_name, new_name);
        }
        break;
      case instruction_kind::case_:
        {
          auto &i{ static_cast<inst::case_ &>(*inst.data) };
          rewrite(i.shadow, old_name, new_name);
          for(auto &b : i.case_blocks)
          {
            rewrite(b.second, old_name, new_name);
          }
          rewrite(i.merge_block, old_name, new_name);
        }
        break;
      case instruction_kind::try_:
        {
          auto &i{ static_cast<inst::try_ &>(*inst.data) };
          rewrite(i.shadow, old_name, new_name);
          for(auto &c : i.catches)
          {
            rewrite(c.second, old_name, new_name);
          }
          rewrite(i.finally_block, old_name, new_name);
          rewrite(i.merge_block, old_name, new_name);
        }
        break;
      case instruction_kind::catch_:
        {
          auto &i{ static_cast<inst::catch_ &>(*inst.data) };
          rewrite(i.shadow, old_name, new_name);
          rewrite(i.finally_block, old_name, new_name);
          rewrite(i.merge_block, old_name, new_name);
        }
        break;
      case instruction_kind::finally:
        {
          auto &i{ static_cast<inst::finally &>(*inst.data) };
          rewrite(i.merge_block, old_name, new_name);
        }
        break;
      case instruction_kind::throw_:
        {
          auto &i{ static_cast<inst::throw_ &>(*inst.data) };
          rewrite(i.value, old_name, new_name);
        }
        break;
      case instruction_kind::ret:
        {
          auto &i{ static_cast<inst::ret &>(*inst.data) };
          rewrite(i.value, old_name, new_name);
        }
        break;
      case instruction_kind::cpp_into_object:
        {
          auto &i{ static_cast<inst::cpp_into_object &>(*inst.data) };
          rewrite(i.value, old_name, new_name);
        }
        break;
      case instruction_kind::cpp_from_object:
        {
          auto &i{ static_cast<inst::cpp_from_object &>(*inst.data) };
          rewrite(i.value, old_name, new_name);
        }
        break;
      case instruction_kind::cpp_unsafe_cast:
        {
          auto &i{ static_cast<inst::cpp_unsafe_cast &>(*inst.data) };
          rewrite(i.value, old_name, new_name);
        }
        break;
      case instruction_kind::cpp_call:
        {
          auto &i{ static_cast<inst::cpp_call &>(*inst.data) };
          rewrite(i.value, old_name, new_name);
          for(auto &arg : i.args)
          {
            rewrite(arg, old_name, new_name);
          }
        }
        break;
      case instruction_kind::cpp_constructor_call:
        {
          auto &i{ static_cast<inst::cpp_constructor_call &>(*inst.data) };
          for(auto &arg : i.args)
          {
            rewrite(arg, old_name, new_name);
          }
        }
        break;
      case instruction_kind::cpp_member_call:
        {
          auto &i{ static_cast<inst::cpp_member_call &>(*inst.data) };
          rewrite(i.value, old_name, new_name);
          for(auto &arg : i.args)
          {
            rewrite(arg, old_name, new_name);
          }
        }
        break;
      case instruction_kind::cpp_member_access:
        {
          auto &i{ static_cast<inst::cpp_member_access &>(*inst.data) };
          rewrite(i.value, old_name, new_name);
        }
        break;
      case instruction_kind::cpp_builtin_operator_call:
        {
          auto &i{ static_cast<inst::cpp_builtin_operator_call &>(*inst.data) };
          for(auto &arg : i.args)
          {
            rewrite(arg, old_name, new_name);
          }
        }
        break;
      case instruction_kind::cpp_box:
        {
          auto &i{ static_cast<inst::cpp_box &>(*inst.data) };
          rewrite(i.value, old_name, new_name);
        }
        break;
      case instruction_kind::cpp_unbox:
        {
          auto &i{ static_cast<inst::cpp_unbox &>(*inst.data) };
          rewrite(i.value, old_name, new_name);
          rewrite(i.meta, old_name, new_name);
        }
        break;
      case instruction_kind::cpp_new:
        {
          auto &i{ static_cast<inst::cpp_new &>(*inst.data) };
          rewrite(i.value, old_name, new_name);
        }
        break;
      case instruction_kind::cpp_delete:
        {
          auto &i{ static_cast<inst::cpp_delete &>(*inst.data) };
          rewrite(i.value, old_name, new_name);
        }
        break;
    }
  }

  /* Rewrite all uses of `old_name` in the function to use `new_name` instead.
   * This covers all instruction operands in every block. */
  void rewrite_uses(function const &fn, identifier const &old_name, identifier const &new_name)
  {
    for(auto const &block : fn.blocks)
    {
      for(auto const inst : block.instructions)
      {
        rewrite_uses(inst, old_name, new_name);
      }
    }
  }
}
