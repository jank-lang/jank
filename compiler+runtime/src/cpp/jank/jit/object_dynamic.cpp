#include <map>
#include <unordered_map>
#include <unordered_set>
#include <type_traits>
#include <vector>

#include <llvm/ExecutionEngine/JITLink/JITLink.h>

#include <CppInterOp/Compatibility.h>

#include <cpptrace/gdb_jit.hpp>

#include <jank/jit/object.hpp>
#include <jank/runtime/context.hpp>

namespace jank::jit
{
  struct object_tracker
  {
    /* LLVM's GDB-JIT integration publishes the real emitted debug object for each materialization
     * through `__jit_debug_descriptor.relevant_entry`. We mirror those registrations into cpptrace
     * incrementally and keep just enough per-resource state to unregister them later. */
    struct jit_debug_object
    {
      char const *object_start{};
    };

    /* Runtime symbol metadata captured from JITLink once ORC assigns final addresses. This is the
     * bridge between a raw PC in a stack trace and the original object-file symbol metadata above. */
    struct materialized_symbol
    {
      uptr resource_key{};
      uptr runtime_address{};
      usize runtime_size{};
      std::string object_path;
      uptr object_address{};
      usize object_size{};
      std::string symbol;
    };

    void register_loaded_object(uptr resource_key, loaded_object const &object);
    void transfer_loaded_object(uptr dst_resource_key, uptr src_resource_key);
    void remove_loaded_object(uptr resource_key);
    void register_emitted_jit_debug_object(uptr resource_key);
    void remove_jit_debug_objects(uptr resource_key);
    void transfer_jit_debug_objects(uptr dst_resource_key, uptr src_resource_key);
    void record_materialized_symbol(uptr resource_key,
                                    std::string const &symbol,
                                    uptr runtime_address,
                                    usize runtime_size);
    void remove_materialized_symbols(uptr resource_key);
    void transfer_materialized_symbols(uptr dst_resource_key, uptr src_resource_key);
    jtl::option<materialized_object_frame> find_materialized_object_frame(uptr raw_address) const;

    /* Resource key -> emitted debug objects already registered with cpptrace, plus a flat set of
     * every registered `object_start` for O(1) dedup checks. Both members are updated together,
     * so they're wrapped in a single `Synchronized` to keep them consistent under one lock. */
    struct jit_debug_object_state
    {
      std::unordered_map<uptr, std::vector<jit_debug_object>> objects_by_resource_key;
      std::unordered_set<char const *> registered_object_starts;
    };

    /* Resource key -> original object file metadata captured at `load_object` time. */
    folly::Synchronized<std::unordered_map<uptr, loaded_object>> loaded_objects;
    folly::Synchronized<jit_debug_object_state> jit_debug_objects;
    /* Runtime symbol start address -> materialized symbol metadata captured from JITLink. */
    folly::Synchronized<std::map<uptr, materialized_symbol>> materialized_symbols;
  };

  object_tracker &global_tracker()
  {
    static object_tracker tracker;
    return tracker;
  }

  static loaded_object_symbol const *
  find_loaded_object_symbol(loaded_object const &object, std::string const &symbol)
  {
    if(auto const it{ object.symbols.find(symbol) }; it != object.symbols.end())
    {
      return &it->second;
    }

#if defined(__APPLE__)
    /* Mach-O object files commonly spell external symbols with a leading underscore in the
     * object-file symbol table, while JITLink may report the same symbol without it. Treat
     * those as equivalent so lazy `.o` frame resolution can join ORC runtime symbols back to
     * their original on-disk DWARF entries. */
    if(!symbol.empty() && symbol[0] == '_')
    {
      if(auto const it{ object.symbols.find(symbol.substr(1)) }; it != object.symbols.end())
      {
        return &it->second;
      }
    }
    else
    {
      std::string underscored_symbol{ "_" };
      underscored_symbol += symbol;
      if(auto const it{ object.symbols.find(underscored_symbol) }; it != object.symbols.end())
      {
        return &it->second;
      }
    }
#endif

    return nullptr;
  }

  /* ORC associates every materialization with a resource key. We use that key as the stable join
   * point between:
   * 1. the original object-file symbol table collected in `load_object`, and
   * 2. the final runtime addresses reported by JITLink when those symbols are emitted. */
  static jtl::option<uptr> get_resource_key(llvm::orc::MaterializationResponsibility const &mr)
  {
    uptr ret{};
    auto err{ mr.withResourceKeyDo([&ret](llvm::orc::ResourceKey const key) -> llvm::Error {
      ret = key;
      return llvm::Error::success();
    }) };
    if(err)
    {
      llvm::consumeError(jtl::move(err));
      return none;
    }
    return ret;
  }

