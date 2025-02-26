#include <jank/runtime/obj/repeat.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/math.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/seq.hpp>

namespace jank::runtime::obj
{
  repeat::repeat(object_ptr const value)
    : value{ value }
  {
  }

  repeat::repeat(size_t const count, object_ptr const value)
    : value{ value }
    , count{ count }
  {
    if(count == infinite)
    {
      throw std::runtime_error{ "repeat must be constructed with positive count: "
                                + std::to_string(count) };
    }
  }

  object_ptr repeat::create(object_ptr const value)
  {
    return make_box<repeat>(value);
  }

  object_ptr repeat::create(object_ptr const count, object_ptr const value)
  {
    auto const c(to_int(count));
    if(c <= 0)
    {
      return persistent_list::empty();
    }
    return make_box<repeat>(static_cast<size_t>(c), value);
  }

  repeat_ptr repeat::seq()
  {
    return this;
  }

  repeat_ptr repeat::fresh_seq() const
  {
    if(count == infinite)
    {
      return this;
    }
    return make_box<repeat>(count, value);
  }

  object_ptr repeat::first() const
  {
    return value;
  }

  object_ptr repeat::reduce(object_ptr const f, object_ptr const start) const
  {
    return visit_object(
      [](auto const typed_f, auto ret, auto const value, auto const bound) {
        using T = typename decltype(typed_f)::value_type;
        native_bool direct{};
        if constexpr(std::same_as<T, runtime::obj::jit_function>
                     || std::same_as<T, runtime::obj::jit_closure>)
        {
          auto const arity_flags(typed_f->get_arity_flags());
          auto const mask(behavior::callable::extract_variadic_arity_mask(arity_flags));
          switch(mask)
          {
            case behavior::callable::mask_variadic_arity(0):
            case behavior::callable::mask_variadic_arity(1):
            case behavior::callable::mask_variadic_arity(2):
              //TODO create new jit_{function,closure} that can be called
              //with 2 arity so no extra branch is needed for the common case.
              break;
            default:
              direct = true;
          }
        }
        if(bound == infinite)
        {
          while(true)
          {
            if constexpr(std::same_as<T, runtime::obj::jit_function>
                         || std::same_as<T, runtime::obj::jit_closure>)
            {
              if(direct)
              {
                ret = typed_f->call(ret, value);
              }
              else
              {
                ret = dynamic_call(typed_f, ret, value);
              }
            }
            else
            {
              ret = dynamic_call(typed_f, ret, value);
            }

            if(ret->type == object_type::reduced)
            {
              return expect_object<obj::reduced>(ret)->val;
            }
          }
        }
        else
        {
          for(size_t i{}; i < bound; ++i)
          {
            if constexpr(std::same_as<T, runtime::obj::jit_function>
                         || std::same_as<T, runtime::obj::jit_closure>)
            {
              if(direct)
              {
                ret = typed_f->call(ret, value);
              }
              else
              {
                ret = dynamic_call(typed_f, ret, value);
              }
            }
            else
            {
              ret = dynamic_call(typed_f, ret, value);
            }

            if(ret->type == object_type::reduced)
            {
              return expect_object<obj::reduced>(ret)->val;
            }
          }
          return ret;
        }
      },
      f,
      start,
      value,
      count);
  }

  repeat_ptr repeat::next() const
  {
    auto const c(count);
    switch(c)
    {
      case infinite:
        return this;
      case 1:
        return nullptr;
    }
    return make_box<repeat>(c - 1, value);
  }

  repeat_ptr repeat::next_in_place()
  {
    auto const c(count);
    switch(c)
    {
      case infinite:
        return this;
      case 1:
        return nullptr;
    }
    count = c - 1;
    return this;
  }

  cons_ptr repeat::conj(object_ptr const head) const
  {
    return make_box<cons>(head, this);
  }

  native_bool repeat::equal(object const &o) const
  {
    return visit_seqable(
      [this](auto const typed_o) {
        auto seq(typed_o->fresh_seq());
        /* TODO: This is common code; can it be shared? */
        for(auto it(fresh_seq()); it != nullptr;
            it = runtime::next_in_place(it), seq = runtime::next_in_place(seq))
        {
          if(seq == nullptr || !runtime::equal(it->first(), seq->first()))
          {
            return false;
          }
        }
        return seq == nullptr;
      },
      []() { return false; },
      &o);
  }

  void repeat::to_string(util::string_builder &buff)
  {
    runtime::to_string(seq(), buff);
  }

  native_persistent_string repeat::to_string()
  {
    return runtime::to_string(seq());
  }

  native_persistent_string repeat::to_code_string()
  {
    return runtime::to_code_string(seq());
  }

  native_hash repeat::to_hash() const
  {
    return hash::ordered(&base);
  }

  repeat_ptr repeat::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(fresh_seq());
    ret->meta = meta;
    return ret;
  }
}
