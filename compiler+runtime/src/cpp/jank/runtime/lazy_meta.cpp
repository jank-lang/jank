#include <jank/codegen/api.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/lazy_meta.hpp>

namespace jank::runtime
{
  lazy_meta::lazy_meta(jtl::immutable_string const &source)
    : state{
      { source, {} }
  },
    ns{ __rt_ctx->current_ns_var->n }
  {
  }

  lazy_meta::lazy_meta(jtl::immutable_string const &source, ns_ref const ns)
    : state{
      { source, {} }
  },
    ns{ ns }
  {
  }

  lazy_meta::lazy_meta(object_ref const meta)
    : state{
      { {}, meta }
  }
  {
  }

  object_ref lazy_meta::get() const
  {
    auto const locked_state{ state.wlock() };
    if(locked_state->source.empty())
    {
      return locked_state->meta;
    }

    {
      context::binding_scope const _{ runtime::obj::persistent_hash_map::create_unique(
        std::make_pair(__rt_ctx->current_ns_var, ns)) };
      locked_state->meta = _jank_eval_str(locked_state->source.c_str());
    }

    locked_state->source = "";
    return locked_state->meta;
  }

  void lazy_meta::set(object_ref const o)
  {
    auto const locked_state{ state.wlock() };
    locked_state->source = "";
    locked_state->meta = o;
  }
}
