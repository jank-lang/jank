#include <jank/runtime/context.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/truthy.hpp>
#include <jank/runtime/visit.hpp>

namespace jank::runtime
{
  jtl::immutable_string to_string(object_ref const o)
  {
    return visit_object([](auto const typed_o) { return typed_o->to_string(); }, o);
  }

  void to_string(char const ch, jtl::string_builder &buff)
  {
    obj::character{ ch }.to_string(buff);
  }

  void to_string(object_ref const o, jtl::string_builder &buff)
  {
    visit_object([&](auto const typed_o) { typed_o->to_string(buff); }, o);
  }

  void to_code_string(char const ch, jtl::string_builder &buff)
  {
    buff(obj::character{ ch }.to_code_string());
  }

  void to_code_string(object_ref const o, jtl::string_builder &buff)
  {
    auto const print_meta(make_box<obj::symbol>("clojure.core", "*print-meta*"));
    auto const print_meta_var(__rt_ctx->find_var(print_meta));

    if(print_meta_var.is_some() && truthy(print_meta_var->deref()))
    {
      visit_object(
        [&](auto const typed_o) {
          using T = typename jtl::decay_t<decltype(typed_o)>::value_type;

          if constexpr(behavior::metadatable<T>)
          {
            auto const meta(typed_o->get_meta());
            if(meta.is_some())
            {
              buff('^');
              to_code_string(meta, buff);
              buff(' ');
            }
          }

          buff(typed_o->to_code_string());
        },
        o);
    }
    else
    {
      visit_object([&](auto const typed_o) { buff(typed_o->to_code_string()); }, o);
    }
  }

  jtl::immutable_string to_code_string(object_ref const o)
  {
    jtl::string_builder buff;
    to_code_string(o, buff);
    return buff.release();
  }
}
