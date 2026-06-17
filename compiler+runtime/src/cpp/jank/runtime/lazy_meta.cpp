#include <jank/runtime/lazy_meta.hpp>
#include <jank/codegen/api.hpp>

namespace jank::runtime
{
  lazy_meta::lazy_meta(jtl::immutable_string const &source)
    : state{
      { source, {} }
  }
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
    /* TODO: Synchronize. */
    if(locked_state->source.empty())
    {
      return locked_state->meta;
    };

    locked_state->meta = _jank_eval_str(locked_state->source.c_str());
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
