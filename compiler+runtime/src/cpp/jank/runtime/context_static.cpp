#include <jank/runtime/context.hpp>
#include <jank/error/runtime.hpp>

namespace jank::runtime
{
  jtl::option<object_ref> context::eval_file(jtl::immutable_string const &)
  {
    throw error::runtime_static_feature_disabled("eval_file");
  }

  jtl::option<object_ref>
  context::eval_string(jtl::immutable_string const &, read::source_position const &) const
  {
    throw error::runtime_static_feature_disabled("eval_string");
  }

  jtl::option<object_ref> context::eval_string(jtl::immutable_string const &) const
  {
    throw error::runtime_static_feature_disabled("eval_string");
  }

  jtl::result<void, error_ref> context::eval_cpp_string(jtl::immutable_string const &) const
  {
    throw error::runtime_static_feature_disabled("eval_cpp_string");
  }

  native_vector<analyze::expression_ref>
  context::analyze_string(jtl::immutable_string const &, bool const)
  {
    throw error::runtime_static_feature_disabled("analyze_string");
  }

  object_ref context::read_file(jtl::immutable_string const &, object_ref const)
  {
    throw error::runtime_static_feature_disabled("read_file");
  }

  jtl::result<void, error_ref> context::compile_module(jtl::immutable_string const &)
  {
    throw error::runtime_static_feature_disabled("compile_module");
  }

  object_ref context::eval(object_ref const)
  {
    throw error::runtime_static_feature_disabled("eval");
  }

  jtl::immutable_string context::get_output_module_name(jtl::immutable_string const &) const
  {
    throw error::runtime_static_feature_disabled("get_output_module_name");
  }

  jtl::string_result<void>
  context::write_module(jtl::immutable_string const &, jtl::immutable_string const &) const
  {
    throw error::runtime_static_feature_disabled("write_module");
  }
}
