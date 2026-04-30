#include <jank/ir/print.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/analyze/cpp_util.hpp>
#include <jank/analyze/visit.hpp>
#include <jank/util/fmt.hpp>

namespace jank::ir
{
  using namespace analyze::cpp_util;

  usize determine_indent(jtl::string_builder const &sb)
  {
    if(sb.empty())
    {
      return 0;
    }

    auto const last_nl{ sb.view().rfind('\n') };
    if(last_nl == jtl::immutable_string_view::npos)
    {
      return sb.size();
    }

    return sb.size() - last_nl - 1;
  }

  void print_indent(jtl::string_builder &sb, usize const indent)
  {
    for(usize i{}; i < indent; ++i)
    {
      sb(' ');
    }
  }

  void inst::parameter::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :parameter :type \"{}\"}",
                    name,
                    get_qualified_type_name(type));
  }

  void inst::capture::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :capture :value {} :type \"{}\"}",
                    name,
                    value,
                    get_qualified_type_name(type));
  }

  void inst::literal::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :literal :value {} :type \"{}\"}",
                    name,
                    obj.to_code_string(),
                    get_qualified_type_name(type));
  }

  void inst::persistent_list::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb, "{:name {} :op :persistent-list :values [", name);
    bool needs_space{};
    for(auto const &value : values)
    {
      if(needs_space)
      {
        util::format_to(sb, " ");
      }
      needs_space = true;
      sb(value);
    }
    util::format_to(sb, "] :type \"{}\"}", get_qualified_type_name(type));
  }

  void inst::persistent_vector::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb, "{:name {} :op :persistent-vector :values [", name);
    bool needs_space{};
    for(auto const &value : values)
    {
      if(needs_space)
      {
        util::format_to(sb, " ");
      }
      needs_space = true;
      sb(value);
    }
    util::format_to(sb, "] :type \"{}\"}", get_qualified_type_name(type));
  }

  void inst::persistent_array_map::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb, "{:name {} :op :persistent-array-map :values [", name);
    bool needs_space{};
    for(auto const &value : values)
    {
      if(needs_space)
      {
        util::format_to(sb, " ");
      }
      needs_space = true;
      util::format_to(sb, "[{} {}]", value.first, value.second);
    }
    util::format_to(sb, "] :type \"{}\"}", get_qualified_type_name(type));
  }

  void inst::persistent_hash_map::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb, "{:name {} :op :persistent-hash-map :values [", name);
    bool needs_space{};
    for(auto const &value : values)
    {
      if(needs_space)
      {
        util::format_to(sb, " ");
      }
      needs_space = true;
      util::format_to(sb, "[{} {}]", value.first, value.second);
    }
    util::format_to(sb, "] :type \"{}\"}", get_qualified_type_name(type));
  }

  void inst::persistent_hash_set::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb, "{:name {} :op :persistent-hash-set :values [", name);
    bool needs_space{};
    for(auto const &value : values)
    {
      if(needs_space)
      {
        util::format_to(sb, " ");
      }
      needs_space = true;
      sb(value);
    }
    util::format_to(sb, "] :type \"{}\"}", get_qualified_type_name(type));
  }

  void inst::function::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb, "{:name {} :op :function :arities {", name);
    bool needs_space{};
    for(auto const &arity : arities)
    {
      if(needs_space)
      {
        util::format_to(sb, " ");
      }
      needs_space = true;
      util::format_to(sb, "{} {}", arity.first, arity.second);
    }
    util::format_to(sb,
                    "} :arity-flags {} :type \"{}\"}",
                    arity_flags,
                    get_qualified_type_name(type));
  }

  void inst::closure::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb, "{:name {} :op :closure :context {} :arities {", name, context);
    bool needs_space{};
    for(auto const &arity : arities)
    {
      if(needs_space)
      {
        util::format_to(sb, " ");
      }
      needs_space = true;
      util::format_to(sb, "{} {}", arity.first, arity.second);
    }
    util::format_to(sb, "} :captures {");

    needs_space = false;
    for(auto const &capture : captures)
    {
      if(needs_space)
      {
        util::format_to(sb, " ");
      }
      needs_space = true;
      util::format_to(sb,
                      "{} {:name {} :type {}}",
                      capture.first,
                      capture.second.name,
                      get_qualified_type_name(capture.second.type));
    }
    util::format_to(sb,
                    "} :arity-flags {} :type \"{}\"}",
                    arity_flags,
                    get_qualified_type_name(type));
  }

  void inst::letfn::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb, "{:name {} :op :letfn :bindings [", name);
    bool needs_space{};
    for(auto const &binding : bindings)
    {
      if(needs_space)
      {
        util::format_to(sb, " ");
      }
      needs_space = true;
      sb(binding);
    }
    util::format_to(sb, "] :type \"{}\"}", get_qualified_type_name(type));
  }

  void inst::def::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb, "{:name {} :op :def :var {}", name, qualified_var);
    if(value.is_some())
    {
      util::format_to(sb, " :value {}", value.unwrap());
    }
    util::format_to(sb, " :meta {} :type \"{}\"}", meta, get_qualified_type_name(type));
  }

  void inst::var_deref::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :var-deref :var {} :type \"{}\"}",
                    name,
                    var,
                    get_qualified_type_name(type));
  }

  void inst::var_ref::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :var-ref :var {} :type \"{}\"}",
                    name,
                    var,
                    get_qualified_type_name(type));
  }

  void inst::type_erase::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :type-erase :value {} :type \"{}\"}",
                    name,
                    value,
                    get_qualified_type_name(type));
  }

  void inst::dynamic_call::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb, "{:name {} :op :dynamic-call :fn {} :args [", name, fn);
    bool needs_space{};
    for(auto const &arg : args)
    {
      if(needs_space)
      {
        util::format_to(sb, " ");
      }
      needs_space = true;
      sb(arg);
    }
    util::format_to(sb, "] :type \"{}\"}", get_qualified_type_name(type));
  }

  void inst::named_recursion::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb, "{:name {} :op :named-recursion :fn {} :args [", name, fn);
    bool needs_space{};
    for(auto const &arg : args)
    {
      if(needs_space)
      {
        util::format_to(sb, " ");
      }
      needs_space = true;
      sb(arg);
    }
    util::format_to(sb, "] :type \"{}\"}", get_qualified_type_name(type));
  }

  void inst::recursion_reference::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :recursion-reference :type \"{}\"}",
                    name,
                    get_qualified_type_name(type));
  }

  void inst::truthy::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :truthy :value {} :type \"{}\"}",
                    name,
                    value,
                    get_qualified_type_name(type));
  }

  void inst::jump::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :jump :block {} :loop {} :type \"{}\"}",
                    name,
                    block,
                    loop,
                    get_qualified_type_name(type));
  }

  void inst::branch_set::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :branch-set :shadow {} :value {} :type \"{}\"}",
                    name,
                    shadow,
                    value,
                    get_qualified_type_name(type));
  }

  void inst::branch_get::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :branch-get :type \"{}\"}",
                    name,
                    get_qualified_type_name(type));
  }

  void inst::branch::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(
      sb,
      "{:name {} :op :branch :condition {} :then {} :else {} :merge {} :shadow {} :type \"{}\"}",
      name,
      condition,
      then_block,
      else_block,
      merge_block.unwrap_or("nil"),
      (shadow.is_some() ? shadow.unwrap().name : "nil"),
      get_qualified_type_name(type));
  }

  void inst::loop::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :loop :loop-block {} :merge {} :shadow {} :shadows {",
                    name,
                    loop_block,
                    merge_block.unwrap_or("nil"),
                    (shadow.is_some() ? shadow.unwrap().name : "nil"));
    bool needs_space{};
    for(auto const &s : binding_shadows)
    {
      if(needs_space)
      {
        util::format_to(sb, " ");
      }
      needs_space = true;
      util::format_to(sb,
                      "{:name {} :value {} :type \"{}\"}",
                      s.name,
                      s.value,
                      get_qualified_type_name(s.type));
    }
    util::format_to(sb, "} :type \"{}\"}", get_qualified_type_name(type));
  }

  void inst::case_::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :case :shift {} :mask {} :value {} :case-blocks [",
                    name,
                    shift,
                    mask,
                    value);
    bool needs_space{};
    for(auto const &c : case_blocks)
    {
      if(needs_space)
      {
        util::format_to(sb, " ");
      }
      needs_space = true;
      util::format_to(sb, "{:value {} :block {}}", c.first, c.second);
    }
    util::format_to(sb,
                    "] :default-block {} :merge-block {} :shadow {} :type \"{}\"}",
                    default_block,
                    merge_block.unwrap_or("nil"),
                    shadow.unwrap_or("nil"),
                    get_qualified_type_name(type));
  }

  void inst::try_::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb, "{:name {} :op :try :catches [", name);
    bool needs_space{};
    for(auto const &catch_ : catches)
    {
      if(needs_space)
      {
        util::format_to(sb, " ");
      }
      needs_space = true;
      util::format_to(sb,
                      "{:type \"{}\" :block {}}",
                      get_qualified_type_name(catch_.first),
                      catch_.second);
    }
    util::format_to(sb,
                    "] :merge {} :shadow {} :finally {} :type \"{}\"}",
                    merge_block,
                    shadow,
                    finally_block.unwrap_or("nil"),
                    get_qualified_type_name(type));
  }

  void inst::catch_::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :catch :merge {} :shadow {} :type \"{}\"}",
                    name,
                    merge_block.unwrap_or("nil"),
                    shadow.unwrap_or("nil"),
                    get_qualified_type_name(type));
  }

  void inst::finally::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :finally :merge {} :type \"{}\"}",
                    name,
                    merge_block,
                    get_qualified_type_name(type));
  }

  void inst::throw_::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :throw :value {} :type \"{}\"}",
                    name,
                    value,
                    get_qualified_type_name(type));
  }

  void inst::ret::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :ret :value {} :type \"{}\"}",
                    name,
                    value,
                    get_qualified_type_name(type));
  }

  void inst::cpp_raw::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :cpp/raw :type \"{}\"}",
                    name,
                    get_qualified_type_name(type));
  }

  void inst::cpp_value::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    R"({:name {} :op :cpp/value :scope "{}" :type "{}"})",
                    name,
                    get_qualified_name(expr->scope),
                    get_qualified_type_name(type));
  }

  void inst::cpp_into_object::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :cpp/into-object :value {} :type \"{}\"}",
                    name,
                    value,
                    get_qualified_type_name(type));
  }

  void inst::cpp_from_object::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :cpp/from-object :value {} :type \"{}\"}",
                    name,
                    value,
                    get_qualified_type_name(type));
  }

  void inst::cpp_unsafe_cast::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :cpp/unsafe-cast :value {} :type \"{}\"}",
                    name,
                    value,
                    get_qualified_type_name(type));
  }

  void inst::cpp_call::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb, "{:name {} :op :cpp/call :value {} :args [", name, value.unwrap_or("nil"));
    bool needs_space{};
    for(auto const &arg : args)
    {
      if(needs_space)
      {
        util::format_to(sb, " ");
      }
      needs_space = true;
      sb(arg);
    }
    util::format_to(sb, "] :type \"{}\"}", get_qualified_type_name(type));
  }

  void inst::cpp_constructor_call::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb, "{:name {} :op :cpp/constructor-call :args [", name);
    bool needs_space{};
    for(auto const &arg : args)
    {
      if(needs_space)
      {
        util::format_to(sb, " ");
      }
      needs_space = true;
      sb(arg);
    }
    util::format_to(sb, "] :type \"{}\"}", get_qualified_type_name(type));
  }

  void inst::cpp_member_call::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :cpp/member-call :fn \"{}\" :args [",
                    name,
                    get_qualified_name(expr->fn));
    bool needs_space{};
    for(auto const &arg : args)
    {
      if(needs_space)
      {
        util::format_to(sb, " ");
      }
      needs_space = true;
      sb(arg);
    }
    util::format_to(sb, "] :type \"{}\"}", get_qualified_type_name(type));
  }

  void inst::cpp_member_access::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    R"({:name {} :op :cpp/member-access :value {} :member "{}" :type "{}"})",
                    name,
                    value,
                    get_qualified_name(expr->scope),
                    get_qualified_type_name(type));
  }

  void inst::cpp_builtin_operator_call::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :cpp/builtin-operator-call :op \"{}\" :args [",
                    name,
                    operator_name(static_cast<Cpp::Operator>(expr->op)).unwrap());
    bool needs_space{};
    for(auto const &arg : args)
    {
      if(needs_space)
      {
        util::format_to(sb, " ");
      }
      needs_space = true;
      sb(arg);
    }
    util::format_to(sb, "] :type \"{}\"}", get_qualified_type_name(type));
  }

  void inst::cpp_box::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    R"({:name {} :op :cpp/box :value {} :type "{}"})",
                    name,
                    value,
                    get_qualified_type_name(type));
  }

  void inst::cpp_unbox::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    R"({:name {} :op :cpp/unbox :value {} :type "{}"})",
                    name,
                    value,
                    get_qualified_type_name(type));
  }

  void inst::cpp_new::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    R"({:name {} :op :cpp/new :value {} :type "{}"})",
                    name,
                    value,
                    get_qualified_type_name(type));
  }

  void inst::cpp_delete::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    R"({:name {} :op :cpp/delete :value {} :type "{}"})",
                    name,
                    value,
                    get_qualified_type_name(type));
  }

  void print(block const &b, jtl::string_builder &sb, usize indent)
  {
    util::format_to(sb, "{:name {}\n", b.name);
    print_indent(sb, ++indent);
    util::format_to(sb, ":instructions [");
    indent = determine_indent(sb);
    bool needs_indent{};
    for(auto const &i : b.instructions)
    {
      if(needs_indent)
      {
        sb('\n');
        print_indent(sb, indent);
      }
      needs_indent = true;
      i->print(sb, indent);
    }
    util::format_to(sb, "]}");
  }

  void print(function const &f, jtl::string_builder &sb, usize indent)
  {
    util::format_to(sb, "{:name {}\n", f.name);
    print_indent(sb, ++indent);
    util::format_to(sb, ":blocks [");
    indent = determine_indent(sb);
    bool needs_indent{};
    for(auto const &b : f.blocks)
    {
      if(needs_indent)
      {
        sb('\n');
        print_indent(sb, indent);
      }
      needs_indent = true;
      print(b, sb, indent);
    }
    util::format_to(sb, "]}");
  }

  void print(module const &mod, jtl::string_builder &sb, usize indent)
  {
    util::format_to(sb, "{:name {}\n", mod.name);
    print_indent(sb, ++indent);
    util::format_to(sb, ":lifted-vars {");
    {
      auto const indent{ determine_indent(sb) };
      bool needs_indent{};
      for(auto const &v : mod.lifted_vars)
      {
        if(needs_indent)
        {
          sb('\n');
          print_indent(sb, indent);
        }
        needs_indent = true;
        util::format_to(sb, "{} {}", v.first, v.second.qualified_var);
      }
    }
    util::format_to(sb, "}\n");
    print_indent(sb, indent);
    util::format_to(sb, ":lifted-constants {");
    {
      auto const indent{ determine_indent(sb) };
      bool needs_indent{};
      for(auto const &c : mod.lifted_constants)
      {
        if(needs_indent)
        {
          sb('\n');
          print_indent(sb, indent);
        }
        needs_indent = true;
        util::format_to(sb, "{} {}", c.first, c.second.to_code_string());
      }
    }
    util::format_to(sb, "}\n");
    print_indent(sb, indent);
    util::format_to(sb, ":functions [");
    {
      auto const indent{ determine_indent(sb) };
      bool needs_indent{};
      for(auto const &f : mod.functions)
      {
        if(needs_indent)
        {
          sb('\n');
          print_indent(sb, indent);
        }
        needs_indent = true;
        print(f, sb, indent);
      }
    }
    util::format_to(sb, "]}");
  }

  jtl::immutable_string print(function const &fn)
  {
    jtl::string_builder sb;
    print(fn, sb, 0);
    return sb.release();
  }

  jtl::immutable_string print(module const &mod)
  {
    jtl::string_builder sb;
    print(mod, sb, 0);
    return sb.release();
  }
}
