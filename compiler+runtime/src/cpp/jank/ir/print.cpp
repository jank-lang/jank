#include <jank/ir/print.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/util/fmt.hpp>

namespace jank::ir
{
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
    util::format_to(sb, "{:name {} :op :literal :value {}}", name, runtime::to_code_string(value));
  }

  void inst::def::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb, "{:name {} :op :def :var {}", name, qualified_var);
    if(value.is_some())
    {
      util::format_to(sb, " :value {}", value.unwrap());
    }
    util::format_to(sb, " :meta {}}", meta);
  }

  void inst::var_deref::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb, "{:name {} :op :var-deref :var {}}", name, qualified_var);
  }

  void inst::var_ref::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb, "{:name {} :op :var-ref :var {}}", name, qualified_var);
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
    util::format_to(sb, "]}");
  }

  void inst::ret::print(jtl::string_builder &sb, usize const) const
  {
    util::format_to(sb, "{:op :ret :value {}}", value);
  }

  void print(block const &b, jtl::string_builder &sb, usize indent)
  {
    util::format_to(sb, "{:name {}\n", b.name);
    print_indent(sb, ++indent);
    util::format_to(sb, ":blocks [", b.name);
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
