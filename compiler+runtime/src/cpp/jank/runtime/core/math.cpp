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

  native_integer add(obj::integer_ref const l, obj::integer_ref const r)
  {
    return l->data + r->data;
  }

  native_real add(obj::real_ref const l, obj::real_ref const r)
  {
    return l->data + r->data;
  }

  native_real add(obj::real_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_real { return typed_l + typed_r->data; },
      r,
      l->data);
  }

  native_real add(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_real { return typed_l->data + typed_r; },
      l,
      r->data);
  }

  native_real add(obj::real_ref const l, obj::integer_ref const r)
  {
    return l->data + static_cast<native_real>(r->data);
  }

  native_real add(obj::integer_ref const l, obj::real_ref const r)
  {
    return static_cast<native_real>(l->data) + r->data;
  }

  native_real add(object_ref const l, native_real const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_real { return typed_l->data + typed_r; },
      l,
      r);
  }

  native_real add(native_real const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_real { return typed_l + typed_r->data; },
      r,
      l);
  }

  native_real add(native_real const l, native_real const r)
  {
    return l + r;
  }

  native_real add(native_integer const l, native_real const r)
  {
    return static_cast<native_real>(l) + r;
  }

  native_real add(native_real const l, native_integer const r)
  {
    return l + static_cast<native_real>(r);
  }

  object_ref add(object_ref const l, native_integer const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return make_box(typed_l->data + typed_r);
      },
      l,
      r);
  }

  object_ref add(native_integer const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return make_box(typed_l + typed_r->data);
      },
      r,
      l);
  }

  native_integer add(native_integer const l, native_integer const r)
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

  native_integer sub(obj::integer_ref const l, obj::integer_ref const r)
  {
    return l->data - r->data;
  }

  native_real sub(obj::real_ref const l, obj::real_ref const r)
  {
    return l->data - r->data;
  }

  native_real sub(obj::real_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_real { return typed_l - typed_r->data; },
      r,
      l->data);
  }

  native_real sub(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_real { return typed_l->data - typed_r; },
      l,
      r->data);
  }

  native_real sub(obj::real_ref const l, obj::integer_ref const r)
  {
    return l->data - static_cast<native_real>(r->data);
  }

  native_real sub(obj::integer_ref const l, obj::real_ref const r)
  {
    return static_cast<native_real>(l->data) - r->data;
  }

  native_real sub(object_ref const l, native_real const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_real { return typed_l->data - typed_r; },
      l,
      r);
  }

  native_real sub(native_real const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_real { return typed_l - typed_r->data; },
      r,
      l);
  }

  native_real sub(native_real const l, native_real const r)
  {
    return l - r;
  }

  native_real sub(native_integer const l, native_real const r)
  {
    return static_cast<native_real>(l) - r;
  }

  native_real sub(native_real const l, native_integer const r)
  {
    return l - static_cast<native_real>(r);
  }

  object_ref sub(object_ref const l, native_integer const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return make_box(typed_l->data - typed_r);
      },
      l,
      r);
  }

  object_ref sub(native_integer const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return make_box(typed_l - typed_r->data);
      },
      r,
      l);
  }

  native_integer sub(native_integer const l, native_integer const r)
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

  native_integer div(obj::integer_ref const l, obj::integer_ref const r)
  {
    return l->data / r->data;
  }

  native_real div(obj::real_ref const l, obj::real_ref const r)
  {
    return l->data / r->data;
  }

  native_real div(obj::real_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_real { return typed_l / typed_r->data; },
      r,
      l->data);
  }

  native_real div(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_real { return typed_l->data / typed_r; },
      l,
      r->data);
  }

  native_real div(obj::real_ref const l, obj::integer_ref const r)
  {
    return l->data / static_cast<native_real>(r->data);
  }

  native_real div(obj::integer_ref const l, obj::real_ref const r)
  {
    return static_cast<native_real>(l->data) / r->data;
  }

  native_real div(object_ref const l, native_real const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_real { return typed_l->data / typed_r; },
      l,
      r);
  }

  native_real div(native_real const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_real { return typed_l / typed_r->data; },
      r,
      l);
  }

  native_real div(native_real const l, native_real const r)
  {
    return l / r;
  }

  native_real div(native_integer const l, native_real const r)
  {
    return static_cast<native_real>(l) / r;
  }

  native_real div(native_real const l, native_integer const r)
  {
    return l / static_cast<native_real>(r);
  }

  object_ref div(object_ref const l, native_integer const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return make_box(typed_l->data / typed_r);
      },
      l,
      r);
  }

  object_ref div(native_integer const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return make_box(typed_l / typed_r->data);
      },
      r,
      l);
  }

  native_integer div(native_integer const l, native_integer const r)
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

  native_integer mul(obj::integer_ref const l, obj::integer_ref const r)
  {
    return l->data * r->data;
  }

  native_real mul(obj::real_ref const l, obj::real_ref const r)
  {
    return l->data * r->data;
  }

  native_real mul(obj::real_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_real { return typed_l * typed_r->data; },
      r,
      l->data);
  }

  native_real mul(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_real { return typed_l->data * typed_r; },
      l,
      r->data);
  }

  native_real mul(obj::real_ref const l, obj::integer_ref const r)
  {
    return l->data * static_cast<native_real>(r->data);
  }

  native_real mul(obj::integer_ref const l, obj::real_ref const r)
  {
    return static_cast<native_real>(l->data) * r->data;
  }

  native_real mul(object_ref const l, native_real const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_real { return typed_l->data * typed_r; },
      l,
      r);
  }

  native_real mul(native_real const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_real { return typed_l * typed_r->data; },
      r,
      l);
  }

  native_real mul(native_real const l, native_real const r)
  {
    return l * r;
  }

  native_real mul(native_integer const l, native_real const r)
  {
    return static_cast<native_real>(l) * r;
  }

  native_real mul(native_real const l, native_integer const r)
  {
    return l * static_cast<native_real>(r);
  }

  object_ref mul(object_ref const l, native_integer const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return make_box(typed_l->data * typed_r);
      },
      l,
      r);
  }

  object_ref mul(native_integer const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return make_box(typed_l * typed_r->data);
      },
      r,
      l);
  }

  native_integer mul(native_integer const l, native_integer const r)
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
            if(typed_r_data == 0LL)
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
        auto const ret{ make_box(typed_l->data + 1LL) };
        object_ref r{ ret };
        return r;
      },
      l);
  }

  object_ref dec(object_ref const l)
  {
    return visit_number_like(
      [](auto const typed_l) -> object_ref { return make_box(typed_l->data - 1LL).erase(); },
      l);
  }

  native_bool is_zero(object_ref const l)
  {
    return visit_number_like(
      [](auto const typed_l) -> native_bool {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
        {
          return typed_l->data == 0LL;
        }
#pragma clang diagnostic pop
      },
      l);
  }

  native_bool is_pos(object_ref const l)
  {
    return visit_number_like(
      [](auto const typed_l) -> native_bool {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
        {
          return typed_l->data > 0LL;
        }
#pragma clang diagnostic pop
      },
      l);
  }

  native_bool is_neg(object_ref const l)
  {
    return visit_number_like(
      [](auto const typed_l) -> native_bool {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
        {
          return typed_l->data < 0LL;
        }
#pragma clang diagnostic pop
      },
      l);
  }

  native_bool is_even(object_ref const l)
  {
    return visit_type<obj::integer>(
      [=](auto const typed_l) -> native_bool { return typed_l->data % 2 == 0; },
      l);
  }

  native_bool is_odd(object_ref const l)
  {
    return visit_type<obj::integer>(
      [=](auto const typed_l) -> native_bool { return typed_l->data % 2 == 1; },
      l);
  }

  native_bool is_equiv(object_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, object_ref const r) -> native_bool {
        return visit_number_like(
          [](auto const typed_r, auto const typed_l) -> native_bool {
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

  native_integer bit_not(object_ref const l)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l) -> native_integer { return ~typed_l->data; },
      l);
  }

  native_integer bit_and(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> native_integer {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> native_integer {
            return typed_l & typed_r->data;
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  native_integer bit_or(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> native_integer {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> native_integer {
            return typed_l | typed_r->data;
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  native_integer bit_xor(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> native_integer {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> native_integer {
            return typed_l ^ typed_r->data;
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  native_integer bit_and_not(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> native_integer {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> native_integer {
            return typed_l & (~typed_r->data);
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  native_integer bit_clear(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> native_integer {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> native_integer {
            return typed_l & ~(static_cast<native_integer>(1) << typed_r->data);
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  native_integer bit_set(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> native_integer {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> native_integer {
            return typed_l | (static_cast<native_integer>(1) << typed_r->data);
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  native_integer bit_flip(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> native_integer {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> native_integer {
            return typed_l ^ (static_cast<native_integer>(1) << typed_r->data);
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  native_bool bit_test(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> native_bool {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> native_bool {
            return (typed_l >> typed_r->data) & static_cast<native_integer>(1);
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  native_integer bit_shift_left(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> native_integer {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> native_integer {
            return typed_l << typed_r->data;
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  native_integer bit_shift_right(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> native_integer {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> native_integer {
            return typed_l >> typed_r->data;
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  native_integer bit_unsigned_shift_right(object_ref const l, object_ref const r)
  {
    return visit_type<obj::integer>(
      [](auto const typed_l, auto const r) -> native_integer {
        return visit_type<obj::integer>(
          [](auto const typed_r, auto const typed_l) -> native_integer {
            using uni = std::make_unsigned_t<native_integer>;
            return static_cast<uni>(typed_l) >> static_cast<uni>(typed_r->data);
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  native_real rand()
  {
    static std::mt19937 gen;
    static std::uniform_real_distribution<native_real> dis(0.0, 1.0);
    return dis(gen);
  }

  native_bool lt(object_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const r) -> native_bool {
        return visit_number_like(
          [](auto const typed_r, auto const typed_l) -> native_bool {
            return typed_l < typed_r->data;
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  native_bool lt(obj::integer_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_bool { return typed_l < typed_r->data; },
      r,
      l->data);
  }

  native_bool lt(object_ref const l, obj::integer_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_bool { return typed_l->data < typed_r; },
      l,
      r->data);
  }

  native_bool lt(obj::integer_ref const l, obj::integer_ref const r)
  {
    return l->data < r->data;
  }

  native_bool lt(obj::real_ref const l, obj::real_ref const r)
  {
    return l->data < r->data;
  }

  native_bool lt(obj::real_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_bool { return typed_l < typed_r->data; },
      r,
      l->data);
  }

  native_bool lt(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_bool { return typed_l->data < typed_r; },
      l,
      r->data);
  }

  native_bool lt(obj::real_ref const l, obj::integer_ref const r)
  {
    return l->data < static_cast<native_real>(r->data);
  }

  native_bool lt(obj::integer_ref const l, obj::real_ref const r)
  {
    return static_cast<native_real>(l->data) < r->data;
  }

  native_bool lt(object_ref const l, native_real const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_bool { return typed_l->data < typed_r; },
      l,
      r);
  }

  native_bool lt(native_real const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_bool { return typed_l < typed_r->data; },
      r,
      l);
  }

  native_bool lt(native_real const l, native_real const r)
  {
    return l < r;
  }

  native_bool lt(native_integer const l, native_real const r)
  {
    return static_cast<native_real>(l) < r;
  }

  native_bool lt(native_real const l, native_integer const r)
  {
    return l < static_cast<native_real>(r);
  }

  native_bool lt(object_ref const l, native_integer const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_bool { return typed_l->data < typed_r; },
      l,
      r);
  }

  native_bool lt(native_integer const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_bool { return typed_l < typed_r->data; },
      r,
      l);
  }

  native_bool lt(native_integer const l, native_integer const r)
  {
    return l < r;
  }

  native_bool lte(object_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const r) -> native_bool {
        return visit_number_like(
          [](auto const typed_r, auto const typed_l) -> native_bool {
            return typed_l <= typed_r->data;
          },
          r,
          typed_l->data);
      },
      l,
      r);
  }

  native_bool lte(obj::integer_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_bool {
        return typed_l <= typed_r->data;
      },
      r,
      l->data);
  }

  native_bool lte(object_ref const l, obj::integer_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_bool {
        return typed_l->data <= typed_r;
      },
      l,
      r->data);
  }

  native_bool lte(obj::integer_ref const l, obj::integer_ref const r)
  {
    return l->data <= r->data;
  }

  native_bool lte(obj::real_ref const l, obj::real_ref const r)
  {
    return l->data <= r->data;
  }

  native_bool lte(obj::real_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_bool {
        return typed_l <= typed_r->data;
      },
      r,
      l->data);
  }

  native_bool lte(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_bool {
        return typed_l->data <= typed_r;
      },
      l,
      r->data);
  }

  native_bool lte(obj::real_ref const l, obj::integer_ref const r)
  {
    return l->data <= static_cast<native_real>(r->data);
  }

  native_bool lte(obj::integer_ref const l, obj::real_ref const r)
  {
    return static_cast<native_real>(l->data) <= r->data;
  }

  native_bool lte(object_ref const l, native_real const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_bool {
        return typed_l->data <= typed_r;
      },
      l,
      r);
  }

  native_bool lte(native_real const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_bool {
        return typed_l <= typed_r->data;
      },
      r,
      l);
  }

  native_bool lte(native_real const l, native_real const r)
  {
    return l <= r;
  }

  native_bool lte(native_integer const l, native_real const r)
  {
    return static_cast<native_real>(l) <= r;
  }

  native_bool lte(native_real const l, native_integer const r)
  {
    return l <= static_cast<native_real>(r);
  }

  native_bool lte(object_ref const l, native_integer const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_bool {
        return typed_l->data <= typed_r;
      },
      l,
      r);
  }

  native_bool lte(native_integer const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_bool {
        return typed_l <= typed_r->data;
      },
      r,
      l);
  }

  native_bool lte(native_integer const l, native_integer const r)
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

  native_integer min(obj::integer_ref const l, obj::integer_ref const r)
  {
    return std::min(l->data, r->data);
  }

  native_real min(obj::real_ref const l, obj::real_ref const r)
  {
    return std::min(l->data, r->data);
  }

  native_real min(obj::real_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_real {
        auto const typed_r_data{ to_number(typed_r->data) };
        using C = std::common_type_t<decltype(typed_l), decltype(typed_r_data)>;
        return std::min(static_cast<C>(typed_l), static_cast<C>(typed_r_data));
      },
      r,
      l->data);
  }

  native_real min(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_real {
        auto const typed_l_data{ to_number(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), decltype(typed_r)>;
        return std::min(static_cast<C>(typed_l_data), static_cast<C>(typed_r));
      },
      l,
      r->data);
  }

  native_real min(obj::real_ref const l, obj::integer_ref const r)
  {
    return std::min(l->data, static_cast<native_real>(r->data));
  }

  native_real min(obj::integer_ref const l, obj::real_ref const r)
  {
    return std::min(static_cast<native_real>(l->data), r->data);
  }

  native_real min(object_ref const l, native_real const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_real {
        auto const typed_l_data{ to_number(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), decltype(typed_r)>;
        return std::min(static_cast<C>(typed_l_data), static_cast<C>(typed_r));
      },
      l,
      r);
  }

  native_real min(native_real const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_real {
        auto const typed_r_data{ to_number(typed_r->data) };
        using C = std::common_type_t<decltype(typed_r_data), decltype(typed_l)>;
        return std::min(static_cast<C>(typed_r_data), static_cast<C>(typed_l));
      },
      r,
      l);
  }

  native_real min(native_real const l, native_real const r)
  {
    return std::min(l, r);
  }

  native_real min(native_integer const l, native_real const r)
  {
    return std::min(static_cast<native_real>(l), r);
  }

  native_real min(native_real const l, native_integer const r)
  {
    return std::min(l, static_cast<native_real>(r));
  }

  object_ref min(object_ref const l, native_integer const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return typed_l->data < typed_r ? make_box(typed_l).erase() : make_box(typed_r).erase();
      },
      l,
      r);
  }

  object_ref min(native_integer const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return typed_l < typed_r->data ? make_box(typed_l).erase() : make_box(typed_r).erase();
      },
      r,
      l);
  }

  native_integer min(native_integer const l, native_integer const r)
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

  native_integer max(obj::integer_ref const l, obj::integer_ref const r)
  {
    return std::max(l->data, r->data);
  }

  native_real max(obj::real_ref const l, obj::real_ref const r)
  {
    return std::max(l->data, r->data);
  }

  native_real max(obj::real_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_real {
        auto const typed_r_data{ to_number(typed_r->data) };
        using C = std::common_type_t<decltype(typed_l), decltype(typed_r_data)>;
        return std::max(static_cast<C>(typed_l), static_cast<C>(typed_r_data));
      },
      r,
      l->data);
  }

  native_real max(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_real {
        auto const typed_l_data{ to_number(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), decltype(typed_r)>;
        return std::max(static_cast<C>(typed_r), static_cast<C>(typed_l_data));
      },
      l,
      r->data);
  }

  native_real max(obj::real_ref const l, obj::integer_ref const r)
  {
    return std::max(l->data, static_cast<native_real>(r->data));
  }

  native_real max(obj::integer_ref const l, obj::real_ref const r)
  {
    return std::max(static_cast<native_real>(l->data), r->data);
  }

  native_real max(object_ref const l, native_real const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_real {
        auto const typed_l_data{ to_number(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), decltype(typed_r)>;
        return std::max(static_cast<C>(typed_r), static_cast<C>(typed_l_data));
      },
      l,
      r);
  }

  native_real max(native_real const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_real {
        auto const typed_r_data{ to_number(typed_r->data) };
        using C = std::common_type_t<decltype(typed_l), decltype(typed_r_data)>;
        return std::max(static_cast<C>(typed_l), static_cast<C>(typed_r_data));
      },
      r,
      l);
  }

  native_real max(native_real const l, native_real const r)
  {
    return std::max(l, r);
  }

  native_real max(native_integer const l, native_real const r)
  {
    return std::max(static_cast<native_real>(l), r);
  }

  native_real max(native_real const l, native_integer const r)
  {
    return std::max(l, static_cast<native_real>(r));
  }

  object_ref max(object_ref const l, native_integer const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> object_ref {
        return typed_l->data > typed_r ? make_box(typed_l).erase() : make_box(typed_r).erase();
      },
      l,
      r);
  }

  object_ref max(native_integer const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> object_ref {
        return typed_l > typed_r->data ? make_box(typed_l).erase() : make_box(typed_r).erase();
      },
      r,
      l);
  }

  native_integer max(native_integer const l, native_integer const r)
  {
    return std::max(l, r);
  }

  object_ref abs(object_ref const l)
  {
    return visit_number_like(
      [](auto const typed_l) -> object_ref {
        return typed_l->data < 0LL ? make_box(-1LL * typed_l->data).erase()
                                   : make_box(typed_l->data).erase();
      },
      l);
  }

  native_integer abs(obj::integer_ref const l)
  {
    return std::abs(l->data);
  }

  native_real abs(obj::real_ref const l)
  {
    return std::fabs(l->data);
  }

  native_integer abs(native_integer const l)
  {
    return std::abs(l);
  }

  native_real abs(native_real const l)
  {
    return std::fabs(l);
  }

  native_real tan(object_ref const l)
  {
    return visit_number_like(
      [](auto const typed_l) -> native_real { return tanf(typed_l->to_real()); },
      l);
  }

  native_real sqrt(object_ref const l)
  {
    return visit_number_like(
      [](auto const typed_l) -> native_real { return std::sqrt(typed_l->to_real()); },
      l);
  }

  native_real sqrt(obj::integer_ref const l)
  {
    return std::sqrt(l->data);
  }

  native_real sqrt(obj::real_ref const l)
  {
    return std::sqrt(l->data);
  }

  native_real sqrt(native_integer const l)
  {
    return std::sqrt(l);
  }

  native_real sqrt(native_real const l)
  {
    return std::sqrt(l);
  }

  native_real pow(object_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const r) -> native_real {
        return visit_number_like(
          [](auto const typed_r, auto const typed_l) -> native_real {
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

  native_real pow(obj::integer_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_real {
        auto const typed_r_data{ to_number(typed_r->data) };
        auto const typed_l_data{ to_number(typed_l) };
        using C = std::common_type_t<decltype(typed_l_data), decltype(typed_r_data)>;
        return std::pow(static_cast<C>(typed_l_data), static_cast<C>(typed_r_data));
      },
      r,
      l->data);
  }

  native_real pow(object_ref const l, obj::integer_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_real {
        auto const typed_l_data{ to_number(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), decltype(typed_r)>;
        return std::pow(static_cast<C>(typed_l_data), static_cast<C>(typed_r));
      },
      l,
      r->data);
  }

  native_real pow(obj::integer_ref const l, obj::integer_ref const r)
  {
    return std::pow(l->data, r->data);
  }

  native_real pow(obj::real_ref const l, obj::real_ref const r)
  {
    return std::pow(l->data, r->data);
  }

  native_real pow(obj::real_ref const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_real {
        auto const typed_r_data{ to_number(typed_r->data) };
        using C = std::common_type_t<decltype(typed_l), decltype(typed_r_data)>;
        return std::pow(static_cast<C>(typed_l), static_cast<C>(typed_r_data));
      },
      r,
      l->data);
  }

  native_real pow(object_ref const l, obj::real_ref const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_real {
        auto const typed_l_data{ to_number(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), decltype(typed_r)>;
        return std::pow(static_cast<C>(typed_l_data), static_cast<C>(typed_r));
      },
      l,
      r->data);
  }

  native_real pow(obj::real_ref const l, obj::integer_ref const r)
  {
    return std::pow(l->data, static_cast<native_real>(r->data));
  }

  native_real pow(obj::integer_ref const l, obj::real_ref const r)
  {
    return std::pow(static_cast<native_real>(l->data), r->data);
  }

  native_real pow(object_ref const l, native_real const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_real {
        auto const typed_l_data{ to_number(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), decltype(typed_r)>;
        return std::pow(static_cast<C>(typed_l_data), static_cast<C>(typed_r));
      },
      l,
      r);
  }

  native_real pow(native_real const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_real {
        auto const typed_r_data{ to_number(typed_r->data) };
        using C = std::common_type_t<decltype(typed_l), decltype(typed_r_data)>;
        return std::pow(static_cast<C>(typed_l), static_cast<C>(typed_r_data));
      },
      r,
      l);
  }

  native_real pow(native_real const l, native_real const r)
  {
    return std::pow(l, r);
  }

  native_real pow(native_integer const l, native_real const r)
  {
    return std::pow(static_cast<native_real>(l), r);
  }

  native_real pow(native_real const l, native_integer const r)
  {
    return std::pow(l, static_cast<native_real>(r));
  }

  native_real pow(object_ref const l, native_integer const r)
  {
    return visit_number_like(
      [](auto const typed_l, auto const typed_r) -> native_real {
        auto const typed_l_data{ to_number(typed_l->data) };
        using C = std::common_type_t<decltype(typed_l_data), decltype(typed_r)>;
        return std::pow(static_cast<C>(typed_l_data), static_cast<C>(typed_r));
      },
      l,
      r);
  }

  native_real pow(native_integer const l, object_ref const r)
  {
    return visit_number_like(
      [](auto const typed_r, auto const typed_l) -> native_real {
        auto const typed_r_data{ to_number(typed_r->data) };
        using C = std::common_type_t<decltype(typed_l), decltype(typed_r_data)>;
        return std::pow(static_cast<C>(typed_l), static_cast<C>(typed_r_data));
      },
      r,
      l);
  }

  native_real pow(native_integer const l, native_integer const r)
  {
    return std::pow(l, r);
  }

  native_integer numerator(object_ref const o)
  {
    return try_object<obj::ratio>(o)->data.numerator;
  }

  native_integer denominator(object_ref const o)
  {
    return try_object<obj::ratio>(o)->data.denominator;
  }

  native_integer to_int(object_ref const l)
  {
    return visit_number_like(
      [](auto const typed_l) -> native_integer { return typed_l->to_integer(); },
      l);
  }

  native_integer to_int(obj::integer_ref const l)
  {
    return l->data;
  }

  native_integer to_int(obj::real_ref const l)
  {
    return static_cast<native_integer>(l->data);
  }

  native_integer to_int(native_integer const l)
  {
    return l;
  }

  native_integer to_int(native_real const l)
  {
    return static_cast<native_integer>(l);
  }

  native_bool is_number(object_ref const o)
  {
    return visit_number_like([=](auto const) -> native_bool { return true; },
                             [=]() -> native_bool { return false; },
                             o);
  }

  native_bool is_integer(object_ref const o)
  {
    return o->type == object_type::integer;
  }

  native_bool is_real(object_ref const o)
  {
    return o->type == object_type::real;
  }

  native_bool is_ratio(object_ref const o)
  {
    return o->type == object_type::ratio;
  }

  native_bool is_boolean(object_ref const o)
  {
    return o->type == object_type::boolean;
  }

  native_bool is_nan(object_ref const o)
  {
    return visit_number_like(
      [=](auto const typed_o) -> native_bool {
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

  native_bool is_infinite(object_ref const o)
  {
    return visit_number_like(
      [=](auto const typed_o) -> native_bool {
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

  native_integer parse_long(object_ref const o)
  {
    if(auto const typed_o = dyn_cast<obj::persistent_string>(o))
    {
      return std::stoll(typed_o->data);
    }
    else
    {
      throw make_box(util::format("Expected string, got {}", object_type_str(o->type))).erase();
    }
  }

  native_real parse_double(object_ref const o)
  {
    if(auto const typed_o = dyn_cast<obj::persistent_string>(o))
    {
      return std::stod(typed_o->data);
    }
    else
    {
      throw make_box(util::format("Expected string, got {}", object_type_str(o->type))).erase();
    }
  }
}
