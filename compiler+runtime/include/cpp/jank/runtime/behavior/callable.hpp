#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using persistent_list_ref = oref<struct persistent_list>;
  }

  constexpr usize const max_params{ 10 };

  struct variadic_tag
  {
  };

  /* When analysis can match a call to a fn definition, we can know the
   * supported arities at compile-time, but that's not always the case in
   * dynamic code. We may not every know if the source is callable at all, so
   * codegen will use this suite of fns instead. */
  object_ref dynamic_call(object_ref source);
  object_ref dynamic_call(object_ref source, object_ref);
  object_ref dynamic_call(object_ref source, object_ref, object_ref);
  object_ref dynamic_call(object_ref source, object_ref, object_ref, object_ref);
  object_ref dynamic_call(object_ref source, object_ref, object_ref, object_ref, object_ref);
  object_ref
  dynamic_call(object_ref source, object_ref, object_ref, object_ref, object_ref, object_ref);
  object_ref dynamic_call(object_ref source,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref);
  object_ref dynamic_call(object_ref source,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref);
  object_ref dynamic_call(object_ref source,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref);
  object_ref dynamic_call(object_ref source,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref);
  object_ref dynamic_call(object_ref source,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref);
  object_ref dynamic_call(object_ref source,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          obj::persistent_list_ref);

  object_ref apply_to(object_ref source, object_ref args);

  namespace behavior
  {
    /* TODO: I think we can use CRTP here. */
    struct callable
    {
      using arity_flag_t = u8;

      virtual ~callable() = default;

      virtual object_ref call();
      virtual object_ref call(object_ref);
      virtual object_ref call(object_ref, object_ref);
      virtual object_ref call(object_ref, object_ref, object_ref);
      virtual object_ref call(object_ref, object_ref, object_ref, object_ref);
      virtual object_ref call(object_ref, object_ref, object_ref, object_ref, object_ref);
      virtual object_ref
        call(object_ref, object_ref, object_ref, object_ref, object_ref, object_ref);
      virtual object_ref
        call(object_ref, object_ref, object_ref, object_ref, object_ref, object_ref, object_ref);
      virtual object_ref call(object_ref,
                              object_ref,
                              object_ref,
                              object_ref,
                              object_ref,
                              object_ref,
                              object_ref,
                              object_ref);
      virtual object_ref call(object_ref,
                              object_ref,
                              object_ref,
                              object_ref,
                              object_ref,
                              object_ref,
                              object_ref,
                              object_ref,
                              object_ref);
      virtual object_ref call(object_ref,
                              object_ref,
                              object_ref,
                              object_ref,
                              object_ref,
                              object_ref,
                              object_ref,
                              object_ref,
                              object_ref,
                              object_ref);

      /* Callables need a way to get back to the root object so we can do helpful
       * error reporting on failed calls. */
      virtual object_ref this_object_ref() = 0;

      /* When dynamically calling a function, we need to know three things:
       *
       * 1. Is the function variadic?
       * 2. Is there an ambiguous fixed overload?
       * 3. How many fixed arguments are required before the packed args?
       *
       * We cannot perform the correct call without all of this information. Since function calls
       * are on the hottest path there is, we pack all of this into a single byte. Questions
       * 1 and 2 each get a bit and question 3 gets 6 bits to store the fixed arg count.
       *
       * From there, when we use it, we strip out the bit for question 2 and we switch/case on
       * the rest. This allows us to do a O(1) jump on the combination of whether it's variadic
       * and the required fixed args. Finally, we only need the question 2 bit to disambiguate
       * one branch of each switch.
       *
       * The ambiguity comes in this case:
       *
       * ```
       * (defn ambiguous
       *   ([a] 1)
       *   ([a & args] args))
       * (ambiguous :a)
       * ```
       *
       * When we call `ambiguous` with a single arg, we want it to match the fixed unary arity.
       * However, given just questions 1 and 3, we will see that we've met the required args
       * and that the function is variadic and we'll instead dispatch to the variadic arity, with
       * an empty sequence for `args`.
       */
      virtual arity_flag_t get_arity_flags() const;

      static constexpr arity_flag_t mask_variadic_arity(u8 const pos)
      {
        return (0b10000000 | pos);
      }

      static constexpr arity_flag_t extract_variadic_arity_mask(arity_flag_t const arity_flags)
      {
        return (arity_flags & 0b10001111);
      }

      static constexpr native_bool is_variadic_ambiguous(arity_flag_t const arity_flags)
      {
        return (arity_flags & 0b01000000);
      }

      static constexpr arity_flag_t build_arity_flags(u8 const highest_fixed_arity,
                                                      native_bool const is_variadic,
                                                      native_bool const is_variadic_ambiguous)
      {
        return (is_variadic << 7) | (is_variadic_ambiguous << 6) | highest_fixed_arity;
      }
    };

    using callable_ref = jtl::ref<callable>;

    /* TODO: Is this needed? A non-callable function-like would need to define all call overloads? :( */
    template <typename T>
    concept function_like = requires(T * const t) {
      { t->call(object_ref{}) } -> std::convertible_to<object_ref>;
      { t->call(object_ref{}, object_ref{}) } -> std::convertible_to<object_ref>;
      { t->call(object_ref{}, object_ref{}, object_ref{}) } -> std::convertible_to<object_ref>;
      {
        t->call(object_ref{}, object_ref{}, object_ref{}, object_ref{})
      } -> std::convertible_to<object_ref>;
      {
        t->call(object_ref{}, object_ref{}, object_ref{}, object_ref{}, object_ref{})
      } -> std::convertible_to<object_ref>;
      {
        t->call(object_ref{}, object_ref{}, object_ref{}, object_ref{}, object_ref{}, object_ref{})
      } -> std::convertible_to<object_ref>;
      {
        t->call(object_ref{},
                object_ref{},
                object_ref{},
                object_ref{},
                object_ref{},
                object_ref{},
                object_ref{})
      } -> std::convertible_to<object_ref>;
      {
        t->call(object_ref{},
                object_ref{},
                object_ref{},
                object_ref{},
                object_ref{},
                object_ref{},
                object_ref{},
                object_ref{})
      } -> std::convertible_to<object_ref>;
      {
        t->call(object_ref{},
                object_ref{},
                object_ref{},
                object_ref{},
                object_ref{},
                object_ref{},
                object_ref{},
                object_ref{},
                object_ref{})
      } -> std::convertible_to<object_ref>;

      { t->get_arity_flags() } -> std::convertible_to<size_t>;
    };
  }
}
