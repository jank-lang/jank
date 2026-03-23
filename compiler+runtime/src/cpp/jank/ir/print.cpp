#include <jank/ir/print.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/analyze/cpp_util.hpp>
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

  void inst::literal::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :literal :value {} :type \"{}\"}",
                    name,
                    runtime::to_code_string(value),
                    get_qualified_type_name(type));
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
                    qualified_var,
                    get_qualified_type_name(type));
  }

  void inst::var_ref::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb,
                    "{:name {} :op :var-ref :var {} :type \"{}\"}",
                    name,
                    qualified_var,
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
                    "{:name {} :op :jump :block {} :type \"{}\"}",
                    name,
                    block,
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
    util::format_to(sb,
                    "{:name {} :op :branch :condition {} :then {} :else {} :type \"{}\"}",
                    name,
                    condition,
                    then_block,
                    else_block,
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

  void print(block const &b, jtl::string_builder &sb, usize indent)
  {
    util::format_to(sb, "{:name {}\n", b.name);
    print_indent(sb, ++indent);
    util::format_to(sb, ":instructions [", b.name);
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
    util::format_to(sb, "]");
  }

  void print(function const &f, jtl::string_builder &sb, usize indent)
  {
    util::format_to(sb, "{:name {}\n", f.name);
    print_indent(sb, ++indent);
    util::format_to(sb, ":blocks [", f.name);
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

  jtl::immutable_string print(function const &fn)
  {
    jtl::string_builder sb;
    print(fn, sb, 0);
    return sb.release();
  }
}