  /* This plugin is the core of the lazy `.o` stack-trace path. `load_object` only tells ORC about
   * a relocatable object file; actual code emission can happen much later and only for the symbols
   * that are needed. JITLink's post-allocation hook is the first place where we can reliably see
   * the final runtime addresses and sizes for those emitted symbols. */
  struct object_tracking_plugin : public llvm::orc::ObjectLinkingLayer::Plugin
  {
    object_tracking_plugin(object_tracker &tracker)
      : tracker{ tracker }
    {
    }

    void modifyPassConfig(llvm::orc::MaterializationResponsibility &mr,
                          llvm::jitlink::LinkGraph &,
                          llvm::jitlink::PassConfiguration &config) override
    {
      auto const resource_key{ get_resource_key(mr) };
      if(resource_key.is_none())
      {
        return;
      }

      config.PostAllocationPasses.emplace_back([this, resource_key = resource_key.unwrap()](
                                                 llvm::jitlink::LinkGraph &graph) -> llvm::Error {
        using U = std::underlying_type_t<llvm::orc::MemProt>;
        for(auto *symbol : graph.defined_symbols())
        {
          /* We only care about executable symbols here, since these are the ones that can appear
           * as stack frames. */
          if(!symbol->hasName()
             || (static_cast<U>(symbol->getSection().getMemProt())
                 & static_cast<U>(llvm::orc::MemProt::Exec))
               == static_cast<U>(llvm::orc::MemProt::None))
          {
            continue;
          }

          tracker.record_materialized_symbol(resource_key,
                                            (*symbol->getName()).str(),
                                            symbol->getAddress().getValue(),
                                            symbol->getSize());
        }

        return llvm::Error::success();
      });
    }

    llvm::Error notifyEmitted(llvm::orc::MaterializationResponsibility &mr) override
    {
      if(auto const resource_key{ get_resource_key(mr) }; resource_key.is_some())
      {
        tracker.register_emitted_jit_debug_object(resource_key.unwrap());
      }
      return llvm::Error::success();
    }

    llvm::Error notifyFailed(llvm::orc::MaterializationResponsibility &mr) override
    {
      if(auto const resource_key{ get_resource_key(mr) }; resource_key.is_some())
      {
        tracker.remove_loaded_object(resource_key.unwrap());
        tracker.remove_jit_debug_objects(resource_key.unwrap());
        tracker.remove_materialized_symbols(resource_key.unwrap());
      }
      return llvm::Error::success();
    }

    llvm::Error
    notifyRemovingResources(llvm::orc::JITDylib &, llvm::orc::ResourceKey const key) override
    {
      tracker.remove_loaded_object(key);
      tracker.remove_jit_debug_objects(key);
      tracker.remove_materialized_symbols(key);
      return llvm::Error::success();
    }

    void notifyTransferringResources(llvm::orc::JITDylib &,
                                     llvm::orc::ResourceKey const dst_key,
                                     llvm::orc::ResourceKey const src_key) override
    {
      tracker.transfer_loaded_object(dst_key, src_key);
      tracker.transfer_jit_debug_objects(dst_key, src_key);
      tracker.transfer_materialized_symbols(dst_key, src_key);
    }

    object_tracker &tracker;
  };

  void install_object_tracking_plugin()
  {
    auto const ee{ runtime::__rt_ctx->jit_prc.interpreter->getExecutionEngine() };
    auto &ol{ ee->getObjLinkingLayer() };
    auto &oll{ llvm::cast<llvm::orc::ObjectLinkingLayer>(ol) };
    oll.addPlugin(std::make_shared<object_tracking_plugin>(global_tracker()));
  }

  void object_tracker::register_loaded_object(uptr const resource_key, loaded_object const &object)
  {
    auto loaded_objects_guard{ loaded_objects.wlock() };
    (*loaded_objects_guard)[resource_key] = object;
  }

  void
  object_tracker::transfer_loaded_object(uptr const dst_resource_key, uptr const src_resource_key)
  {
    auto loaded_objects_guard{ loaded_objects.wlock() };
    auto const it{ loaded_objects_guard->find(src_resource_key) };
    if(it == loaded_objects_guard->end())
    {
      return;
    }
    (*loaded_objects_guard)[dst_resource_key] = jtl::move(it->second);
    loaded_objects_guard->erase(it);
  }

  void object_tracker::remove_loaded_object(uptr const resource_key)
  {
    loaded_objects.wlock()->erase(resource_key);
  }

