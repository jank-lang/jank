#include <jank/c_api.h>
#include <jank/util/try.hpp>
#include <jank/runtime/context.hpp>
#include <jank/aot/resource.hpp>
#include <jank/util/environment.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/environment/check_health.hpp>

#include <jank/compiler_native.hpp>
#include <clojure/core_native.hpp>
#include <clojure/string_native.hpp>

extern "C"
{
  void jank_load_clojure_core();
  void jank_load_clojure_string();
  void jank_load_clojure_walk();
  void jank_load_jank_nrepl_server_core();
  void jank_load_jank_nrepl_server_inspect();
  void jank_load_jank_nrepl_server_handler();
  void jank_load_jank_nrepl_server_bencode();
  void jank_load_jank_nrepl_server_capture();
  void jank_load_jank_nrepl_server_util();
  void jank_load_jank_nrepl_server_eval();
  void jank_load_jank_nrepl_server_parsec();
  void jank_load_jank_nrepl_server_handler_close();
  void jank_load_jank_nrepl_server_handler_clone();
  void jank_load_jank_nrepl_server_handler_describe();
  void jank_load_jank_nrepl_server_handler_completions();
  void jank_load_jank_nrepl_server_handler_eval();
  void jank_load_jank_nrepl_server_handler_lookup();

  void jank_init_core_libs()
  {
    jank::util::println("jank_init_core_libs_phase_2");
    jank::runtime::__rt_ctx->module_loader.add_load_fn("clojure.core", &jank_load_clojure_core);
    jank::runtime::__rt_ctx->module_loader.add_load_fn("clojure.string", &jank_load_clojure_string);
    jank::runtime::__rt_ctx->module_loader.add_load_fn("clojure.walk", &jank_load_clojure_walk);
    jank::runtime::__rt_ctx->module_loader.add_load_fn("jank.nrepl.server.core",
                                                       &jank_load_jank_nrepl_server_core);
    jank::runtime::__rt_ctx->module_loader.add_load_fn("jank.nrepl.server.inspect",
                                                       &jank_load_jank_nrepl_server_inspect);
    jank::runtime::__rt_ctx->module_loader.add_load_fn("jank.nrepl.server.handler",
                                                       &jank_load_jank_nrepl_server_handler);
    jank::runtime::__rt_ctx->module_loader.add_load_fn("jank.nrepl.server.bencode",
                                                       &jank_load_jank_nrepl_server_bencode);
    jank::runtime::__rt_ctx->module_loader.add_load_fn("jank.nrepl.server.capture",
                                                       &jank_load_jank_nrepl_server_capture);
    jank::runtime::__rt_ctx->module_loader.add_load_fn("jank.nrepl.server.util",
                                                       &jank_load_jank_nrepl_server_util);
    jank::runtime::__rt_ctx->module_loader.add_load_fn("jank.nrepl.server.eval",
                                                       &jank_load_jank_nrepl_server_eval);
    jank::runtime::__rt_ctx->module_loader.add_load_fn("jank.nrepl.server.parsec",
                                                       &jank_load_jank_nrepl_server_parsec);
    jank::runtime::__rt_ctx->module_loader.add_load_fn("jank.nrepl.server.handler.close",
                                                       &jank_load_jank_nrepl_server_handler_close);
    jank::runtime::__rt_ctx->module_loader.add_load_fn("jank.nrepl.server.handler.clone",
                                                       &jank_load_jank_nrepl_server_handler_clone);
    jank::runtime::__rt_ctx->module_loader.add_load_fn(
      "jank.nrepl.server.handler.describe",
      &jank_load_jank_nrepl_server_handler_describe);
    jank::runtime::__rt_ctx->module_loader.add_load_fn(
      "jank.nrepl.server.handler.completions",
      &jank_load_jank_nrepl_server_handler_completions);
    jank::runtime::__rt_ctx->module_loader.add_load_fn("jank.nrepl.server.handler.eval",
                                                       &jank_load_jank_nrepl_server_handler_eval);
    jank::runtime::__rt_ctx->module_loader.add_load_fn("jank.nrepl.server.handler.lookup",
                                                       &jank_load_jank_nrepl_server_handler_lookup);
  }
}
