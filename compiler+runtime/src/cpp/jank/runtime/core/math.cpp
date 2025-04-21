#include <random>

#include <jank/runtime/core/math.hpp>
#include <jank/runtime/behavior/number_like.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::runtime
{
  template <typename T>
  static auto to_number(T const &t)
  {
    if constexpr(std::same_as<T, obj::ratio_data>)
    {
      return t.to_real();
    }
    else
    {
      return t;
    }
  }

  /* TODO: visit_number_like */
  object_ref add(object_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const r) -> object_ref {
        return visit_number_like(
          [](auto const typed_r, auto const typed_l) -> object_ref {
            return make_box(typed_l + typed_r->data).erase();
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  object_ref add(obj::integer_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return make_box(typed_l + typed_r->data);
      },
      r,
      l->data);
  }

  object_ref add(object_ref const l, obj::integer_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return make_box(typed_l->data + typed_r);
      },
      l,
      r->data);
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
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> f64 { return typed_l + typed_r->data; },
      r,
      l->data);
  }

  f64 add(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> f64 { return typed_l->data + typed_r; },
      l,
      r->data);
  }

  f64 add(obj::real_ref const l, obj::integer_ref const r)
  {
    return l->data + static_cast<f64>(r->data);
  }

  f64 add(obj::integer_ref const l, obj::real_ref const r)
  {
    return static_cast<f64>(l->data) + r->data;
  }

  f64 add(object_ref const l, f64 const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> f64 { return typed_l->data + typed_r; },
      l,
      r);
  }

  f64 add(f64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> f64 { return typed_l + typed_r->data; },
      r,
      l);
  }

  f64 add(f64 const l, f64 const r)
  {
    return l + r;
  }

  f64 add(i64 const l, f64 const r)
  {
    return static_cast<f64>(l) + r;
  }

  f64 add(f64 const l, i64 const r)
  {
    return l + static_cast<f64>(r);
  }

  object_ref add(object_ref const l, i64 const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return make_box(typed_l->data + typed_r);
      },
      l,
      r);
  }

  object_ref add(i64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return make_box(typed_l + typed_r->data);
      },
      r,
      l);
  }

  i64 add(i64 const l, i64 const r)
  {
    return l + r;
  }

  object_ref sub(object_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const r) -> object_ref {
        return visit_number_like(
          [](auto const typed_r, auto const typed_l) -> object_ref {
            return make_box(typed_l - typed_r->data).erase();
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  object_ref sub(obj::integer_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return make_box(typed_l - typed_r->data);
      },
      r,
      l->data);
  }

  object_ref sub(object_ref const l, obj::integer_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return make_box(typed_l->data - typed_r);
      },
      l,
      r->data);
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
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> f64 { return typed_l - typed_r->data; },
      r,
      l->data);
  }

  f64 sub(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> f64 { return typed_l->data - typed_r; },
      l,
      r->data);
  }

  f64 sub(obj::real_ref const l, obj::integer_ref const r)
  {
    return l->data - static_cast<f64>(r->data);
  }

  f64 sub(obj::integer_ref const l, obj::real_ref const r)
  {
    return static_cast<f64>(l->data) - r->data;
  }

  f64 sub(object_ref const l, f64 const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> f64 { return typed_l->data - typed_r; },
      l,
      r);
  }

  f64 sub(f64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> f64 { return typed_l - typed_r->data; },
      r,
      l);
  }

  f64 sub(f64 const l, f64 const r)
  {
    return l - r;
  }

  f64 sub(i64 const l, f64 const r)
  {
    return static_cast<f64>(l) - r;
  }

  f64 sub(f64 const l, i64 const r)
  {
    return l - static_cast<f64>(r);
  }

  object_ref sub(object_ref const l, i64 const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return make_box(typed_l->data - typed_r);
      },
      l,
      r);
  }

  object_ref sub(i64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return make_box(typed_l - typed_r->data);
      },
      r,
      l);
  }

  i64 sub(i64 const l, i64 const r)
  {
    return l - r;
  }

  object_ref div(object_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const r) -> object_ref {
        return visit_number_like(
          [](auto const typed_r, auto const typed_l) -> object_ref {
            return make_box(typed_l / typed_r->data).erase();
          },
          r,
          typed_l->data);
      },
      l,
      r);
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

  f64 div(obj::real_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> f64 { return typed_l / typed_r->data; },
      r,
      l->data);
  }

  f64 div(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> f64 { return typed_l->data / typed_r; },
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

  f64 div(object_ref const l, f64 const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> f64 { return typed_l->data / typed_r; },
      l,
      r);
  }

  f64 div(f64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> f64 { return typed_l / typed_r->data; },
      r,
      l);
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

  object_ref mul(object_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const r) -> object_ref {
        return visit_number_like(
          [](auto const typed_r, auto const typed_l) -> object_ref {
            return make_box(typed_l * typed_r->data).erase();
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  object_ref mul(obj::integer_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return make_box(typed_l * typed_r->data);
      },
      r,
      l->data);
  }

  object_ref mul(object_ref const l, obj::integer_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return make_box(typed_l->data * typed_r);
      },
      l,
      r->data);
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
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> f64 { return typed_l * typed_r->data; },
      r,
      l->data);
  }

  f64 mul(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> f64 { return typed_l->data * typed_r; },
      l,
      r->data);
  }

  f64 mul(obj::real_ref const l, obj::integer_ref const r)
  {
    return l->data * static_cast<f64>(r->data);
  }

  f64 mul(obj::integer_ref const l, obj::real_ref const r)
  {
    return static_cast<f64>(l->data) * r->data;
  }

  f64 mul(object_ref const l, f64 const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> f64 { return typed_l->data * typed_r; },
      l,
      r);
  }

  f64 mul(f64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> f64 { return typed_l * typed_r->data; },
      r,
      l);
  }

  f64 mul(f64 const l, f64 const r)
  {
    return l * r;
  }

  f64 mul(i64 const l, f64 const r)
  {
    return static_cast<f64>(l) * r;
  }

  f64 mul(f64 const l, i64 const r)
  {
    return l * static_cast<f64>(r);
  }

  object_ref mul(object_ref const l, i64 const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return make_box(typed_l->data * typed_r);
      },
      l,
      r);
  }

  object_ref mul(i64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return make_box(typed_l * typed_r->data);
      },
      r,
      l);
  }

  i64 mul(i64 const l, i64 const r)
  {
    return l * r;
  }

  object_ref rem(object_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const r) -> object_ref {
        return visit_number_like(
          [](auto const typed_r, auto const typed_l) -> object_ref {
            auto const typed_l_data{ to_number(typed_l) };
            auto const typed_r_data{ to_number(typed_r->data) };
            return make_box(std::fmod(typed_l_data, typed_r_data)).erase();
          },
          r,
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
          [](auto const typed_r, auto const typed_l) -> object_ref {
            auto const typed_l_data{ to_number(typed_l) };
            auto const typed_r_data{ to_number(typed_r->data) };
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
            if(typed_r_data == 0ll)
            {
#pragma clang diagnostic pop
              throw make_box("Illegal divide by zero in 'quot'").erase();
            }
            else
            {
              return make_box(long(typed_l_data / typed_r_data)).erase();
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
          [](auto const typed_r, auto const typed_l) -> bool {
            auto const data_l{ to_number(typed_l) };
            auto const data_r{ to_number(typed_r->data) };

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
    return visit_number_like(
      [](auto const typed_l, auto const r) -> bool {
        return visit_number_like(
          [](auto const typed_r, auto const typed_l) -> bool {
            return typed_l < typed_r->data;
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  bool lt(obj::integer_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> bool { return typed_l < typed_r->data; },
      r,
      l->data);
  }

  bool lt(object_ref const l, obj::integer_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> bool { return typed_l->data < typed_r; },
      l,
      r->data);
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
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> bool { return typed_l < typed_r->data; },
      r,
      l->data);
  }

  bool lt(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> bool { return typed_l->data < typed_r; },
      l,
      r->data);
  }

  bool lt(obj::real_ref const l, obj::integer_ref const r)
  {
    return l->data < static_cast<f64>(r->data);
  }

  bool lt(obj::integer_ref const l, obj::real_ref const r)
  {
    return static_cast<f64>(l->data) < r->data;
  }

  bool lt(object_ref const l, f64 const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> bool { return typed_l->data < typed_r; },
      l,
      r);
  }

  bool lt(f64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> bool { return typed_l < typed_r->data; },
      r,
      l);
  }

  bool lt(f64 const l, f64 const r)
  {
    return l < r;
  }

  bool lt(i64 const l, f64 const r)
  {
    return static_cast<f64>(l) < r;
  }

  bool lt(f64 const l, i64 const r)
  {
    return l < static_cast<f64>(r);
  }

  bool lt(object_ref const l, i64 const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> bool { return typed_l->data < typed_r; },
      l,
      r);
  }

  bool lt(i64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> bool { return typed_l < typed_r->data; },
      r,
      l);
  }

  bool lt(i64 const l, i64 const r)
  {
    return l < r;
  }

  bool lte(object_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const r) -> bool {
        return visit_number_like(
          [](auto const typed_r, auto const typed_l) -> bool {
            return typed_l <= typed_r->data;
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  bool lte(obj::integer_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> bool {
        return typed_l <= typed_r->data;
      },
      r,
      l->data);
  }

  bool lte(object_ref const l, obj::integer_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> bool {
        return typed_l->data <= typed_r;
      },
      l,
      r->data);
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
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> bool {
        return typed_l <= typed_r->data;
      },
      r,
      l->data);
  }

  bool lte(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> bool {
        return typed_l->data <= typed_r;
      },
      l,
      r->data);
  }

  bool lte(obj::real_ref const l, obj::integer_ref const r)
  {
    return l->data <= static_cast<f64>(r->data);
  }

  bool lte(obj::integer_ref const l, obj::real_ref const r)
  {
    return static_cast<f64>(l->data) <= r->data;
  }

  bool lte(object_ref const l, f64 const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> bool {
        return typed_l->data <= typed_r;
      },
      l,
      r);
  }

  bool lte(f64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> bool {
        return typed_l <= typed_r->data;
      },
      r,
      l);
  }

  bool lte(f64 const l, f64 const r)
  {
    return l <= r;
  }

  bool lte(i64 const l, f64 const r)
  {
    return static_cast<f64>(l) <= r;
  }

  bool lte(f64 const l, i64 const r)
  {
    return l <= static_cast<f64>(r);
  }

  bool lte(object_ref const l, i64 const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> bool {
        return typed_l->data <= typed_r;
      },
      l,
      r);
  }

  bool lte(i64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> bool {
        return typed_l <= typed_r->data;
      },
      r,
      l);
  }

  bool lte(i64 const l, i64 const r)
  {
    return l <= r;
  }

  object_ref min(object_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const r) -> object_ref {
        return visit_number_like(
          [](auto const typed_r, auto const typed_l) -> object_ref {
            return typed_l < typed_r->data ? make_box(typed_l).erase()
                                           : make_box(typed_r->data).erase();
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  object_ref min(obj::integer_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return typed_l < typed_r->data ? make_box(typed_l).erase()
                                       : make_box(typed_r->data).erase();
      },
      r,
      l->data);
  }

  object_ref min(object_ref const l, obj::integer_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return typed_l->data < typed_r ? make_box(typed_l->data).erase()
                                       : make_box(typed_r).erase();
      },
      l,
      r->data);
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
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> f64 {
        auto const typed_r_data{ to_number(typed_r->data) };
        using C = std::common_type_t<decltype(typed_l), decltype(typed_r_data)>;
        return std::min(static_cast<C>(typed_l), static_cast<C>(typed_r_data));
      },
      r,
      l->data);
  }

  f64 min(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> f64 {
        auto const typed_l_data{ to_number(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), decltype(typed_r)>;
        return std::min(static_cast<C>(typed_l_data), static_cast<C>(typed_r));
      },
      l,
      r->data);
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
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> f64 {
        auto const typed_l_data{ to_number(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), decltype(typed_r)>;
        return std::min(static_cast<C>(typed_l_data), static_cast<C>(typed_r));
      },
      l,
      r);
  }

  f64 min(f64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> f64 {
        auto const typed_r_data{ to_number(typed_r->data) };
        using C = std::common_type_t<decltype(typed_r_data), decltype(typed_l)>;
        return std::min(static_cast<C>(typed_r_data), static_cast<C>(typed_l));
      },
      r,
      l);
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
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return typed_l->data < typed_r ? make_box(typed_l).erase() : make_box(typed_r).erase();
      },
      l,
      r);
  }

  object_ref min(i64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return typed_l < typed_r->data ? make_box(typed_l).erase() : make_box(typed_r).erase();
      },
      r,
      l);
  }

  i64 min(i64 const l, i64 const r)
  {
    return std::min(l, r);
  }

  object_ref max(object_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const r) -> object_ref {
        return visit_number_like(
          [](auto const typed_r, auto const typed_l) -> object_ref {
            return typed_r->data > typed_l ? make_box(typed_r).erase() : make_box(typed_l).erase();
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  object_ref max(obj::integer_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return typed_l > typed_r->data ? make_box(typed_l).erase() : make_box(typed_r).erase();
      },
      r,
      l->data);
  }

  object_ref max(object_ref const l, obj::integer_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return typed_l->data > typed_r ? make_box(typed_l).erase() : make_box(typed_r).erase();
      },
      l,
      r->data);
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
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> f64 {
        auto const typed_r_data{ to_number(typed_r->data) };
        using C = std::common_type_t<decltype(typed_l), decltype(typed_r_data)>;
        return std::max(static_cast<C>(typed_l), static_cast<C>(typed_r_data));
      },
      r,
      l->data);
  }

  f64 max(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> f64 {
        auto const typed_l_data{ to_number(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), decltype(typed_r)>;
        return std::max(static_cast<C>(typed_r), static_cast<C>(typed_l_data));
      },
      l,
      r->data);
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
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> f64 {
        auto const typed_l_data{ to_number(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), decltype(typed_r)>;
        return std::max(static_cast<C>(typed_r), static_cast<C>(typed_l_data));
      },
      l,
      r);
  }

  f64 max(f64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> f64 {
        auto const typed_r_data{ to_number(typed_r->data) };
        using C = std::common_type_t<decltype(typed_l), decltype(typed_r_data)>;
        return std::max(static_cast<C>(typed_l), static_cast<C>(typed_r_data));
      },
      r,
      l);
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
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return typed_l->data > typed_r ? make_box(typed_l).erase() : make_box(typed_r).erase();
      },
      l,
      r);
  }

  object_ref max(i64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return typed_l > typed_r->data ? make_box(typed_l).erase() : make_box(typed_r).erase();
      },
      r,
      l);
  }

  i64 max(i64 const l, i64 const r)
  {
    return std::max(l, r);
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
          [](auto const typed_r, auto const typed_l) -> f64 {
            auto const typed_r_data{ to_number(typed_r->data) };
            auto const typed_l_data{ to_number(typed_l) };
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
        auto const typed_r_data{ to_number(typed_r->data) };
        auto const typed_l_data{ to_number(typed_l) };
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
        auto const typed_l_data{ to_number(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), decltype(typed_r)>;
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
        auto const typed_r_data{ to_number(typed_r->data) };
        using C = std::common_type_t<decltype(typed_l), decltype(typed_r_data)>;
        return std::pow(static_cast<C>(typed_l), static_cast<C>(typed_r_data));
      },
      r,
      l->data);
  }

  f64 pow(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> f64 {
        auto const typed_l_data{ to_number(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), decltype(typed_r)>;
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

  f64 pow(object_ref const l, f64 const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> f64 {
        auto const typed_l_data{ to_number(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), decltype(typed_r)>;
        return std::pow(static_cast<C>(typed_l_data), static_cast<C>(typed_r));
      },
      l,
      r);
  }

  f64 pow(f64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> f64 {
        auto const typed_r_data{ to_number(typed_r->data) };
        using C = std::common_type_t<decltype(typed_l), decltype(typed_r_data)>;
        return std::pow(static_cast<C>(typed_l), static_cast<C>(typed_r_data));
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
        auto const typed_l_data{ to_number(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), decltype(typed_r)>;
        return std::pow(static_cast<C>(typed_l_data), static_cast<C>(typed_r));
      },
      l,
      r);
  }

  f64 pow(i64 const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> f64 {
        auto const typed_r_data{ to_number(typed_r->data) };
        using C = std::common_type_t<decltype(typed_l), decltype(typed_r_data)>;
        return std::pow(static_cast<C>(typed_l), static_cast<C>(typed_r_data));
      },
      r,
      l);
  }

  f64 pow(i64 const l, i64 const r)
  {
    return std::pow(l, r);
  }

  i64 numerator(object_ref const o)
  {
    return try_object<obj::ratio>(o)->data.numerator;
  }

  i64 denominator(object_ref const o)
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