  void object_tracker::register_emitted_jit_debug_object(uptr const resource_key)
  {
    auto jit_debug_objects_guard{ jit_debug_objects.wlock() };
    auto &objects{ jit_debug_objects_guard->objects_by_resource_key[resource_key] };
    auto &registered_object_starts{ jit_debug_objects_guard->registered_object_starts };
    usize registered_count{};

    auto const register_entry{ [&](cpptrace::detail::jit_code_entry const &entry) -> bool {
      if(entry.symfile_addr == nullptr || entry.symfile_size == 0
         || registered_object_starts.contains(entry.symfile_addr))
      {
        return false;
      }

      /* We call cpptrace's public `register_jit_object` (declared in `cpptrace/basic.hpp`,
       * transitively included above) rather than reaching into `cpptrace::detail`. It's a thin,
       * stable wrapper around the same detail-namespaced implementation. */
      cpptrace::register_jit_object(entry.symfile_addr, entry.symfile_size);
      objects.emplace_back(jit_debug_object{ entry.symfile_addr });
      registered_object_starts.insert(entry.symfile_addr);
      ++registered_count;
      return true;
    } };

    auto const register_contiguous_unseen_entries
     = [&](cpptrace::detail::jit_code_entry const *entry) {
     for(auto *current{ entry }; current != nullptr; current = current->next_entry)
     {
       if(current->symfile_addr == nullptr || current->symfile_size == 0)
       {
         continue;
       }
       if(registered_object_starts.contains(current->symfile_addr))
       {
         break;
       }
       register_entry(*current);
     }
    };

    auto const action_flag{ cpptrace::detail::__jit_debug_descriptor.action_flag };
    auto * const relevant_entry{ cpptrace::detail::__jit_debug_descriptor.relevant_entry };
    if(action_flag == cpptrace::detail::JIT_REGISTER_FN && relevant_entry != nullptr
      && register_entry(*relevant_entry))
    {
     /* Mach-O debugger support can publish more than one new GDB-JIT entry before ORC calls our
      * callback. Once we have positively identified the current emission via `relevant_entry`,
      * consume any additional unseen head entries from the same burst as well. */
     register_contiguous_unseen_entries(cpptrace::detail::__jit_debug_descriptor.first_entry);
     return;
    }

    /* If callback ordering means `relevant_entry` is not usable here, fall back to the current
    * GDB-JIT list and register the newest unseen emitted object. LLVM prepends new entries, so
    * the first unseen node in the list is our best incremental fallback. If there are multiple
    * adjacent unseen head entries, they're part of the same unpublished burst, so mirror all of
    * them before returning. */
    for(auto *entry{ cpptrace::detail::__jit_debug_descriptor.first_entry }; entry != nullptr;
       entry = entry->next_entry)
    {
     if(register_entry(*entry))
     {
       register_contiguous_unseen_entries(entry->next_entry);
       return;
     }
    }
  }

  void object_tracker::remove_jit_debug_objects(uptr const resource_key)
  {
    auto jit_debug_objects_guard{ jit_debug_objects.wlock() };
    auto &objects_by_resource_key{ jit_debug_objects_guard->objects_by_resource_key };
    auto const it{ objects_by_resource_key.find(resource_key) };
    if(it == objects_by_resource_key.end())
    {
      return;
    }

    auto &registered_object_starts{ jit_debug_objects_guard->registered_object_starts };
    for(auto const &object : it->second)
    {
      if(object.object_start != nullptr)
      {
        cpptrace::unregister_jit_object(object.object_start);
        registered_object_starts.erase(object.object_start);
      }
    }
    objects_by_resource_key.erase(it);
  }

  void object_tracker::transfer_jit_debug_objects(uptr const dst_resource_key,
                                                  uptr const src_resource_key)
  {
    auto jit_debug_objects_guard{ jit_debug_objects.wlock() };
    auto &objects_by_resource_key{ jit_debug_objects_guard->objects_by_resource_key };
    auto const it{ objects_by_resource_key.find(src_resource_key) };
    if(it == objects_by_resource_key.end())
    {
      return;
    }

    auto &dst_objects{ objects_by_resource_key[dst_resource_key] };
    for(auto &object : it->second)
    {
      dst_objects.emplace_back(jtl::move(object));
    }
    objects_by_resource_key.erase(it);
  }

