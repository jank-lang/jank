#include <functional>

#include <jank/analyze/local_frame.hpp>

namespace jank::analyze
{
  local_frame::local_frame
  (
    frame_type const &type,
    runtime::context &ctx,
    option<std::reference_wrapper<local_frame>> const &p
  ) : type{ type }, parent{ p }, runtime_ctx{ ctx }
  { }

  local_frame& local_frame::operator=(local_frame const &rhs)
  {
    if(this == &rhs)
    { return *this; }

    type = rhs.type;
    parent = rhs.parent;
    locals = rhs.locals;

    return *this;
  }
  local_frame& local_frame::operator=(local_frame &&rhs)
  {
    if(this == &rhs)
    { return *this; }

    type = rhs.type;
    parent = std::move(rhs.parent);
    locals = std::move(rhs.locals);

    return *this;
  }

  option<local_frame::find_result> local_frame::find(runtime::obj::symbol_ptr const &sym)
  {
    decltype(local_frame::find_result::crossed_fns) crossed_fns;

    for(local_frame *it{ this }; it != nullptr; )
    {
      auto const result(it->locals.find(sym));
      if(result != it->locals.end())
      { return local_frame::find_result{ result->second, std::move(crossed_fns) }; }
      else if(it->parent.is_some())
      {
        if(it->type == frame_type::fn)
        { crossed_fns.emplace_back(*it); }
        it = &it->parent.unwrap().get();
      }
      else
      { return none; }
    }

    __builtin_unreachable();
  }

  void local_frame::register_captures(find_result const &result)
  {
    for(auto const &crossed_fn : result.crossed_fns)
    { crossed_fn.get().captures.emplace(result.binding.name, result.binding); }
  }
}
