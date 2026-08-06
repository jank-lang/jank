#include <unordered_map>
#include <type_traits>

#include <llvm/ExecutionEngine/JITLink/JITLink.h>

#include <CppInterOp/Compatibility.h>

#include <jank/jit/object.hpp>
#include <jank/runtime/context.hpp>

namespace jank::jit
{
  struct object_tracker
  {
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
    void record_materialized_symbol(uptr resource_key,
                                    std::string const &symbol,
                                    uptr runtime_address,
                                    usize runtime_size);
    void remove_materialized_symbols(uptr resource_key);
    void transfer_materialized_symbols(uptr dst_resource_key, uptr src_resource_key);
    jtl::option<materialized_object_frame> find_materialized_object_frame(uptr raw_address) const;

    /* Resource key -> original object file metadata captured at `load_object` time. */
    folly::Synchronized<std::unordered_map<uptr, loaded_object>> loaded_objects;
    /* Runtime symbol start address -> materialized symbol metadata captured from JITLink. */
    folly::Synchronized<std::map<uptr, materialized_symbol>> materialized_symbols;
  };

  object_tracker &global_tracker()
  {
    static object_tracker tracker;
    return tracker;
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

    llvm::Error notifyFailed(llvm::orc::MaterializationResponsibility &mr) override
    {
      if(auto const resource_key{ get_resource_key(mr) }; resource_key.is_some())
      {
        tracker.remove_materialized_symbols(resource_key.unwrap());
      }
      return llvm::Error::success();
    }

    llvm::Error
    notifyRemovingResources(llvm::orc::JITDylib &, llvm::orc::ResourceKey const key) override
    {
      tracker.remove_loaded_object(key);
      tracker.remove_materialized_symbols(key);
      return llvm::Error::success();
    }

    void notifyTransferringResources(llvm::orc::JITDylib &,
                                     llvm::orc::ResourceKey const dst_key,
                                     llvm::orc::ResourceKey const src_key) override
    {
      tracker.transfer_loaded_object(dst_key, src_key);
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

      auto const symbol_it{ loaded_object_it->second.symbols.find(symbol) };
      if(symbol_it == loaded_object_it->second.symbols.end())
      {
        return;
      }
      object_path = loaded_object_it->second.path;
      object_symbol = symbol_it->second;
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

  jtl::option<materialized_object_frame> find_materialized_object_frame(uptr const raw_address)
  {
    return global_tracker().find_materialized_object_frame(raw_address);
  }
}