  void object_tracker::record_materialized_symbol(uptr const resource_key,
                                                  std::string const &symbol,
                                                  uptr const runtime_address,
                                                  usize const runtime_size)
  {
    std::string object_path;
    loaded_object_symbol object_symbol{};
    usize symbol_size{};
    {
      auto const loaded_objects_guard{ loaded_objects.rlock() };
      auto const loaded_object_it{ loaded_objects_guard->find(resource_key) };
      if(loaded_object_it == loaded_objects_guard->end())
      {
        return;
      }

      auto const *symbol_match{ find_loaded_object_symbol(loaded_object_it->second, symbol) };
      if(symbol_match == nullptr)
      {
        return;
      }
      object_path = loaded_object_it->second.path;
      object_symbol = *symbol_match;
      symbol_size = runtime_size == 0 ? object_symbol.object_size : runtime_size;
    }

    /* Combine the object-file symbol metadata gathered at load time with the runtime address
     * assigned by JITLink. After this point, raw PCs can be mapped back to backing `.o` files
     * without consulting ORC again. */
    materialized_symbols.wlock()->insert_or_assign(
      runtime_address,
      materialized_symbol{ resource_key,
                           runtime_address,
                           symbol_size,
                           object_path,
                           object_symbol.object_address,
                           object_symbol.object_size,
                           symbol });
  }

  void object_tracker::remove_materialized_symbols(uptr const resource_key)
  {
    auto materialized_symbols_guard{ materialized_symbols.wlock() };
    for(auto it{ materialized_symbols_guard->begin() }; it != materialized_symbols_guard->end();)
    {
      if(it->second.resource_key == resource_key)
      {
        it = materialized_symbols_guard->erase(it);
      }
      else
      {
        ++it;
      }
    }
  }

  void object_tracker::transfer_materialized_symbols(uptr const dst_resource_key,
                                                     uptr const src_resource_key)
  {
    auto materialized_symbols_guard{ materialized_symbols.wlock() };
    for(auto &[_, symbol] : *materialized_symbols_guard)
    {
      if(symbol.resource_key == src_resource_key)
      {
        symbol.resource_key = dst_resource_key;
      }
    }
  }

  jtl::option<materialized_object_frame>
  object_tracker::find_materialized_object_frame(uptr const raw_address) const
  {
    auto const materialized_symbols_guard{ materialized_symbols.rlock() };
    /* `materialized_symbols` is ordered by runtime symbol start address, so `upper_bound`
     * gives us the nearest candidate range at or below the raw PC. */
    auto const upper{ materialized_symbols_guard->upper_bound(raw_address) };
    if(upper == materialized_symbols_guard->begin())
    {
      return none;
    }

    auto const &symbol{ std::prev(upper)->second };
    if(raw_address < symbol.runtime_address
       || raw_address >= symbol.runtime_address + symbol.runtime_size)
    {
      return none;
    }

    return materialized_object_frame{
      symbol.runtime_address, symbol.runtime_size,
      symbol.object_path,     symbol.object_address + (raw_address - symbol.runtime_address),
      symbol.object_size,     symbol.symbol
    };
  }

  void register_loaded_object(uptr const resource_key, loaded_object const &object)
  {
    global_tracker().register_loaded_object(resource_key, object);
  }

  cpptrace::stacktrace_frame resolve_materialized_object_frame(cpptrace::stacktrace_frame frame)
  {
    if((!frame.filename.empty() && frame.line.has_value()) || runtime::__rt_ctx == nullptr)
    {
      return frame;
    }

    /* cpptrace has already done its normal best-effort resolution. If a frame is still missing
     * source information, ask jank whether the raw PC belongs to lazily materialized object-file
     * code and, if so, re-resolve it against the original `.o` file on disk. */
    auto const resolved{ global_tracker().find_materialized_object_frame(frame.raw_address) };
    if(resolved.is_none())
    {
      return frame;
    }

    /* Feed cpptrace an object-space frame for the original `.o` file so it can reuse its normal
     * on-disk DWARF resolver rather than teaching cpptrace about ORC-specific bookkeeping. */
    cpptrace::object_trace object_trace;
    object_trace.frames.push_back(
      { frame.raw_address, resolved.unwrap().object_address, resolved.unwrap().object_path });
    auto const resolved_trace{ object_trace.resolve() };
    if(resolved_trace.frames.empty())
    {
      return frame;
    }

    auto const &resolved_frame{ resolved_trace.frames.front() };
    frame.object_address = resolved_frame.object_address;
    if(frame.filename.empty())
    {
      frame.filename = resolved_frame.filename;
    }
    if(!frame.line.has_value())
    {
      frame.line = resolved_frame.line;
    }
    if(!frame.column.has_value())
    {
      frame.column = resolved_frame.column;
    }
    if(frame.symbol.empty())
    {
      frame.symbol
        = resolved_frame.symbol.empty() ? resolved.unwrap().symbol : resolved_frame.symbol;
    }

    return frame;
  }
}
