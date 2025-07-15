#include <jtl/immutable_string.hpp>

#include <clojure/core_native.hpp>

#include <jank/runtime/context.hpp>
#include <jank/runtime/convert/function.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/equal.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/util/fmt.hpp>

using jank_object_ref = void *;

using namespace jank;
using namespace jank::runtime;

static object_ref greet_str(object_ref name)
{
  auto const s_obj(try_object<obj::persistent_string>(name));
  auto const new_str{ "Hello from cpp module, " + s_obj->to_string() + "!"};
  return make_box(new_str).erase();
}

extern "C" jank_object_ref jank_load_my_lib()
{
  auto const ns_name{ "my-lib" };
  auto const ns(__rt_ctx->intern_ns(ns_name));
  __rt_ctx->current_ns_var->set(ns).expect_ok();


  auto const intern_fn([=](jtl::immutable_string const &name, auto const fn) {
    ns->intern_var(name)->bind_root(
      make_box<obj::native_function_wrapper>(convert_function(fn))
        ->with_meta(obj::persistent_hash_map::create_unique(std::make_pair(
          __rt_ctx->intern_keyword("name").expect_ok(),
          make_box(obj::symbol{ __rt_ctx->current_ns()->to_string(), name }.to_string())))));
  });

  intern_fn("greet-str", &greet_str);

  __rt_ctx->module_loader.set_is_loaded(ns_name);
  std::cout<<"Loaded '"<<ns_name<<"\n";
  return jank_nil.erase();
}
