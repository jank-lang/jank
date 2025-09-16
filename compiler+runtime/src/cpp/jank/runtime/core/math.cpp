#include <random>

#include <jank/runtime/core/math.hpp>
#include <jank/runtime/behavior/number_like.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::runtime
{
  struct integer_ops;
  struct real_ops;

  struct number_ops
  {
    virtual ~number_ops() = default;

    virtual number_ops const &combine(number_ops const &) const = 0;
    virtual number_ops const &with(integer_ops const &) const = 0;
    virtual number_ops const &with(real_ops const &) const = 0;

    virtual object_ref add() const = 0;
    virtual f64 add_real() const = 0;
    virtual object_ref subtract() const = 0;
    virtual f64 sub_real() const = 0;
    virtual object_ref multiply() const = 0;
    virtual f64 mul_real() const = 0;
    virtual object_ref divide() const = 0;
    virtual f64 div_real() const = 0;
    virtual object_ref remainder() const = 0;
    virtual object_ref inc() const = 0;
    virtual object_ref dec() const = 0;
    virtual object_ref negate() const = 0;
    virtual object_ref abs() const = 0;
    virtual object_ref min() const = 0;
    virtual f64 min_real() const = 0;
    virtual object_ref max() const = 0;
    virtual f64 max_real() const = 0;
    virtual f64 pow() const = 0;
    virtual bool lt() const = 0;
    virtual bool lte() const = 0;
    virtual bool gte() const = 0;
    virtual bool equal() const = 0;
    virtual bool is_positive() const = 0;
    virtual bool is_negative() const = 0;
    virtual bool is_zero() const = 0;
  };

  number_ops &left_ops(object_ref n);
  number_ops &right_ops(object_ref n);

  struct integer_ops : number_ops
  {
    number_ops const &combine(number_ops const &l) const final
    {
      return l.with(*this);
    }

    number_ops const &with(integer_ops const &) const final;
    number_ops const &with(real_ops const &) const final;

    object_ref add() const final
    {
      return make_box<obj::integer>(left + right);
    }

    f64 add_real() const final
    {
      return left + right;
    }

    object_ref subtract() const final
    {
      return make_box<obj::integer>(left - right);
    }

    f64 sub_real() const final
    {
      return left - right;
    }

    object_ref multiply() const final
    {
      return make_box<obj::integer>(left * right);
    }

    f64 mul_real() const final
    {
      return left * right;
    }

    object_ref divide() const final
    {
      return make_box<obj::integer>(left / right);
    }

    f64 div_real() const final
    {
      return static_cast<f64>(left) / right;
    }

    object_ref remainder() const final
    {
      return make_box<obj::integer>(left % right);
    }

    object_ref inc() const final
    {
      return make_box<obj::integer>(left + 1);
    }

    object_ref dec() const final
    {
      return make_box<obj::integer>(left - 1);
    }

    object_ref negate() const final
    {
      return make_box<obj::integer>(-left);
    }

    object_ref abs() const final
    {
      return make_box<obj::integer>(std::labs(left));
    }

    object_ref min() const final
    {
      return make_box<obj::integer>(std::min(left, right));
    }

    f64 min_real() const final
    {
      return std::min(left, right);
    }

    object_ref max() const final
    {
      return make_box<obj::integer>(std::max(left, right));
    }

    f64 max_real() const final
    {
      return std::max(left, right);
    }

    f64 pow() const final
    {
      return std::pow(left, right);
    }

    bool lt() const final
    {
      return left < right;
    }

    bool lte() const final
    {
      return left <= right;
    }

    bool gte() const final
    {
      return left >= right;
    }

    bool equal() const final
    {
      return left == right;
    }

    bool is_positive() const final
    {
      return left > 0;
    }

    bool is_negative() const final
    {
      return left < 0;
    }

    bool is_zero() const final
    {
      return left == 0;
    }

    i64 left{}, right{};
  };

  struct real_ops : number_ops
  {
    number_ops const &combine(number_ops const &l) const final
    {
      return l.with(*this);
    }

    number_ops const &with(integer_ops const &) const final;
    number_ops const &with(real_ops const &) const final;

    object_ref add() const final
    {
      return make_box<obj::real>(left + right);
    }

    f64 add_real() const final
    {
      return left + right;
    }

    object_ref subtract() const final
    {
      return make_box<obj::real>(left - right);
    }

    f64 sub_real() const final
    {
      return left - right;
    }

    object_ref multiply() const final
    {
      return make_box<obj::real>(left * right);
    }

    f64 mul_real() const final
    {
      return left * right;
    }

    object_ref divide() const final
    {
      return make_box<obj::real>(left / right);
    }

    f64 div_real() const final
    {
      return left / right;
    }

    object_ref remainder() const final
    {
      return make_box<obj::real>(std::fmod(left, right));
    }

    object_ref inc() const final
    {
      return make_box<obj::real>(left + 1);
    }

    object_ref dec() const final
    {
      return make_box<obj::real>(right + 1);
    }

    object_ref negate() const final
    {
      return make_box<obj::real>(-left);
    }

    object_ref abs() const final
    {
      return make_box<obj::real>(std::fabs(left));
    }

    object_ref min() const final
    {
      return make_box<obj::real>(std::min(left, right));
    }

    f64 min_real() const final
    {
      return std::min(left, right);
    }

    object_ref max() const final
    {
      return make_box<obj::real>(std::max(left, right));
    }

    f64 max_real() const final
    {
      return std::max(left, right);
    }

    f64 pow() const final
    {
      return std::pow(left, right);
    }

    bool lt() const final
    {
      return left < right;
    }

    bool lte() const final
    {
      return left <= right;
    }

    bool gte() const final
    {
      return left >= right;
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"

    bool equal() const final
    {
      return left == right;
    }

#pragma clang diagnostic pop

    bool is_positive() const final
    {
      return left > 0;
    }

    bool is_negative() const final
    {
      return left < 0;
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"

    bool is_zero() const final
    {
      return left == 0;
    }

#pragma clang diagnostic pop

    f64 left{}, right{};
  };

  // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables): These are thread-local.
  static thread_local integer_ops i_ops;
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables): These are thread-local.
  static thread_local real_ops r_ops;

  number_ops const &integer_ops::with(integer_ops const &) const
  {
    return i_ops;
  }

  number_ops const &integer_ops::with(real_ops const &) const
  {
    r_ops.left = left;
    return r_ops;
  }

  number_ops const &real_ops::with(integer_ops const &r) const
  {
    r_ops.right = r.right;
    return r_ops;
  }

  number_ops const &real_ops::with(real_ops const &) const
  {
    return r_ops;
  }

  number_ops &left_ops(object_ref const n)
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
    switch(n->type)
    {
      case object_type::integer:
        {
          i_ops.left = expect_object<obj::integer>(n)->data;
          return i_ops;
        }
      case object_type::real:
        {
          r_ops.left = expect_object<obj::real>(n)->data;
          return r_ops;
        }
      default:
        /* TODO: Exception type. */
        {
          throw std::runtime_error{ util::format("(left_ops) not a number: {}",
                                                 runtime::to_code_string(n)) };
        }
    }
#pragma clang diagnostic pop
  }

  static integer_ops &left_ops(obj::integer_ref const n)
  {
    i_ops.left = n->data;
    return i_ops;
  }

  static real_ops &left_ops(obj::real_ref const n)
  {
    r_ops.left = n->data;
    return r_ops;
  }

  static integer_ops &left_ops(i64 const n)
  {
    i_ops.left = n;
    return i_ops;
  }

  static real_ops &left_ops(f64 const n)
  {
    r_ops.left = n;
    return r_ops;
  }

  number_ops &right_ops(object_ref const n)
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
    switch(n->type)
    {
      case object_type::integer:
        {
          i_ops.right = expect_object<obj::integer>(n)->data;
          return i_ops;
        }
      case object_type::real:
        {
          r_ops.right = expect_object<obj::real>(n)->data;
          return r_ops;
        }
      default:
        /* TODO: Exception type. */
        {
          throw std::runtime_error{ util::format("(right_ops) not a number: {}",
                                                 runtime::to_code_string(n)) };
        }
    }
#pragma clang diagnostic pop
  }

  static integer_ops &right_ops(obj::integer_ref const n)
  {
    i_ops.right = n->data;
    return i_ops;
  }

  static real_ops &right_ops(obj::real_ref const n)
  {
    r_ops.right = n->data;
    return r_ops;
  }

  static integer_ops &right_ops(i64 const n)
  {
    i_ops.right = n;
    return i_ops;
  }

  static real_ops &right_ops(f64 const n)
  {
    r_ops.right = n;
    return r_ops;
  }

  /* This version of `with` avoids two dynamic dispatches per operation, so it's
   * preferable over `number_ops.combine`. */
  [[maybe_unused]]
  static integer_ops const &with(integer_ops const &, integer_ops const &)
  {
    return i_ops;
  }

  [[maybe_unused]]
  static real_ops const &with(real_ops const &, real_ops const &)
  {
    return r_ops;
  }

  static real_ops const &with(integer_ops const &, real_ops const &)
  {
    r_ops.left = i_ops.left;
    return r_ops;
  }

  static real_ops const &with(real_ops const &, integer_ops const &)
  {
    r_ops.right = i_ops.right;
    return r_ops;
  }

  static number_ops const &with(number_ops const &l, number_ops const &r)
  {
    return r.combine(l);
  }

  template <typename T>
  static f64 to_real(T const &val)
  {
    if constexpr(std::is_same_v<T, i64>)
    {
      return static_cast<f64>(val);
    }
    else if constexpr(std::is_same_v<T, obj::big_integer>)
    {
      return val.template convert_to<f64>();
    }
    else if constexpr(std::is_same_v<T, native_big_integer>)
    {
      return val.template convert_to<f64>();
    }
    else if constexpr(std::is_same_v<T, f64>)
    {
      return val;
    }
    else if constexpr(std::is_same_v<T, obj::ratio_data>)
    {
      return val.to_real();
    }
    else
    {
      static_assert(!sizeof(T *), "Unsupported type for to_real conversion.");
      return 0.0;
    }
  }

  object_ref add(object_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).add();
  }

  object_ref add(obj::integer_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).add();
  }

  object_ref add(object_ref const l, obj::integer_ref const r)
  {
    return with(left_ops(l), right_ops(r)).add();
  }

  i64 add(obj::integer_ref const l, obj::integer_ref const r)
  {
    return l->data + r->data;
  }

  f64 add(obj::real_ref const l, obj::real_ref const r)
  {
    return l->data + r->data;
  }

  f64 add(obj::real_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).add_real();
  }

  f64 add(object_ref const l, obj::real_ref const r)
  {
    return with(left_ops(l), right_ops(r)).add_real();
  }

  f64 add(obj::real_ref const l, obj::integer_ref const r)
  {
    return l->data + r->data;
  }

  f64 add(obj::integer_ref const l, obj::real_ref const r)
  {
    return l->data + r->data;
  }

  f64 add(object_ref const l, f64 const r)
  {
    return with(left_ops(l), right_ops(r)).add_real();
  }

  f64 add(f64 const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).add_real();
  }

  f64 add(f64 const l, f64 const r)
  {
    return l + r;
  }

  f64 add(i64 const l, f64 const r)
  {
    return l + r;
  }

  f64 add(f64 const l, i64 const r)
  {
    return l + r;
  }

  object_ref sub(object_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).subtract();
  }

  object_ref sub(obj::integer_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).subtract();
  }

  object_ref sub(object_ref const l, obj::integer_ref const r)
  {
    return with(left_ops(l), right_ops(r)).subtract();
  }

  i64 sub(obj::integer_ref const l, obj::integer_ref const r)
  {
    return l->data - r->data;
  }

  f64 sub(obj::real_ref const l, obj::real_ref const r)
  {
    return l->data - r->data;
  }

  f64 sub(obj::real_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).sub_real();
  }

  f64 sub(object_ref const l, obj::real_ref const r)
  {
    return with(left_ops(l), right_ops(r)).sub_real();
  }

  f64 sub(obj::real_ref const l, obj::integer_ref const r)
  {
    return l->data - r->data;
  }

  f64 sub(obj::integer_ref const l, obj::real_ref const r)
  {
    return l->data - r->data;
  }

  f64 sub(object_ref const l, f64 const r)
  {
    return with(left_ops(l), right_ops(r)).sub_real();
  }

  f64 sub(f64 const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).sub_real();
  }

  f64 sub(f64 const l, f64 const r)
  {
    return l - r;
  }

  f64 sub(i64 const l, f64 const r)
  {
    return l - r;
  }

  f64 sub(f64 const l, i64 const r)
  {
    return l - r;
  }

  object_ref sub(object_ref const l, i64 const r)
  {
    return with(left_ops(l), right_ops(r)).subtract();
  }

  object_ref sub(i64 const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).subtract();
  }

  i64 sub(i64 const l, i64 const r)
  {
    return l - r;
  }

  object_ref div(object_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).divide();
  }

  object_ref div(obj::integer_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).divide();
  }

  object_ref div(object_ref const l, obj::integer_ref const r)
  {
    return with(left_ops(l), right_ops(r)).divide();
  }

  i64 div(obj::integer_ref const l, obj::integer_ref const r)
  {
    return l->data / r->data;
  }

  f64 div(obj::real_ref const l, obj::real_ref const r)
  {
    return l->data / r->data;
  }

  f64 div(obj::real_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).div_real();
  }

  f64 div(object_ref const l, obj::real_ref const r)
  {
    return with(left_ops(l), right_ops(r)).div_real();
  }

  f64 div(obj::real_ref const l, obj::integer_ref const r)
  {
    return l->data / r->data;
  }

  f64 div(obj::integer_ref const l, obj::real_ref const r)
  {
    return l->data / r->data;
  }

  f64 div(object_ref const l, f64 const r)
  {
    return with(left_ops(l), right_ops(r)).div_real();
  }

  f64 div(f64 const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).div_real();
  }

  f64 div(f64 const l, f64 const r)
  {
    return l / r;
  }

  f64 div(i64 const l, f64 const r)
  {
    return l / r;
  }

  f64 div(f64 const l, i64 const r)
  {
    return l / r;
  }

  object_ref div(object_ref const l, i64 const r)
  {
    return with(left_ops(l), right_ops(r)).divide();
  }

  object_ref div(i64 const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).divide();
  }

  i64 div(i64 const l, i64 const r)
  {
    return l / r;
  }

  object_ref mul(object_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).multiply();
  }

  object_ref mul(obj::integer_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).multiply();
  }

  object_ref mul(object_ref const l, obj::integer_ref const r)
  {
    return with(left_ops(l), right_ops(r)).multiply();
  }

  i64 mul(obj::integer_ref const l, obj::integer_ref const r)
  {
    return l->data * r->data;
  }

  f64 mul(obj::real_ref const l, obj::real_ref const r)
  {
    return l->data * r->data;
  }

  f64 mul(obj::real_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).mul_real();
  }

  f64 mul(object_ref const l, obj::real_ref const r)
  {
    return with(left_ops(l), right_ops(r)).mul_real();
  }

  f64 mul(obj::real_ref const l, obj::integer_ref const r)
  {
    return l->data * r->data;
  }

  f64 mul(obj::integer_ref const l, obj::real_ref const r)
  {
    return l->data * r->data;
  }

  f64 mul(object_ref const l, f64 const r)
  {
    return with(left_ops(l), right_ops(r)).mul_real();
  }

  f64 mul(f64 const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).mul_real();
  }

  f64 mul(f64 const l, f64 const r)
  {
    return l * r;
  }

  f64 mul(i64 const l, f64 const r)
  {
    return l * r;
  }

  f64 mul(f64 const l, i64 const r)
  {
    return l * r;
  }

  object_ref mul(object_ref const l, i64 const r)
  {
    return with(left_ops(l), right_ops(r)).multiply();
  }

  object_ref mul(i64 const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).multiply();
  }

  i64 mul(i64 const l, i64 const r)
  {
    return l * r;
  }

  object_ref rem(object_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const r_obj) -> object_ref {
        return visit_number_like(
          []<typename T>(auto const typed_r, T const &typed_l_data) -> object_ref {
            using LeftType = std::decay_t<T>;
            using RightType = std::decay_t<decltype(typed_r->data)>;

            constexpr bool left_is_int_like{ std::is_same_v<LeftType, i64>
                                             || std::is_same_v<LeftType, native_big_integer> };
            constexpr bool right_is_int_like{ std::is_same_v<RightType, i64>
                                              || std::is_same_v<RightType, native_big_integer> };

            if constexpr(left_is_int_like && right_is_int_like)
            {
              if constexpr(std::is_same_v<LeftType, i64>
                           && std::is_same_v<RightType, native_big_integer>)
              {
                return make_box(native_big_integer(typed_l_data) % typed_r->data).erase();
              }
              else if constexpr(std::is_same_v<LeftType, native_big_integer>
                                && std::is_same_v<RightType, i64>)
              {
                return make_box(typed_l_data % native_big_integer(typed_r->data)).erase();
              }
              else
              {
                return make_box(typed_l_data % typed_r->data).erase();
              }
            }
            else
            {
              auto const l_real{ to_real(typed_l_data) };
              auto const r_real{ to_real(typed_r->data) };
              return make_box(std::fmod(l_real, r_real)).erase();
            }
          },
          r_obj,
          typed_l->data);
      },
      l,
      r);
  }

  object_ref quot(object_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const r) -> object_ref {
        return visit_number_like(
          [](auto const typed_r, auto const &typed_l) -> object_ref {
            auto const typed_l_data{ to_real(typed_l) };
            auto const typed_r_data{ to_real(typed_r->data) };
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
            if(typed_r_data == 0ll)
            {
#pragma clang diagnostic pop
              throw make_box("Illegal divide by zero in 'quot'").erase();
            }
            else
            {
              return make_box(static_cast<i64>(typed_l_data / typed_r_data)).erase();
            }
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  object_ref inc(object_ref const l)
  {
    return visit_number_like(
      [](auto const typed_l) -> object_ref {
        auto const ret{ make_box(typed_l->data + 1ll) };
        object_ref r{ ret };
        return r;
      },
      l);
  }

  object_ref dec(object_ref const l)
  {
    return visit_number_like(
      [](auto const typed_l) -> object_ref { return make_box(typed_l->data - 1ll).erase(); },
      l);
  }

  bool is_zero(object_ref const l)
  {
    return visit_number_like(
      [](auto const typed_l) -> bool {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
        {
          return typed_l->data == 0ll;
        }
#pragma clang diagnostic pop
      },
      l);
  }

  bool is_pos(object_ref const l)
  {
    return visit_number_like(
      [](auto const typed_l) -> bool {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
        {
          return typed_l->data > 0ll;
        }
#pragma clang diagnostic pop
      },
      l);
  }

  bool is_neg(object_ref const l)
  {
    return visit_number_like(
      [](auto const typed_l) -> bool {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
        {
          return typed_l->data < 0ll;
        }
#pragma clang diagnostic pop
      },
      l);
  }

  bool is_even(object_ref const l)
  {
    return visit_type<obj::integer>(
      [=](auto const typed_l) -> bool { return typed_l->data % 2 == 0; },
      l);
  }

  bool is_odd(object_ref const l)
  {
    return visit_type<obj::integer>(
      [=](auto const typed_l) -> bool { return typed_l->data % 2 == 1; },
      l);
  }

  bool is_equiv(object_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, object_ref const r) -> bool {
        return visit_number_like(
          [](auto const typed_r, auto const &typed_l) -> bool {
            auto const data_l{ to_real(typed_l) };
            auto const data_r{ to_real(typed_r->data) };

            using C = std::common_type_t<decltype(data_l), decltype(data_r)>;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
            return static_cast<C>(data_l) == static_cast<C>(data_r);
#pragma clang diagnostic pop
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  i64 bit_not(object_ref const l)
  {
    return visit_type<obj::integer>([](auto const typed_l) -> i64 { return ~typed_l->data; }, l);
  }

  i64 bit_and(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> i64 {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> i64 { return typed_l & typed_r->data; },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  i64 bit_or(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> i64 {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> i64 { return typed_l | typed_r->data; },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  i64 bit_xor(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> i64 {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> i64 { return typed_l ^ typed_r->data; },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  i64 bit_and_not(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> i64 {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> i64 { return typed_l & (~typed_r->data); },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  i64 bit_clear(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> i64 {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> i64 {
            return typed_l & ~(static_cast<i64>(1) << typed_r->data);
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  i64 bit_set(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> i64 {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> i64 {
            return typed_l | (static_cast<i64>(1) << typed_r->data);
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  i64 bit_flip(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> i64 {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> i64 {
            return typed_l ^ (static_cast<i64>(1) << typed_r->data);
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  bool bit_test(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> bool {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> bool {
            return (typed_l >> typed_r->data) & static_cast<i64>(1);
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  i64 bit_shift_left(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> i64 {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> i64 { return typed_l << typed_r->data; },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  i64 bit_shift_right(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> i64 {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> i64 { return typed_l >> typed_r->data; },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  i64 bit_unsigned_shift_right(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> i64 {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> i64 {
            using uni = std::make_unsigned_t<i64>;
            return static_cast<uni>(typed_l) >> static_cast<uni>(typed_r->data);
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  f64 rand()
  {
    static std::mt19937 gen;
    static std::uniform_real_distribution<f64> dis(0.0, 1.0);
    return dis(gen);
  }

  bool lt(object_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).lt();
  }

  bool lt(obj::integer_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).lt();
  }

  bool lt(object_ref const l, obj::integer_ref const r)
  {
    return with(left_ops(l), right_ops(r)).lt();
  }

  bool lt(obj::integer_ref const l, obj::integer_ref const r)
  {
    return l->data < r->data;
  }

  bool lt(obj::real_ref const l, obj::real_ref const r)
  {
    return l->data < r->data;
  }

  bool lt(obj::real_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).lt();
  }

  bool lt(object_ref const l, obj::real_ref const r)
  {
    return with(left_ops(l), right_ops(r)).lt();
  }

  bool lt(obj::real_ref const l, obj::integer_ref const r)
  {
    return with(left_ops(l), right_ops(r)).lt();
  }

  bool lt(obj::integer_ref const l, obj::real_ref const r)
  {
    return with(left_ops(l), right_ops(r)).lt();
  }

  bool lt(object_ref const l, f64 const r)
  {
    return with(left_ops(l), right_ops(r)).lt();
  }

  bool lt(f64 const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).lt();
  }

  bool lt(f64 const l, f64 const r)
  {
    return l < r;
  }

  bool lt(i64 const l, f64 const r)
  {
    return l < r;
  }

  bool lt(f64 const l, i64 const r)
  {
    return l < r;
  }

  bool lt(object_ref const l, i64 const r)
  {
    return with(left_ops(l), right_ops(r)).lt();
  }

  bool lt(i64 const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).lt();
  }

  bool lt(i64 const l, i64 const r)
  {
    return l < r;
  }

  bool lte(object_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).lte();
  }

  bool lte(obj::integer_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).lte();
  }

  bool lte(object_ref const l, obj::integer_ref const r)
  {
    return with(left_ops(l), right_ops(r)).lte();
  }

  bool lte(obj::integer_ref const l, obj::integer_ref const r)
  {
    return l->data <= r->data;
  }

  bool lte(obj::real_ref const l, obj::real_ref const r)
  {
    return l->data <= r->data;
  }

  bool lte(obj::real_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).lte();
  }

  bool lte(object_ref const l, obj::real_ref const r)
  {
    return with(left_ops(l), right_ops(r)).lte();
  }

  bool lte(obj::real_ref const l, obj::integer_ref const r)
  {
    return with(left_ops(l), right_ops(r)).lte();
  }

  bool lte(obj::integer_ref const l, obj::real_ref const r)
  {
    return with(left_ops(l), right_ops(r)).lte();
  }

  bool lte(object_ref const l, f64 const r)
  {
    return with(left_ops(l), right_ops(r)).lte();
  }

  bool lte(f64 const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).lte();
  }

  bool lte(f64 const l, f64 const r)
  {
    return l < r;
  }

  bool lte(i64 const l, f64 const r)
  {
    return l < r;
  }

  bool lte(f64 const l, i64 const r)
  {
    return l < r;
  }

  bool lte(object_ref const l, i64 const r)
  {
    return with(left_ops(l), right_ops(r)).lte();
  }

  bool lte(i64 const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).lte();
  }

  bool lte(i64 const l, i64 const r)
  {
    return l < r;
  }

  object_ref min(object_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).min();
  }

  object_ref min(obj::integer_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).min();
  }

  object_ref min(object_ref const l, obj::integer_ref const r)
  {
    return with(left_ops(l), right_ops(r)).min();
  }

  i64 min(obj::integer_ref const l, obj::integer_ref const r)
  {
    return std::min(l->data, r->data);
  }

  f64 min(obj::real_ref const l, obj::real_ref const r)
  {
    return std::min(l->data, r->data);
  }

  f64 min(obj::real_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).min_real();
  }

  f64 min(object_ref const l, obj::real_ref const r)
  {
    return with(left_ops(l), right_ops(r)).min_real();
  }

  f64 min(obj::real_ref const l, obj::integer_ref const r)
  {
    return std::min(l->data, static_cast<f64>(r->data));
  }

  f64 min(obj::integer_ref const l, obj::real_ref const r)
  {
    return std::min(static_cast<f64>(l->data), r->data);
  }

  f64 min(object_ref const l, f64 const r)
  {
    return with(left_ops(l), right_ops(r)).min_real();
  }

  f64 min(f64 const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).min_real();
  }

  f64 min(f64 const l, f64 const r)
  {
    return std::min(l, r);
  }

  f64 min(i64 const l, f64 const r)
  {
    return std::min(static_cast<f64>(l), r);
  }

  f64 min(f64 const l, i64 const r)
  {
    return std::min(l, static_cast<f64>(r));
  }

  object_ref min(object_ref const l, i64 const r)
  {
    return with(left_ops(l), right_ops(r)).min();
  }

  object_ref min(i64 const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).min();
  }

  i64 min(i64 const l, i64 const r)
  {
    return std::min(l, r);
  }

  object_ref max(object_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).max();
  }

  object_ref max(obj::integer_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).max();
  }

  object_ref max(object_ref const l, obj::integer_ref const r)
  {
    return with(left_ops(l), right_ops(r)).max();
  }

  i64 max(obj::integer_ref const l, obj::integer_ref const r)
  {
    return std::max(l->data, r->data);
  }

  f64 max(obj::real_ref const l, obj::real_ref const r)
  {
    return std::max(l->data, r->data);
  }

  f64 max(obj::real_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).max_real();
  }

  f64 max(object_ref const l, obj::real_ref const r)
  {
    return with(left_ops(l), right_ops(r)).max_real();
  }

  f64 max(obj::real_ref const l, obj::integer_ref const r)
  {
    return std::max(l->data, static_cast<f64>(r->data));
  }

  f64 max(obj::integer_ref const l, obj::real_ref const r)
  {
    return std::max(static_cast<f64>(l->data), r->data);
  }

  f64 max(object_ref const l, f64 const r)
  {
    return with(left_ops(l), right_ops(r)).max_real();
  }

  f64 max(f64 const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).max_real();
  }

  f64 max(f64 const l, f64 const r)
  {
    return std::max(l, r);
  }

  f64 max(i64 const l, f64 const r)
  {
    return std::max(static_cast<f64>(l), r);
  }

  f64 max(f64 const l, i64 const r)
  {
    return std::max(l, static_cast<f64>(r));
  }

  object_ref max(object_ref const l, i64 const r)
  {
    return with(left_ops(l), right_ops(r)).max();
  }

  object_ref max(i64 const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).max();
  }

  i64 max(i64 const l, i64 const r)
  {
    return std::max(l, r);
  }

  object_ref abs(object_ref const l)
  {
    return left_ops(l).abs();
  }

  i64 abs(obj::integer_ref const l)
  {
    return std::abs(l->data);
  }

  f64 abs(obj::real_ref const l)
  {
    return std::fabs(l->data);
  }

  i64 abs(i64 const l)
  {
    return std::abs(l);
  }

  f64 abs(f64 const l)
  {
    return std::fabs(l);
  }

  f64 tan(object_ref const l)
  {
    return visit_number_like([](auto const typed_l) -> f64 { return tanf(typed_l->to_real()); }, l);
  }

  f64 sqrt(object_ref const l)
  {
    return visit_number_like(
      [](auto const typed_l) -> f64 { return std::sqrt(typed_l->to_real()); },
      l);
  }

  f64 sqrt(obj::integer_ref const l)
  {
    return std::sqrt(l->data);
  }

  f64 sqrt(obj::real_ref const l)
  {
    return std::sqrt(l->data);
  }

  f64 sqrt(i64 const l)
  {
    return std::sqrt(l);
  }

  f64 sqrt(f64 const l)
  {
    return std::sqrt(l);
  }

  f64 pow(object_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).pow();
  }

  f64 pow(obj::integer_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).pow();
  }

  f64 pow(object_ref const l, obj::integer_ref const r)
  {
    return with(left_ops(l), right_ops(r)).pow();
  }

  f64 pow(obj::integer_ref const l, obj::integer_ref const r)
  {
    return std::pow(l->data, r->data);
  }

  f64 pow(obj::real_ref const l, obj::real_ref const r)
  {
    return std::pow(l->data, r->data);
  }

  f64 pow(obj::real_ref const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).pow();
  }

  f64 pow(object_ref const l, obj::real_ref const r)
  {
    return with(left_ops(l), right_ops(r)).pow();
  }

  f64 pow(obj::real_ref const l, obj::integer_ref const r)
  {
    return std::pow(l->data, r->data);
  }

  f64 pow(obj::integer_ref const l, obj::real_ref const r)
  {
    return std::pow(l->data, r->data);
  }

  f64 pow(object_ref const l, f64 const r)
  {
    return with(left_ops(l), right_ops(r)).pow();
  }

  f64 pow(f64 const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).pow();
  }

  f64 pow(f64 const l, f64 const r)
  {
    return std::pow(l, r);
  }

  f64 pow(i64 const l, f64 const r)
  {
    return std::pow(l, r);
  }

  f64 pow(f64 const l, i64 const r)
  {
    return std::pow(l, r);
  }

  f64 pow(object_ref const l, i64 const r)
  {
    return with(left_ops(l), right_ops(r)).pow();
  }

  f64 pow(i64 const l, object_ref const r)
  {
    return with(left_ops(l), right_ops(r)).pow();
  }

  f64 pow(i64 const l, i64 const r)
  {
    return std::pow(l, r);
  }

  native_big_integer numerator(object_ref const o)
  {
    return try_object<obj::ratio>(o)->data.numerator;
  }

  native_big_integer denominator(object_ref const o)
  {
    return try_object<obj::ratio>(o)->data.denominator;
  }

  i64 to_int(object_ref const l)
  {
    return visit_number_like([](auto const typed_l) -> i64 { return typed_l->to_integer(); }, l);
  }

  i64 to_int(obj::integer_ref const l)
  {
    return l->data;
  }

  i64 to_int(obj::real_ref const l)
  {
    return static_cast<i64>(l->data);
  }

  i64 to_int(i64 const l)
  {
    return l;
  }

  i64 to_int(f64 const l)
  {
    return static_cast<i64>(l);
  }

  bool is_number(object_ref const o)
  {
    return visit_number_like([=](auto const) -> bool { return true; },
                             [=]() -> bool { return false; },
                             o);
  }

  bool is_integer(object_ref const o)
  {
    return o->type == object_type::integer;
  }

  bool is_real(object_ref const o)
  {
    return o->type == object_type::real;
  }

  bool is_ratio(object_ref const o)
  {
    return o->type == object_type::ratio;
  }

  bool is_boolean(object_ref const o)
  {
    return o->type == object_type::boolean;
  }

  bool is_nan(object_ref const o)
  {
    return visit_number_like(
      [=](auto const typed_o) -> bool {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(std::same_as<T, obj::real>)
        {
          return std::isnan(typed_o->data);
        }
        else
        {
          return false;
        }
      },
      o);
  }

  bool is_infinite(object_ref const o)
  {
    return visit_number_like(
      [=](auto const typed_o) -> bool {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(std::same_as<T, obj::real>)
        {
          return std::isinf(typed_o->data);
        }
        else
        {
          return false;
        }
      },
      o);
  }

  /* TODO: Rename these to match the type name. */
  i64 parse_long(object_ref const o)
  {
    auto const typed_o{ dyn_cast<obj::persistent_string>(o) };
    if(typed_o.is_some())
    {
      return std::stoll(typed_o->data);
    }
    else
    {
      throw make_box(util::format("Expected string, got {}", object_type_str(o->type))).erase();
    }
  }

  f64 parse_double(object_ref const o)
  {
    auto const typed_o{ dyn_cast<obj::persistent_string>(o) };
    if(typed_o.is_some())
    {
      return std::stod(typed_o->data);
    }
    else
    {
      throw make_box(util::format("Expected string, got {}", object_type_str(o->type))).erase();
    }
  }
}
