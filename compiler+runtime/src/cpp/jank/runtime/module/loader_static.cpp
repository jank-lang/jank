#include <jank/runtime/module/loader.hpp>
#include <jank/error/runtime.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::module
{
  loader::loader() = default;

  jtl::result<void, error_ref> loader::load(jtl::immutable_string const &module, origin const)
  {
    {
      /* If a load fn has been provided for this module already, just skip right to
       * calling it. */
      auto const locked_state{ state.lock() };
      auto const managed_load_fn{ locked_state->managed_load_fns.find(module) };
      if(managed_load_fn != locked_state->managed_load_fns.end())
      {
        (*managed_load_fn->second)();
        set_is_loaded(module);
        return ok();
      }
    }

    return error::runtime_module_not_found(util::format("Unable to find module '{}'.", module));
  }
}
