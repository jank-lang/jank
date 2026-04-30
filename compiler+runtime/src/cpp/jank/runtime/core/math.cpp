#include <random>

#include <jank/runtime/core/math.hpp>
#include <jank/runtime/behavior/number_like.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::runtime
{
  template <typename T>
  static f64 to_real(T const &val)
  {
    if constexpr(std::is_same_v<T, i64>)
    {
      return static_cast<f64>(val);
    }
    else if constexpr(std::is_same_v<T, native_big_integer>
                      || std::is_same_v<T, native_big_decimal>)
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

  i64 to_i64(object_ref const o)
  {
    if(detail::is_small_int(o.data))
    {
      return detail::as_int(o.data);
    }

    if(auto const i{ dyn_cast<obj::integer>(o) }; i.is_some())
    {
      return i->data;
    }

    throw std::runtime_error{ util::format("An integer was required here, but a {} was provided.",
                                           object_type_str(o.get_type())) };
  }

  object_ref promoting_add(object_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const r) -> object_ref {
        using LT = typename decltype(typed_l)::value_type;

        return visit_number_like(
          [](auto const typed_r, auto const &l_val) -> object_ref {
            using RT = typename decltype(typed_r)::value_type;

            if constexpr(jtl::is_any_same<LT, obj::integer, obj::small_integer>
                         && jtl::is_any_same<RT, obj::integer, obj::small_integer>)
            {
              i64 res{};

              if(static_cast<bool>(__builtin_add_overflow(l_val, typed_r->data, &res)))
              {
                native_big_integer const l{ l_val };
                return make_box<obj::big_integer>(l + typed_r->data);
              }

              return make_box(res);
            }
            else
            {
              return make_box(l_val + typed_r->data);
            }
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  object_ref promoting_sub(object_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const r) -> object_ref {
        using LT = typename decltype(typed_l)::value_type;

        return visit_number_like(
          [](auto const typed_r, auto const &l_val) -> object_ref {
            using RT = typename decltype(typed_r)::value_type;

            if constexpr(jtl::is_any_same<LT, obj::integer, obj::small_integer>
                         && jtl::is_any_same<RT, obj::integer, obj::small_integer>)
            {
              i64 res{};

              if(__builtin_sub_overflow(l_val, typed_r->data, &res))
              {
                native_big_integer const l{ l_val };
                return make_box<obj::big_integer>(l_val - typed_r->data);
              }

              return make_box(res);
            }
            else
            {
              return make_box(l_val - typed_r->data);
            }
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  object_ref div(object_ref const l, object_ref const r)
  {
    if(is_zero(r))
    {
      throw make_box("Illegal divide by zero in '/'").erase();
    }

    return visit_number_like(
      [](auto const typed_l, auto const r) -> object_ref {
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
              return obj::ratio::create(native_big_integer(typed_l_data),
                                        native_big_integer(typed_r->data));
            }

            return make_box(typed_l_data / typed_r->data).erase();
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  object_ref div(obj::nil_ref const l, obj::nil_ref const r)
  {
    throw std::runtime_error{
      util::format("Can't divide {} by {}.", to_code_string(l), to_code_string(r))
    };
  }

  object_ref div(obj::nil_ref const l, object_ref const r)
  {
    throw std::runtime_error{
      util::format("Can't divide {} by {}.", to_code_string(l), to_code_string(r))
    };
  }

  object_ref div(object_ref const l, obj::nil_ref const r)
  {
    throw std::runtime_error{
      util::format("Can't divide {} by {}.", to_code_string(l), to_code_string(r))
    };
  }

  object_ref div(obj::integer_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return make_box(typed_l / typed_r->data);
      },
      r,
      l->data);
  }

  object_ref div(object_ref const l, obj::integer_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return make_box(typed_l->data / typed_r);
      },
      l,
      r->data);
  }

  i64 div(obj::integer_ref const l, obj::integer_ref const r)
  {
    return l->data / r->data;
  }

  f64 div(obj::real_ref const l, obj::real_ref const r)
  {
    return l->data / r->data;
  }

  object_ref div(obj::real_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return make_box(typed_l / typed_r->data);
      },
      r,
      l->data);
  }

  object_ref div(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return make_box(typed_l->data / typed_r);
      },
      l,
      r->data);
  }

  f64 div(obj::real_ref const l, obj::integer_ref const r)
  {
    return l->data / static_cast<f64>(r->data);
  }

  f64 div(obj::integer_ref const l, obj::real_ref const r)
  {
    return static_cast<f64>(l->data) / r->data;
  }

  object_ref div(object_ref const l, f64 const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return make_box(typed_l->data / typed_r);
      },
      l,
      r);
  }

  object_ref div(f64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return make_box(typed_l / typed_r->data);
      },
      r,
      l);
  }

  f64 div(obj::integer_ref const l, f64 const r)
  {
    return static_cast<f64>(l->data) / r;
  }

  f64 div(f64 const l, obj::integer_ref const r)
  {
    return l / static_cast<f64>(r->data);
  }

  f64 div(f64 const l, f64 const r)
  {
    return l / r;
  }

  f64 div(i64 const l, f64 const r)
  {
    return static_cast<f64>(l) / r;
  }

  f64 div(f64 const l, i64 const r)
  {
    return l / static_cast<f64>(r);
  }

  object_ref div(object_ref const l, i64 const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return make_box(typed_l->data / typed_r);
      },
      l,
      r);
  }

  object_ref div(i64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return make_box(typed_l / typed_r->data);
      },
      r,
      l);
  }

  i64 div(i64 const l, i64 const r)
  {
    return l / r;
  }

  object_ref promoting_mul(object_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const r) -> object_ref {
        using LT = typename decltype(typed_l)::value_type;

        return visit_number_like(
          [](auto const typed_r, auto const &l_val) -> object_ref {
            using RT = typename decltype(typed_r)::value_type;

            if constexpr(jtl::is_any_same<LT, obj::integer, obj::small_integer>
                         && jtl::is_any_same<RT, obj::integer, obj::small_integer>)
            {
              i64 res{};

              if(__builtin_mul_overflow(l_val, typed_r->data, &res))
              {
                native_big_integer const l{ l_val };
                return make_box<obj::big_integer>(l * typed_r->data);
              }

              return make_box(res);
            }
            else
            {
              return make_box(l_val * typed_r->data);
            }
          },
          r,
          typed_l->data);
      },
      l,
      r);
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

  object_ref promoting_inc(object_ref const l)
  {
    return visit_number_like(
      [](auto const typed_l) -> object_ref {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(jtl::is_any_same<T, obj::integer, obj::small_integer>)
        {
          i64 res{};

          if(__builtin_add_overflow(typed_l->data, 1ll, &res))
          {
            native_big_integer const v{ typed_l->data };
            return make_box<obj::big_integer>(v + 1ll);
          }

          return make_box(res);
        }
        else
        {
          return make_box(typed_l->data + 1ll);
        }
      },
      l);
  }

  object_ref dec(object_ref const l)
  {
    return visit_number_like(
      [](auto const typed_l) -> object_ref { return make_box(typed_l->data - 1ll).erase(); },
      l);
  }

  object_ref promoting_dec(object_ref const l)
  {
    return visit_number_like(
      [](auto const typed_l) -> object_ref {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(jtl::is_any_same<T, obj::integer, obj::small_integer>)
        {
          i64 res{};

          if(__builtin_sub_overflow(typed_l->data, 1ll, &res))
          {
            native_big_integer const v{ typed_l->data };
            return make_box<obj::big_integer>(v - 1ll);
          }

          return make_box(res);
        }
        else
        {
          return make_box(typed_l->data - 1ll);
        }
      },
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
          return 0ll < typed_l->data;
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
    if(l.get_type() == object_type::big_integer)
    {
      return expect_object<obj::big_integer>(l)->data % 2 == 0;
    }

    return to_i64(l) % 2 == 0;
  }

  bool is_odd(object_ref const l)
  {
    if(l.get_type() == object_type::big_integer)
    {
      return expect_object<obj::big_integer>(l)->data % 2 != 0;
    }

    return to_i64(l) % 2 != 0;
  }

  bool is_equiv(object_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, object_ref const r) -> bool {
        return visit_number_like(
          [](auto const typed_r, auto const &typed_l) -> bool {
            auto const data_l{ to_real(typed_l) };
            auto const data_r{ to_real(typed_r->data) };

            using C
              = std::common_type_t<jtl::decay_t<decltype(data_l)>, jtl::decay_t<decltype(data_r)>>;
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
    auto const l_data{ to_i64(l) };
    return ~l_data;
  }

  i64 bit_and(object_ref const l, object_ref const r)
  {
    auto const l_data{ to_i64(l) };
    auto const r_data{ to_i64(r) };

    return l_data & r_data;
  }

  i64 bit_or(object_ref const l, object_ref const r)
  {
    auto const l_data{ to_i64(l) };
    auto const r_data{ to_i64(r) };

    return l_data | r_data;
  }

  i64 bit_xor(object_ref const l, object_ref const r)
  {
    auto const l_data{ to_i64(l) };
    auto const r_data{ to_i64(r) };

    return l_data ^ r_data;
  }

  i64 bit_and_not(object_ref const l, object_ref const r)
  {
    auto const l_data{ to_i64(l) };
    auto const r_data{ to_i64(r) };

    return l_data & (~r_data);
  }

  i64 bit_clear(object_ref const l, object_ref const r)
  {
    auto const l_data{ to_i64(l) };
    auto const r_data{ to_i64(r) };

    return l_data & ~(static_cast<i64>(1) << r_data);
  }

  i64 bit_set(object_ref const l, object_ref const r)
  {
    auto const l_data{ to_i64(l) };
    auto const r_data{ to_i64(r) };

    return l_data | (static_cast<i64>(1) << r_data);
  }

  i64 bit_flip(object_ref const l, object_ref const r)
  {
    auto const l_data{ to_i64(l) };
    auto const r_data{ to_i64(r) };

    return l_data ^ (static_cast<i64>(1) << r_data);
  }

  bool bit_test(object_ref const l, object_ref const r)
  {
    auto const l_data{ to_i64(l) };
    auto const r_data{ to_i64(r) };

    return (l_data >> r_data) & static_cast<i64>(1);
  }

  i64 bit_shift_left(object_ref const l, object_ref const r)
  {
    auto const l_data{ to_i64(l) };
    auto const r_data{ to_i64(r) };

    return l_data << r_data;
  }

  i64 bit_shift_right(object_ref const l, object_ref const r)
  {
    auto const l_data{ to_i64(l) };
    auto const r_data{ to_i64(r) };

    return l_data >> r_data;
  }

  i64 bit_unsigned_shift_right(object_ref const l, object_ref const r)
  {
    auto const l_data{ to_i64(l) };
    auto const r_data{ to_i64(r) };

    return static_cast<i64>(static_cast<u64>(l_data) >> static_cast<u64>(r_data));
  }

  f64 rand()
  {
    static std::random_device dev;
    static std::mt19937 gen{ dev() };
    static std::uniform_real_distribution<f64> dis(0.0, 1.0);
    return dis(gen);
  }

  object_ref abs(object_ref const l)
  {
    return visit_number_like(
      [](auto const typed_l) -> object_ref {
        return typed_l->data < 0ll ? make_box(-1ll * typed_l->data).erase()
                                   : make_box(typed_l->data).erase();
      },
      l);
  }

  object_ref abs(obj::nil_ref const l)
  {
    throw std::runtime_error{ util::format("not a number: {}", to_code_string(l)) };
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
    return visit_number_like(
      [](auto const typed_l, auto const r) -> f64 {
        return visit_number_like(
          [](auto const typed_r, auto const &typed_l) -> f64 {
            auto const typed_r_data{ to_real(typed_r->data) };
            auto const typed_l_data{ to_real(typed_l) };
            using C = std::common_type_t<decltype(typed_l_data), decltype(typed_r_data)>;
            return std::pow(static_cast<C>(typed_l_data), static_cast<C>(typed_r_data));
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  f64 pow(obj::integer_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> f64 {
        auto const typed_r_data{ to_real(typed_r->data) };
        auto const typed_l_data{ to_real(typed_l) };
        using C = std::common_type_t<decltype(typed_l_data), decltype(typed_r_data)>;
        return std::pow(static_cast<C>(typed_l_data), static_cast<C>(typed_r_data));
      },
      r,
      l->data);
  }

  f64 pow(object_ref const l, obj::integer_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> f64 {
        auto const typed_l_data{ to_real(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), jtl::decay_t<decltype(typed_r)>>;
        return std::pow(static_cast<C>(typed_l_data), static_cast<C>(typed_r));
      },
      l,
      r->data);
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
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> f64 {
        auto const typed_r_data{ to_real(typed_r->data) };
        using C = std::common_type_t<jtl::decay_t<decltype(typed_l)>, decltype(typed_r_data)>;
        return std::pow(static_cast<C>(typed_l), static_cast<C>(typed_r_data));
      },
      r,
      l->data);
  }

  f64 pow(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> f64 {
        auto const typed_l_data{ to_real(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), jtl::decay_t<decltype(typed_r)>>;
        return std::pow(static_cast<C>(typed_l_data), static_cast<C>(typed_r));
      },
      l,
      r->data);
  }

  f64 pow(obj::real_ref const l, obj::integer_ref const r)
  {
    return std::pow(l->data, static_cast<f64>(r->data));
  }

  f64 pow(obj::integer_ref const l, obj::real_ref const r)
  {
    return std::pow(static_cast<f64>(l->data), r->data);
  }

  f64 pow(object_ref const l, obj::ratio_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> f64 {
        auto const typed_l_data{ to_real(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), jtl::decay_t<decltype(typed_r)>>;
        return std::pow(static_cast<C>(typed_l_data), static_cast<C>(typed_r));
      },
      l,
      r->to_real());
  }

  f64 pow(obj::ratio_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> f64 {
        auto const typed_r_data{ to_real(typed_r->data) };
        using C = std::common_type_t<decltype(typed_r_data), jtl::decay_t<decltype(typed_l)>>;
        return std::pow(static_cast<C>(typed_r_data), static_cast<C>(typed_l));
      },
      r,
      l->to_real());
  }

  f64 pow(obj::ratio_ref const l, obj::ratio_ref const r)
  {
    return std::pow(l->to_real(), r->to_real());
  }

  object_ref pow(object_ref const l, f64 const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        auto const typed_l_data{ to_real(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), jtl::decay_t<decltype(typed_r)>>;
        return make_box(std::pow(static_cast<C>(typed_l_data), static_cast<C>(typed_r)));
      },
      l,
      r);
  }

  object_ref pow(f64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        auto const typed_r_data{ to_real(typed_r->data) };
        using C = std::common_type_t<jtl::decay_t<decltype(typed_l)>, decltype(typed_r_data)>;
        return make_box(std::pow(static_cast<C>(typed_l), static_cast<C>(typed_r_data)));
      },
      r,
      l);
  }

  f64 pow(f64 const l, f64 const r)
  {
    return std::pow(l, r);
  }

  f64 pow(i64 const l, f64 const r)
  {
    return std::pow(static_cast<f64>(l), r);
  }

  f64 pow(f64 const l, i64 const r)
  {
    return std::pow(l, static_cast<f64>(r));
  }

  f64 pow(object_ref const l, i64 const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> f64 {
        auto const typed_l_data{ to_real(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), jtl::decay_t<decltype(typed_r)>>;
        return std::pow(static_cast<C>(typed_l_data), static_cast<C>(typed_r));
      },
      l,
      r);
  }

  f64 pow(i64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> f64 {
        auto const typed_r_data{ to_real(typed_r->data) };
        using C = std::common_type_t<jtl::decay_t<decltype(typed_l)>, decltype(typed_r_data)>;
        return std::pow(static_cast<C>(typed_l), static_cast<C>(typed_r_data));
      },
      r,
      l);
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

  i64 to_int(obj::nil_ref const l)
  {
    throw std::runtime_error{ util::format("not a number: {}", to_code_string(l)) };
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

  f64 to_real(object_ref const o)
  {
    return visit_number_like(
      [](auto const typed_o) -> f64 { return typed_o->to_real(); },
      [=]() -> f64 { throw std::runtime_error{ util::format("not a number: {}", to_string(o)) }; },
      o);
  }

  bool is_number(object_ref const o)
  {
    return visit_number_like([=](auto const) -> bool { return true; },
                             [=]() -> bool { return false; },
                             o);
  }

  object_ref number(object_ref const o)
  {
    return visit_number_like([](auto const typed_l) -> object_ref { return typed_l; }, o);
  }

  bool is_integer(object_ref const o)
  {
    return detail::is_small_int(o.data) || o.get_type() == object_type::integer;
  }

  bool is_real(object_ref const o)
  {
    return o.get_type() == object_type::real;
  }

  bool is_ratio(object_ref const o)
  {
    return o.get_type() == object_type::ratio;
  }

  bool is_boolean(object_ref const o)
  {
    return o.get_type() == object_type::boolean;
  }

  bool is_nan(object_ref const o)
  {
    return visit_number_like(
      [=](auto const typed_o) -> bool {
        using T = typename jtl::decay_t<decltype(typed_o)>::value_type;

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
        using T = typename jtl::decay_t<decltype(typed_o)>::value_type;

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
      throw make_box(util::format("Expected string, got {}", object_type_str(o.get_type())))
        .erase();
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
      throw make_box(util::format("Expected string, got {}", object_type_str(o.get_type())))
        .erase();
    }
  }

  bool is_big_integer(object_ref const o)
  {
    return o.get_type() == object_type::big_integer;
  }

  obj::big_integer_ref to_big_integer(object_ref const o)
  {
    return visit_object(
      [&](auto const typed_o) -> obj::big_integer_ref {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(std::same_as<T, obj::big_integer>)
        {
          return typed_o;
        }
        else if constexpr(jtl::is_any_same<T,
                                           obj::integer,
                                           obj::small_integer,
                                           obj::persistent_string>)
        {
          return make_box<obj::big_integer>(typed_o->data);
        }
        else if constexpr(jtl::is_any_same<T, obj::real, obj::ratio, obj::big_decimal>)
        {
          return make_box<obj::big_integer>(typed_o->to_integer());
        }
        else
        {
          throw make_box(
            util::format("Expected a numeric value, got {}", object_type_str(o.get_type())))
            .erase();
        }
      },
      o);
  }

  bool is_big_decimal(object_ref const o)
  {
    return o.get_type() == object_type::big_decimal;
  }

  obj::big_decimal_ref to_big_decimal(object_ref const o)
  {
    return visit_object(
      [&](auto const typed_o) -> obj::big_decimal_ref {
        using T = typename jtl::decay_t<decltype(typed_o)>::value_type;

        if constexpr(jtl::is_any_same<T, obj::integer, obj::small_integer>)
        {
          return make_box<obj::big_decimal>(typed_o->to_real());
        }
        else if constexpr(
          jtl::is_any_same<T, obj::big_integer, obj::real, obj::ratio, obj::persistent_string>)
        {
          return make_box<obj::big_decimal>(typed_o->data);
        }
        else if constexpr(std::same_as<T, obj::big_decimal>)
        {
          return typed_o;
        }
        else
        {
          throw make_box(
            util::format("Expected a numeric value, got {}", object_type_str(o.get_type())))
            .erase();
        }
      },
      o);
  }
}
