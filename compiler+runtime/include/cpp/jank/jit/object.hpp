#pragma once

#include <string>

#include <folly/Synchronized.h>

#include <cpptrace/cpptrace.hpp>

#include <jtl/option.hpp>

/* In order to get great symbolicated stack traces, with source information, we need to
 * keep track of both stack frame and debug information for JIT compiled/loaded code. How
 * we do this varies, depending on whether or not we JIT compiled the code this session
 * or loaded an object file of AOT compiled code.
 *
 * In the object file scenario, each symbol within the object file is lazily resolved by
 * LLVM, so we don't just have all the info we need at any particular point. This is good,
 * for performance, but it means that we need to mimic the laziness on our side, too. We
 * do this by installing a custom ObjectLinkingLayer plugin which tracks when symbols from
 * object files are materialized. We then update the state in this code, which keeps
 * track of enough to be able to symbolicate stack traces when needed. */
namespace jank::jit
{
  /* Describes one lazily materialized object-file-backed frame after we've mapped a runtime
   * PC back to its original `.o` file location. The formatter uses this to ask cpptrace to do
   * ordinary on-disk DWARF resolution against the backing object file. */
  struct materialized_object_frame
  {
    uptr runtime_address{};
    usize runtime_size{};
    std::string object_path;
    uptr object_address{};
    usize object_size{};
    std::string symbol;
  };

  /* Symbol metadata recorded from the original object file before ORC materializes anything. */
  struct loaded_object_symbol
  {
    uptr object_address{};
    usize object_size{};
  };

  /* Bookkeeping for a single object file added through `load_object`. This is keyed by the
   * ORC resource key so we can join it later with the symbols that JITLink actually emits. */
  struct loaded_object
  {
    std::string path;
    std::unordered_map<std::string, loaded_object_symbol> symbols;
  };

  void install_object_tracking_plugin();
  void register_loaded_object(uptr resource_key, loaded_object const &object);
  jtl::option<materialized_object_frame> find_materialized_object_frame(uptr raw_address);

  cpptrace::stacktrace_frame resolve_materialized_object_frame(cpptrace::stacktrace_frame frame);
  void refresh_jit_objects();
}
