#pragma once

#include <iostream>
#include <jank/runtime/object.hpp>
#include <libunwind.h>

namespace jank::runtime
{

  inline void print_stack_trace()
  {
    unw_cursor_t cursor;
    unw_context_t context;

    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    std::cerr << "Stack trace:\n";
    while(unw_step(&cursor) > 0)
    {
      unw_word_t offset = 0, pc = 0;
      char sym[256];

      unw_get_reg(&cursor, UNW_REG_IP, &pc);
      if(unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0)
      {
        std::cerr << "  " << sym << " + 0x" << std::hex << offset << "\n";
      }
      else
      {
        std::cerr << "  [unknown]\n";
      }
    }
  }

  /* Most of the system is polymorphic using type-erased objects. Given any object, an erase call
   * will get you what you need. If you don't need to type-erase, though, don't! */
  template <typename T>
  requires behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr object_ptr erase(native_box<T> const o)
  {
    return &o->base;
  }

  template <typename T>
  requires behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr object_ptr erase(native_box<T const> const o)
  {
    return const_cast<object_ptr>(&o->base);
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr object_ptr erase(object_ptr const o)
  {
    return o;
  }

  template <typename T>
  requires behavior::object_like<std::decay_t<std::remove_pointer_t<T>>>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr object_ptr erase(T const o)
  {
    return const_cast<object *>(&o->base);
  }

  template <typename T>
  requires behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr native_box<T> isa(object const * const o)
  {
    assert(o);
    return o->type == T::obj_type;
  }

  template <typename T>
  requires behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr native_box<T> dyn_cast(object const * const o)
  {
    assert(o);
    if(o->type != T::obj_type)
    {
      return nullptr;
    }
    return reinterpret_cast<T *>(reinterpret_cast<char *>(const_cast<object *>(o))
                                 - offsetof(T, base));
  }

  template <typename T>
  requires behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  native_box<T> try_object(object const * const o)
  {
    assert(o);
    if(o->type != T::obj_type)
    {
      print_stack_trace();
      util::string_builder sb;
      sb("invalid object type (expected ");
      sb(object_type_str(T::obj_type));
      sb(" found ");
      sb(object_type_str(o->type));
      sb(")");
      throw std::runtime_error{ sb.str() };
    }
    return reinterpret_cast<T *>(reinterpret_cast<char *>(const_cast<object *>(o))
                                 - offsetof(T, base));
  }

  /* This is dangerous. You probably don't want it. Just use `try_object` or `visit_object`.
   * However, if you're absolutely certain you know the type of an erased object, I guess
   * you can use this. */
  template <typename T>
  requires behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr native_box<T> expect_object(object_ptr const o)
  {
    assert(o);
    assert(o->type == T::obj_type);
    return reinterpret_cast<T *>(reinterpret_cast<char *>(o.data) - offsetof(T, base));
  }

  template <typename T>
  requires behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr native_box<T> expect_object(object const * const o)
  {
    assert(o);
    assert(o->type == T::obj_type);
    return reinterpret_cast<T *>(reinterpret_cast<char *>(const_cast<object *>(o))
                                 - offsetof(T, base));
  }
}
