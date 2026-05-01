#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using nil_ref = oref<struct nil>;
    using integer_ref = oref<struct integer>;
    using real_ref = oref<struct real>;
    using ratio_ref = oref<struct ratio>;
    using big_integer_ref = oref<struct big_integer>;
    using big_decimal_ref = oref<struct big_decimal>;
  }

  namespace detail
  {
    template <typename T>
    concept primitive_number = (std::is_integral_v<T> || std::is_floating_point_v<T>);

    template <typename T>
    concept typed_object = (behavior::object_like<typename T::value_type>);

    template <typename T>
    concept valid_boxed_math = (jtl::is_same<T, object_ref>
                                || jtl::is_any_same<T,
                                                    obj::integer_ref,
                                                    obj::real_ref,
                                                    obj::big_integer_ref,
                                                    obj::big_decimal_ref,
                                                    obj::ratio_ref>);
  }

  /* Only for fixed integer sizes (i.e. integer and small_integer). */
  i64 to_i64(object_ref const o);

  template <typename L, typename R>
  requires(!detail::primitive_number<L> && !detail::primitive_number<R>)
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto add(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<L> || !detail::valid_boxed_math<R>)
    {
      throw std::runtime_error{
        util::format("Can't add a {} to {}.", object_type_str(l->type), object_type_str(r->type))
      };
      return object_ref{};
    }
    else if constexpr(jtl::is_same<L, object_ref> && jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, auto const r) -> object_ref {
          return visit_number_like(
            [](auto const typed_r, auto const typed_l) -> object_ref {
              return make_box(typed_l->data + typed_r->data).erase();
            },
            r,
            typed_l);
        },
        l,
        r);
    }
    else if constexpr(jtl::is_same<L, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, auto const r) { return make_box(typed_l->data + r->data).erase(); },
        l,
        r);
    }
    else if constexpr(jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_r, auto const l) { return make_box(l->data + typed_r->data).erase(); },
        r,
        l);
    }
    else
    {
      return l->data + r->data;
    }
  }

  template <typename L, typename R>
  requires detail::primitive_number<L>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto add(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<R>)
    {
      throw std::runtime_error{
        util::format("Can't add a {} to {}.", jtl::type_name<L>(), object_type_str(r->type))
      };
      return object_ref{};
    }
    else if constexpr(jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_r, L const l) { return make_box(l + typed_r->data).erase(); },
        r,
        l);
    }
    else if constexpr(detail::typed_object<R>)
    {
      return l + r->data;
    }
    else
    {
      return l + r;
    }
  }

  template <typename L, typename R>
  requires detail::primitive_number<R>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto add(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<L>)
    {
      throw std::runtime_error{
        util::format("Can't add a {} to {}.", object_type_str(l->type), jtl::type_name<R>())
      };
      return object_ref{};
    }
    else if constexpr(jtl::is_same<L, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, R const r) { return make_box(typed_l->data + r).erase(); },
        l,
        r);
    }
    else if constexpr(detail::typed_object<L>)
    {
      return l->data + r;
    }
    else
    {
      return l + r;
    }
  }

  template <typename L, typename R>
  requires(detail::primitive_number<L> && detail::primitive_number<R>)
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto add(L const l, R const r)
  {
    return l + r;
  }

  object_ref promoting_add(object_ref const l, object_ref const r);

  template <typename L, typename R>
  requires(!detail::primitive_number<L> && !detail::primitive_number<R>)
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto sub(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<L> || !detail::valid_boxed_math<R>)
    {
      throw std::runtime_error{ util::format("Can't subtract a {} from a {}.",
                                             object_type_str(l->type),
                                             object_type_str(r->type)) };
      return object_ref{};
    }
    else if constexpr(jtl::is_same<L, object_ref> && jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, auto const r) -> object_ref {
          return visit_number_like(
            [](auto const typed_r, auto const typed_l) -> object_ref {
              return make_box(typed_l->data - typed_r->data).erase();
            },
            r,
            typed_l);
        },
        l,
        r);
    }
    else if constexpr(jtl::is_same<L, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, auto const r) { return make_box(typed_l->data - r->data).erase(); },
        l,
        r);
    }
    else if constexpr(jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_r, auto const l) { return make_box(l->data - typed_r->data).erase(); },
        r,
        l);
    }
    else
    {
      return l->data - r->data;
    }
  }

  template <typename L, typename R>
  requires detail::primitive_number<L>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto sub(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<R>)
    {
      throw std::runtime_error{ util::format("Can't subtract a {} from a {}.",
                                             jtl::type_name<L>(),
                                             object_type_str(r->type)) };
      return object_ref{};
    }
    else if constexpr(jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_r, L const l) { return make_box(l - typed_r->data).erase(); },
        r,
        l);
    }
    else if constexpr(detail::typed_object<R>)
    {
      return l - r->data;
    }
    else
    {
      return l - r;
    }
  }

  template <typename L, typename R>
  requires detail::primitive_number<R>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto sub(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<L>)
    {
      throw std::runtime_error{ util::format("Can't subtract a {} from a {}.",
                                             object_type_str(l->type),
                                             jtl::type_name<R>()) };
      return object_ref{};
    }
    else if constexpr(jtl::is_same<L, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, R const r) { return make_box(typed_l->data - r).erase(); },
        l,
        r);
    }
    else if constexpr(detail::typed_object<L>)
    {
      return l->data - r;
    }
    else
    {
      return l - r;
    }
  }

  template <typename L, typename R>
  requires(detail::primitive_number<L> && detail::primitive_number<R>)
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto sub(L const l, R const r)
  {
    return l - r;
  }

  object_ref promoting_sub(object_ref const l, object_ref const r);

  template <typename L, typename R>
  requires(!detail::primitive_number<L> && !detail::primitive_number<R>)
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto div(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<L> || !detail::valid_boxed_math<R>)
    {
      throw std::runtime_error{ util::format("Can't divide a {} by a {}.",
                                             object_type_str(l->type),
                                             object_type_str(r->type)) };
      return object_ref{};
    }
    else if constexpr(jtl::is_same<L, object_ref> && jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, auto const r) -> object_ref {
          return visit_number_like(
            [](auto const typed_r, auto const typed_l) -> object_ref {
              return make_box(typed_l->data / typed_r->data).erase();
            },
            r,
            typed_l);
        },
        l,
        r);
    }
    else if constexpr(jtl::is_same<L, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, auto const r) { return make_box(typed_l->data / r->data).erase(); },
        l,
        r);
    }
    else if constexpr(jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_r, auto const l) { return make_box(l->data / typed_r->data).erase(); },
        r,
        l);
    }
    else
    {
      return l->data / r->data;
    }
  }

  template <typename L, typename R>
  requires detail::primitive_number<L>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto div(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<R>)
    {
      throw std::runtime_error{
        util::format("Can't divide a {} by a {}.", jtl::type_name<L>(), object_type_str(r->type))
      };
      return object_ref{};
    }
    else if constexpr(jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_r, L const l) { return make_box(l / typed_r->data).erase(); },
        r,
        l);
    }
    else if constexpr(detail::typed_object<R>)
    {
      return l / r->data;
    }
    else
    {
      return l / r;
    }
  }

  template <typename L, typename R>
  requires detail::primitive_number<R>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto div(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<L>)
    {
      throw std::runtime_error{
        util::format("Can't divide a {} by a {}.", object_type_str(l->type), jtl::type_name<R>())
      };
      return object_ref{};
    }
    else if constexpr(jtl::is_same<L, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, R const r) { return make_box(typed_l->data / r).erase(); },
        l,
        r);
    }
    else if constexpr(detail::typed_object<L>)
    {
      return l->data / r;
    }
    else
    {
      return l / r;
    }
  }

  template <typename L, typename R>
  requires(detail::primitive_number<L> && detail::primitive_number<R>)
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto div(L const l, R const r)
  {
    return l / r;
  }

  template <typename L, typename R>
  requires(!detail::primitive_number<L> && !detail::primitive_number<R>)
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto mul(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<L> || !detail::valid_boxed_math<R>)
    {
      throw std::runtime_error{ util::format("Can't multiply a {} with a {}.",
                                             object_type_str(l->type),
                                             object_type_str(r->type)) };
      return object_ref{};
    }
    else if constexpr(jtl::is_same<L, object_ref> && jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, auto const r) -> object_ref {
          return visit_number_like(
            [](auto const typed_r, auto const typed_l) -> object_ref {
              return make_box(typed_l->data * typed_r->data).erase();
            },
            r,
            typed_l);
        },
        l,
        r);
    }
    else if constexpr(jtl::is_same<L, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, auto const r) { return make_box(typed_l->data * r->data).erase(); },
        l,
        r);
    }
    else if constexpr(jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_r, auto const l) { return make_box(l->data * typed_r->data).erase(); },
        r,
        l);
    }
    else
    {
      return l->data * r->data;
    }
  }

  template <typename L, typename R>
  requires detail::primitive_number<L>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto mul(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<R>)
    {
      throw std::runtime_error{ util::format("Can't multiply a {} with a {}.",
                                             jtl::type_name<L>(),
                                             object_type_str(r->type)) };
      return object_ref{};
    }
    else if constexpr(jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_r, L const l) { return make_box(l * typed_r->data).erase(); },
        r,
        l);
    }
    else if constexpr(detail::typed_object<R>)
    {
      return l * r->data;
    }
    else
    {
      return l * r;
    }
  }

  template <typename L, typename R>
  requires detail::primitive_number<R>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto mul(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<L>)
    {
      throw std::runtime_error{ util::format("Can't multiply a {} with a {}.",
                                             object_type_str(l->type),
                                             jtl::type_name<R>()) };
      return object_ref{};
    }
    if constexpr(jtl::is_same<L, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, R const r) { return make_box(typed_l->data * r).erase(); },
        l,
        r);
    }
    else if constexpr(detail::typed_object<L>)
    {
      return l->data * r;
    }
    else
    {
      return l * r;
    }
  }

  template <typename L, typename R>
  requires(detail::primitive_number<L> && detail::primitive_number<R>)
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto mul(L const l, R const r)
  {
    return l * r;
  }

  object_ref promoting_mul(object_ref const l, object_ref const r);

  template <typename L, typename R>
  requires(!detail::primitive_number<L> && !detail::primitive_number<R>)
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  bool lt(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<L> || !detail::valid_boxed_math<R>)
    {
      throw std::runtime_error{ util::format("Can't compare a {} to a {}.",
                                             object_type_str(l->type),
                                             object_type_str(r->type)) };
      return false;
    }
    else if constexpr(jtl::is_same<L, object_ref> && jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, auto const r) {
          return visit_number_like(
            [](auto const typed_r, auto const typed_l) { return typed_l->data < typed_r->data; },
            r,
            typed_l);
        },
        l,
        r);
    }
    else if constexpr(jtl::is_same<L, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, auto const r) { return typed_l->data < r->data; },
        l,
        r);
    }
    else if constexpr(jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_r, auto const l) { return l->data < typed_r->data; },
        r,
        l);
    }
    else
    {
      return l->data < r->data;
    }
  }

  template <typename L, typename R>
  requires detail::primitive_number<L>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  bool lt(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<R>)
    {
      throw std::runtime_error{
        util::format("Can't compare a {} to a {}.", jtl::type_name<L>(), object_type_str(r->type))
      };
      return false;
    }
    else if constexpr(jtl::is_same<R, object_ref>)
    {
      return visit_number_like([](auto const typed_r, L const l) { return l < typed_r->data; },
                               r,
                               l);
    }
    else if constexpr(detail::typed_object<R>)
    {
      return l < r->data;
    }
    else
    {
      return l < r;
    }
  }

  template <typename L, typename R>
  requires detail::primitive_number<R>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  bool lt(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<L>)
    {
      throw std::runtime_error{
        util::format("Can't compare a {} to a {}.", object_type_str(l->type), jtl::type_name<R>())
      };
      return false;
    }
    else if constexpr(jtl::is_same<L, object_ref>)
    {
      return visit_number_like([](auto const typed_l, R const r) { return typed_l->data < r; },
                               l,
                               r);
    }
    else if constexpr(detail::typed_object<L>)
    {
      return l->data < r;
    }
    else
    {
      return l < r;
    }
  }

  template <typename L, typename R>
  requires(detail::primitive_number<L> && detail::primitive_number<R>)
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  bool lt(L const l, R const r)
  {
    return l < r;
  }

  template <typename L, typename R>
  requires(!detail::primitive_number<L> && !detail::primitive_number<R>)
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  bool lte(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<L> || !detail::valid_boxed_math<R>)
    {
      throw std::runtime_error{ util::format("Can't compare a {} to a {}.",
                                             object_type_str(l->type),
                                             object_type_str(r->type)) };
      return false;
    }
    else if constexpr(jtl::is_same<L, object_ref> && jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, auto const r) {
          return visit_number_like(
            [](auto const typed_r, auto const typed_l) { return typed_l->data <= typed_r->data; },
            r,
            typed_l);
        },
        l,
        r);
    }
    else if constexpr(jtl::is_same<L, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, auto const r) { return typed_l->data <= r->data; },
        l,
        r);
    }
    else if constexpr(jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_r, auto const l) { return l->data <= typed_r->data; },
        r,
        l);
    }
    else
    {
      return l->data <= r->data;
    }
  }

  template <typename L, typename R>
  requires detail::primitive_number<L>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  bool lte(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<R>)
    {
      throw std::runtime_error{
        util::format("Can't compare a {} to a {}.", jtl::type_name<L>(), object_type_str(r->type))
      };
      return false;
    }
    else if constexpr(jtl::is_same<R, object_ref>)
    {
      return visit_number_like([](auto const typed_r, L const l) { return l <= typed_r->data; },
                               r,
                               l);
    }
    else if constexpr(detail::typed_object<R>)
    {
      return l <= r->data;
    }
    else
    {
      return l <= r;
    }
  }

  template <typename L, typename R>
  requires detail::primitive_number<R>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  bool lte(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<L>)
    {
      throw std::runtime_error{
        util::format("Can't compare a {} to a {}.", object_type_str(l->type), jtl::type_name<R>())
      };
      return false;
    }
    if constexpr(jtl::is_same<L, object_ref>)
    {
      return visit_number_like([](auto const typed_l, R const r) { return typed_l->data <= r; },
                               l,
                               r);
    }
    else if constexpr(detail::typed_object<L>)
    {
      return l->data <= r;
    }
    else
    {
      return l <= r;
    }
  }

  template <typename L, typename R>
  requires(detail::primitive_number<L> && detail::primitive_number<R>)
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  bool lte(L const l, R const r)
  {
    return l <= r;
  }

  template <typename L, typename R>
  requires(!detail::primitive_number<L> && !detail::primitive_number<R>)
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto min(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<L> || !detail::valid_boxed_math<R>)
    {
      throw std::runtime_error{ util::format("Can't compare a {} to a {}.",
                                             object_type_str(l->type),
                                             object_type_str(r->type)) };
      return object_ref{};
    }
    else if constexpr(jtl::is_same<L, object_ref> && jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, auto const r) -> object_ref {
          return visit_number_like(
            [](auto const typed_r, auto const typed_l) -> object_ref {
              return typed_l->data < typed_r->data ? typed_l.erase() : typed_r.erase();
            },
            r,
            typed_l);
        },
        l,
        r);
    }
    else if constexpr(jtl::is_same<L, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, auto const r) -> object_ref {
          return typed_l->data < r->data ? typed_l.erase() : r.erase();
        },
        l,
        r);
    }
    else if constexpr(jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_r, auto const l) {
          return l->data < typed_r->data ? l.erase() : typed_r.erase();
        },
        r,
        l);
    }
    else
    {
      return std::min(l->data, r->data);
    }
  }

  template <typename L, typename R>
  requires detail::primitive_number<L>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto min(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<R>)
    {
      throw std::runtime_error{
        util::format("Can't compare a {} to a {}.", jtl::type_name<L>(), object_type_str(r->type))
      };
      return object_ref{};
    }
    else if constexpr(jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_r, L const l) -> object_ref {
          return l < typed_r->data ? make_box(l).erase() : typed_r.erase();
        },
        r,
        l);
    }
    else if constexpr(detail::typed_object<R>)
    {
      return std::min(l, r->data);
    }
    else
    {
      return std::min(l, r);
    }
  }

  template <typename L, typename R>
  requires detail::primitive_number<R>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto min(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<L>)
    {
      throw std::runtime_error{
        util::format("Can't compare a {} to a {}.", object_type_str(l->type), jtl::type_name<R>())
      };
      return object_ref{};
    }
    if constexpr(jtl::is_same<L, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, R const r) -> object_ref {
          return typed_l < r ? typed_l.erase() : make_box(r).erase();
        },
        l,
        r);
    }
    else if constexpr(detail::typed_object<L>)
    {
      return std::min(l->data, r);
    }
    else
    {
      return std::min(l, r);
    }
  }

  template <typename L, typename R>
  requires(detail::primitive_number<L> && detail::primitive_number<R>)
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto min(L const l, R const r)
  {
    return std::min(l, r);
  }

  template <typename L, typename R>
  requires(!detail::primitive_number<L> && !detail::primitive_number<R>)
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto max(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<L> || !detail::valid_boxed_math<R>)
    {
      throw std::runtime_error{ util::format("Can't compare a {} to a {}.",
                                             object_type_str(l->type),
                                             object_type_str(r->type)) };
      return object_ref{};
    }
    else if constexpr(jtl::is_same<L, object_ref> && jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, auto const r) -> object_ref {
          return visit_number_like(
            [](auto const typed_r, auto const typed_l) -> object_ref {
              return typed_l->data < typed_r->data ? typed_r.erase() : typed_l.erase();
            },
            r,
            typed_l);
        },
        l,
        r);
    }
    else if constexpr(jtl::is_same<L, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, auto const r) -> object_ref {
          return typed_l->data < r->data ? r.erase() : typed_l.erase();
        },
        l,
        r);
    }
    else if constexpr(jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_r, auto const l) {
          return l->data < typed_r->data ? typed_r.erase() : l.erase();
        },
        r,
        l);
    }
    else
    {
      return std::max(l->data, r->data);
    }
  }

  template <typename L, typename R>
  requires detail::primitive_number<L>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto max(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<R>)
    {
      throw std::runtime_error{
        util::format("Can't compare a {} to a {}.", jtl::type_name<L>(), object_type_str(r->type))
      };
      return object_ref{};
    }
    else if constexpr(jtl::is_same<R, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_r, L const l) -> object_ref {
          return l < typed_r->data ? typed_r.erase() : make_box(l).erase();
        },
        r,
        l);
    }
    else if constexpr(detail::typed_object<R>)
    {
      return std::max(l, r->data);
    }
    else
    {
      return std::max(l, r);
    }
  }

  template <typename L, typename R>
  requires detail::primitive_number<R>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto max(L const l, R const r)
  {
    if constexpr(!detail::valid_boxed_math<L>)
    {
      throw std::runtime_error{
        util::format("Can't compare a {} to a {}.", object_type_str(l->type), jtl::type_name<R>())
      };
      return object_ref{};
    }
    if constexpr(jtl::is_same<L, object_ref>)
    {
      return visit_number_like(
        [](auto const typed_l, R const r) -> object_ref {
          return typed_l < r ? make_box(r).erase() : typed_l.erase();
        },
        l,
        r);
    }
    else if constexpr(detail::typed_object<L>)
    {
      return std::max(l->data, r);
    }
    else
    {
      return std::max(l, r);
    }
  }

  template <typename L, typename R>
  requires(detail::primitive_number<L> && detail::primitive_number<R>)
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  auto max(L const l, R const r)
  {
    return std::max(l, r);
  }

  object_ref abs(object_ref const l);
  object_ref abs(obj::nil_ref const l);
  i64 abs(obj::integer_ref const l);
  f64 abs(obj::real_ref const l);
  i64 abs(i64 l);
  f64 abs(f64 l);

  f64 tan(object_ref const l);

  f64 sqrt(object_ref const l);
  f64 sqrt(obj::integer_ref const l);
  f64 sqrt(obj::real_ref const l);
  f64 sqrt(i64 l);
  f64 sqrt(f64 l);

  f64 pow(object_ref const l, object_ref const r);
  f64 pow(obj::integer_ref const l, object_ref const r);
  f64 pow(object_ref const l, obj::integer_ref const r);
  f64 pow(obj::integer_ref const l, obj::integer_ref const r);
  f64 pow(obj::real_ref const l, obj::real_ref const r);
  f64 pow(obj::real_ref const l, object_ref const r);
  f64 pow(object_ref const l, obj::real_ref const r);
  f64 pow(obj::real_ref const l, obj::integer_ref const r);
  f64 pow(obj::integer_ref const l, obj::real_ref const r);
  f64 pow(object_ref const l, obj::ratio_ref const r);
  f64 pow(obj::ratio_ref const l, object_ref const r);
  f64 pow(obj::ratio_ref const l, obj::ratio_ref const r);

  object_ref pow(object_ref const l, f64 r);
  object_ref pow(f64 l, object_ref const r);
  f64 pow(f64 l, f64 r);

  f64 pow(i64 l, f64 r);
  f64 pow(f64 l, i64 r);

  f64 pow(object_ref const l, i64 r);
  f64 pow(i64 l, object_ref const r);
  f64 pow(i64 l, i64 r);

  template <typename L, typename R>
  auto pow(L const l, R const r)
  {
    using NormalizedL = std::conditional_t<std::is_integral_v<L>,
                                           i64,
                                           std::conditional_t<std::is_floating_point_v<L>, f64, L>>;
    using NormalizedR = std::conditional_t<std::is_integral_v<R>,
                                           i64,
                                           std::conditional_t<std::is_floating_point_v<R>, f64, R>>;
    return pow(static_cast<NormalizedL>(l), static_cast<NormalizedR>(r));
  }

  object_ref rem(object_ref const l, object_ref const r);
  object_ref quot(object_ref const l, object_ref const r);
  object_ref inc(object_ref const l);
  object_ref promoting_inc(object_ref const l);
  object_ref dec(object_ref const l);
  object_ref promoting_dec(object_ref const l);

  bool is_zero(object_ref const l);
  bool is_pos(object_ref const l);
  bool is_neg(object_ref const l);
  bool is_even(object_ref const l);
  bool is_odd(object_ref const l);

  bool is_equiv(object_ref const l, object_ref const r);

  i64 bit_not(object_ref const l);
  i64 bit_and(object_ref const l, object_ref const r);
  i64 bit_or(object_ref const l, object_ref const r);
  i64 bit_xor(object_ref const l, object_ref const r);
  i64 bit_and_not(object_ref const l, object_ref const r);
  i64 bit_clear(object_ref const l, object_ref const r);
  i64 bit_set(object_ref const l, object_ref const r);
  i64 bit_flip(object_ref const l, object_ref const r);
  bool bit_test(object_ref const l, object_ref const r);
  i64 bit_shift_left(object_ref const l, object_ref const r);
  i64 bit_shift_right(object_ref const l, object_ref const r);
  i64 bit_unsigned_shift_right(object_ref const l, object_ref const r);

  f64 rand();

  native_big_integer numerator(object_ref const o);
  native_big_integer denominator(object_ref const o);

  i64 to_int(object_ref const l);
  i64 to_int(obj::nil_ref const l);
  i64 to_int(obj::integer_ref const l);
  i64 to_int(obj::real_ref const l);
  i64 to_int(i64 l);
  i64 to_int(f64 l);

  f64 to_real(object_ref const o);

  bool is_number(object_ref const o);
  object_ref number(object_ref const o);

  bool is_integer(object_ref const o);
  bool is_real(object_ref const o);
  bool is_ratio(object_ref const o);
  bool is_boolean(object_ref const o);
  bool is_nan(object_ref const o);
  bool is_infinite(object_ref const o);

  i64 parse_long(object_ref const o);
  f64 parse_double(object_ref const o);

  bool is_big_integer(object_ref const o);
  obj::big_integer_ref to_big_integer(object_ref const o);

  bool is_big_decimal(object_ref const o);
  obj::big_decimal_ref to_big_decimal(object_ref const o);
}
