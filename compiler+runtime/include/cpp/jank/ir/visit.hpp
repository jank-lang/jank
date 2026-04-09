#pragma once

#include <jank/ir/instruction.hpp>

namespace jank::ir
{
  template <typename F, typename... Args>
  auto visit_inst(F const &f, jtl::ref<instruction> const i, Args &&...args)
  {
    switch(i->kind)
    {
      case instruction_kind::parameter:
        return f(jtl::static_ref_cast<inst::parameter>(i), std::forward<Args>(args)...);
      case instruction_kind::capture:
        return f(jtl::static_ref_cast<inst::capture>(i), std::forward<Args>(args)...);
      case instruction_kind::literal:
        return f(jtl::static_ref_cast<inst::literal>(i), std::forward<Args>(args)...);
      case instruction_kind::persistent_list:
        return f(jtl::static_ref_cast<inst::persistent_list>(i), std::forward<Args>(args)...);
      case instruction_kind::persistent_vector:
        return f(jtl::static_ref_cast<inst::persistent_vector>(i), std::forward<Args>(args)...);
      case instruction_kind::persistent_array_map:
        return f(jtl::static_ref_cast<inst::persistent_array_map>(i), std::forward<Args>(args)...);
      case instruction_kind::persistent_hash_map:
        return f(jtl::static_ref_cast<inst::persistent_hash_map>(i), std::forward<Args>(args)...);
      case instruction_kind::persistent_hash_set:
        return f(jtl::static_ref_cast<inst::persistent_hash_set>(i), std::forward<Args>(args)...);
      case instruction_kind::function:
        return f(jtl::static_ref_cast<inst::function>(i), std::forward<Args>(args)...);
      case instruction_kind::closure:
        return f(jtl::static_ref_cast<inst::closure>(i), std::forward<Args>(args)...);
      case instruction_kind::letfn:
        return f(jtl::static_ref_cast<inst::letfn>(i), std::forward<Args>(args)...);
      case instruction_kind::def:
        return f(jtl::static_ref_cast<inst::def>(i), std::forward<Args>(args)...);
      case instruction_kind::var_deref:
        return f(jtl::static_ref_cast<inst::var_deref>(i), std::forward<Args>(args)...);
      case instruction_kind::var_ref:
        return f(jtl::static_ref_cast<inst::var_ref>(i), std::forward<Args>(args)...);
      case instruction_kind::dynamic_call:
        return f(jtl::static_ref_cast<inst::dynamic_call>(i), std::forward<Args>(args)...);
      case instruction_kind::named_recursion:
        return f(jtl::static_ref_cast<inst::named_recursion>(i), std::forward<Args>(args)...);
      case instruction_kind::recursion_reference:
        return f(jtl::static_ref_cast<inst::recursion_reference>(i), std::forward<Args>(args)...);
      case instruction_kind::truthy:
        return f(jtl::static_ref_cast<inst::truthy>(i), std::forward<Args>(args)...);
      case instruction_kind::jump:
        return f(jtl::static_ref_cast<inst::jump>(i), std::forward<Args>(args)...);
      case instruction_kind::branch_set:
        return f(jtl::static_ref_cast<inst::branch_set>(i), std::forward<Args>(args)...);
      case instruction_kind::branch_get:
        return f(jtl::static_ref_cast<inst::branch_get>(i), std::forward<Args>(args)...);
      case instruction_kind::branch:
        return f(jtl::static_ref_cast<inst::branch>(i), std::forward<Args>(args)...);
      case instruction_kind::loop:
        return f(jtl::static_ref_cast<inst::loop>(i), std::forward<Args>(args)...);
      case instruction_kind::case_:
        return f(jtl::static_ref_cast<inst::case_>(i), std::forward<Args>(args)...);
      case instruction_kind::try_:
        return f(jtl::static_ref_cast<inst::try_>(i), std::forward<Args>(args)...);
      case instruction_kind::catch_:
        return f(jtl::static_ref_cast<inst::catch_>(i), std::forward<Args>(args)...);
      case instruction_kind::finally:
        return f(jtl::static_ref_cast<inst::finally>(i), std::forward<Args>(args)...);
      case instruction_kind::throw_:
        return f(jtl::static_ref_cast<inst::throw_>(i), std::forward<Args>(args)...);
      case instruction_kind::ret:
        return f(jtl::static_ref_cast<inst::ret>(i), std::forward<Args>(args)...);
      case instruction_kind::cpp_raw:
        return f(jtl::static_ref_cast<inst::cpp_raw>(i), std::forward<Args>(args)...);
      case instruction_kind::cpp_value:
        return f(jtl::static_ref_cast<inst::cpp_value>(i), std::forward<Args>(args)...);
      case instruction_kind::cpp_into_object:
        return f(jtl::static_ref_cast<inst::cpp_into_object>(i), std::forward<Args>(args)...);
      case instruction_kind::cpp_from_object:
        return f(jtl::static_ref_cast<inst::cpp_from_object>(i), std::forward<Args>(args)...);
      case instruction_kind::cpp_unsafe_cast:
        return f(jtl::static_ref_cast<inst::cpp_unsafe_cast>(i), std::forward<Args>(args)...);
      case instruction_kind::cpp_call:
        return f(jtl::static_ref_cast<inst::cpp_call>(i), std::forward<Args>(args)...);
      case instruction_kind::cpp_constructor_call:
        return f(jtl::static_ref_cast<inst::cpp_constructor_call>(i), std::forward<Args>(args)...);
      case instruction_kind::cpp_member_call:
        return f(jtl::static_ref_cast<inst::cpp_member_call>(i), std::forward<Args>(args)...);
      case instruction_kind::cpp_member_access:
        return f(jtl::static_ref_cast<inst::cpp_member_access>(i), std::forward<Args>(args)...);
      case instruction_kind::cpp_builtin_operator_call:
        return f(jtl::static_ref_cast<inst::cpp_builtin_operator_call>(i),
                 std::forward<Args>(args)...);
      case instruction_kind::cpp_box:
        return f(jtl::static_ref_cast<inst::cpp_box>(i), std::forward<Args>(args)...);
      case instruction_kind::cpp_unbox:
        return f(jtl::static_ref_cast<inst::cpp_unbox>(i), std::forward<Args>(args)...);
      case instruction_kind::cpp_new:
        return f(jtl::static_ref_cast<inst::cpp_new>(i), std::forward<Args>(args)...);
      case instruction_kind::cpp_delete:
        return f(jtl::static_ref_cast<inst::cpp_delete>(i), std::forward<Args>(args)...);
      default:
        break;
    }

    throw std::runtime_error{ "Invalid instruction kind: "
                              + std::to_string(static_cast<int>(i->kind)) };
  }
}
