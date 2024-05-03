#include <jank/runtime/math.hpp>
#include <jank/runtime/behavior/numberable.hpp>

namespace jank::runtime
{
  /* TODO: visit_numberable */
  object_ptr add(object_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const r) -> object_ptr {
        using L = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<L>)
        {
          return visit_object(
            [](auto const typed_r, auto const typed_l) -> object_ptr {
              using R = typename decltype(typed_r)::value_type;

              if constexpr(behavior::numberable<R>)
              {
                return make_box(typed_l + typed_r->data);
              }
              else
              {
                throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
              }
            },
            r,
            typed_l->data);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  object_ptr add(obj::integer_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> object_ptr {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return make_box(typed_l + typed_r->data);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l->data);
  }

  object_ptr add(object_ptr const l, obj::integer_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> object_ptr {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return make_box(typed_l->data + typed_r);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r->data);
  }

  native_integer add(obj::integer_ptr const l, obj::integer_ptr const r)
  {
    return l->data + r->data;
  }

  native_real add(obj::real_ptr const l, obj::real_ptr const r)
  {
    return l->data + r->data;
  }

  native_real add(obj::real_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_real {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l + typed_r->data;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l->data);
  }

  native_real add(object_ptr const l, obj::real_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_real {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l->data + typed_r;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r->data);
  }

  native_real add(obj::real_ptr const l, obj::integer_ptr const r)
  {
    return l->data + r->data;
  }

  native_real add(obj::integer_ptr const l, obj::real_ptr const r)
  {
    return l->data + r->data;
  }

  native_real add(object_ptr const l, native_real const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_real {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l->data + typed_r;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  native_real add(native_real const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_real {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l + typed_r->data;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l);
  }

  native_real add(native_real const l, native_real const r)
  {
    return l + r;
  }

  native_real add(native_integer const l, native_real const r)
  {
    return l + r;
  }

  native_real add(native_real const l, native_integer const r)
  {
    return l + r;
  }

  object_ptr add(object_ptr const l, native_integer const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> object_ptr {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return make_box(typed_l->data + typed_r);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  object_ptr add(native_integer const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> object_ptr {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return make_box(typed_l + typed_r->data);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l);
  }

  native_integer add(native_integer const l, native_integer const r)
  {
    return l + r;
  }

  object_ptr sub(object_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const r) -> object_ptr {
        using L = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<L>)
        {
          return visit_object(
            [](auto const typed_r, auto const typed_l) -> object_ptr {
              using R = typename decltype(typed_r)::value_type;

              if constexpr(behavior::numberable<R>)
              {
                return make_box(typed_l - typed_r->data);
              }
              else
              {
                throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
              }
            },
            r,
            typed_l->data);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  object_ptr sub(obj::integer_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> object_ptr {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return make_box(typed_l - typed_r->data);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l->data);
  }

  object_ptr sub(object_ptr const l, obj::integer_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> object_ptr {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return make_box(typed_l->data - typed_r);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r->data);
  }

  native_integer sub(obj::integer_ptr const l, obj::integer_ptr const r)
  {
    return l->data - r->data;
  }

  native_real sub(obj::real_ptr const l, obj::real_ptr const r)
  {
    return l->data - r->data;
  }

  native_real sub(obj::real_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_real {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l - typed_r->data;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l->data);
  }

  native_real sub(object_ptr const l, obj::real_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_real {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l->data - typed_r;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r->data);
  }

  native_real sub(obj::real_ptr const l, obj::integer_ptr const r)
  {
    return l->data - r->data;
  }

  native_real sub(obj::integer_ptr const l, obj::real_ptr const r)
  {
    return l->data - r->data;
  }

  native_real sub(object_ptr const l, native_real const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_real {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l->data - typed_r;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  native_real sub(native_real const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_real {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l - typed_r->data;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l);
  }

  native_real sub(native_real const l, native_real const r)
  {
    return l - r;
  }

  native_real sub(native_integer const l, native_real const r)
  {
    return l - r;
  }

  native_real sub(native_real const l, native_integer const r)
  {
    return l - r;
  }

  object_ptr sub(object_ptr const l, native_integer const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> object_ptr {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return make_box(typed_l->data - typed_r);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  object_ptr sub(native_integer const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> object_ptr {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return make_box(typed_l - typed_r->data);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l);
  }

  native_integer sub(native_integer const l, native_integer const r)
  {
    return l - r;
  }

  object_ptr div(object_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const r) -> object_ptr {
        using L = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<L>)
        {
          return visit_object(
            [](auto const typed_r, auto const typed_l) -> object_ptr {
              using R = typename decltype(typed_r)::value_type;

              if constexpr(behavior::numberable<R>)
              {
                return make_box(typed_l / typed_r->data);
              }
              else
              {
                throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
              }
            },
            r,
            typed_l->data);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  object_ptr div(obj::integer_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> object_ptr {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return make_box(typed_l / typed_r->data);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l->data);
  }

  object_ptr div(object_ptr const l, obj::integer_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> object_ptr {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return make_box(typed_l->data / typed_r);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r->data);
  }

  native_integer div(obj::integer_ptr const l, obj::integer_ptr const r)
  {
    return l->data / r->data;
  }

  native_real div(obj::real_ptr const l, obj::real_ptr const r)
  {
    return l->data / r->data;
  }

  native_real div(obj::real_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_real {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l / typed_r->data;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l->data);
  }

  native_real div(object_ptr const l, obj::real_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_real {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l->data / typed_r;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r->data);
  }

  native_real div(obj::real_ptr const l, obj::integer_ptr const r)
  {
    return l->data / r->data;
  }

  native_real div(obj::integer_ptr const l, obj::real_ptr const r)
  {
    return l->data / r->data;
  }

  native_real div(object_ptr const l, native_real const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_real {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l->data / typed_r;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  native_real div(native_real const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_real {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l / typed_r->data;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l);
  }

  native_real div(native_real const l, native_real const r)
  {
    return l / r;
  }

  native_real div(native_integer const l, native_real const r)
  {
    return l / r;
  }

  native_real div(native_real const l, native_integer const r)
  {
    return l / r;
  }

  object_ptr div(object_ptr const l, native_integer const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> object_ptr {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return make_box(typed_l->data / typed_r);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  object_ptr div(native_integer const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> object_ptr {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return make_box(typed_l / typed_r->data);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l);
  }

  native_integer div(native_integer const l, native_integer const r)
  {
    return l / r;
  }

  object_ptr mul(object_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const r) -> object_ptr {
        using L = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<L>)
        {
          return visit_object(
            [](auto const typed_r, auto const typed_l) -> object_ptr {
              using R = typename decltype(typed_r)::value_type;

              if constexpr(behavior::numberable<R>)
              {
                return make_box(typed_l * typed_r->data);
              }
              else
              {
                throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
              }
            },
            r,
            typed_l->data);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  object_ptr mul(obj::integer_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> object_ptr {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return make_box(typed_l * typed_r->data);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l->data);
  }

  object_ptr mul(object_ptr const l, obj::integer_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> object_ptr {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return make_box(typed_l->data * typed_r);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r->data);
  }

  native_integer mul(obj::integer_ptr const l, obj::integer_ptr const r)
  {
    return l->data * r->data;
  }

  native_real mul(obj::real_ptr const l, obj::real_ptr const r)
  {
    return l->data * r->data;
  }

  native_real mul(obj::real_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_real {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l * typed_r->data;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l->data);
  }

  native_real mul(object_ptr const l, obj::real_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_real {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l->data * typed_r;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r->data);
  }

  native_real mul(obj::real_ptr const l, obj::integer_ptr const r)
  {
    return l->data * r->data;
  }

  native_real mul(obj::integer_ptr const l, obj::real_ptr const r)
  {
    return l->data * r->data;
  }

  native_real mul(object_ptr const l, native_real const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_real {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l->data * typed_r;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  native_real mul(native_real const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_real {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l * typed_r->data;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l);
  }

  native_real mul(native_real const l, native_real const r)
  {
    return l * r;
  }

  native_real mul(native_integer const l, native_real const r)
  {
    return l * r;
  }

  native_real mul(native_real const l, native_integer const r)
  {
    return l * r;
  }

  object_ptr mul(object_ptr const l, native_integer const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> object_ptr {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return make_box(typed_l->data * typed_r);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  object_ptr mul(native_integer const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> object_ptr {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return make_box(typed_l * typed_r->data);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l);
  }

  native_integer mul(native_integer const l, native_integer const r)
  {
    return l * r;
  }

  object_ptr rem(object_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const r) -> object_ptr {
        using L = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<L>)
        {
          return visit_object(
            [](auto const typed_r, auto const typed_l) -> object_ptr {
              using R = typename decltype(typed_r)::value_type;

              if constexpr(behavior::numberable<R>)
              {
                if constexpr(std::is_same_v<L, obj::real> || std::is_same_v<R, obj::real>)
                {
                  return make_box(std::fmod(typed_l, typed_r->data));
                }
                else
                {
                  return make_box(typed_l % typed_r->data);
                }
              }
              else
              {
                throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
              }
            },
            r,
            typed_l->data);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  object_ptr inc(object_ptr const l)
  {
    return visit_object(
      [](auto const typed_l) -> object_ptr {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return make_box(typed_l->data + 1);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l);
  }

  object_ptr dec(object_ptr const l)
  {
    return visit_object(
      [](auto const typed_l) -> object_ptr {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return make_box(typed_l->data - 1);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l);
  }

  native_bool is_zero(object_ptr const l)
  {
    return visit_object(
      [](auto const typed_l) -> native_bool {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
        {
          return typed_l->data == 0;
        }
#pragma clang diagnostic pop
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l);
  }

  native_bool is_pos(object_ptr const l)
  {
    return visit_object(
      [](auto const typed_l) -> native_bool {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
        {
          return typed_l->data > 0;
        }
#pragma clang diagnostic pop
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l);
  }

  native_bool is_neg(object_ptr const l)
  {
    return visit_object(
      [](auto const typed_l) -> native_bool {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
        {
          return typed_l->data < 0;
        }
#pragma clang diagnostic pop
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l);
  }

  native_bool is_equiv(object_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const r) -> native_bool {
        using L = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<L>)
        {
          return visit_object(
            [](auto const typed_r, auto const typed_l) -> native_bool {
              using R = typename decltype(typed_r)::value_type;

              if constexpr(behavior::numberable<R>)
              {
                using C = std::common_type_t<decltype(L::data), decltype(R::data)>;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
                return static_cast<C>(typed_l) == static_cast<C>(typed_r->data);
#pragma clang diagnostic pop
              }
              else
              {
                throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
              }
            },
            r,
            typed_l->data);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
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

  native_bool lt(object_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const r) -> native_bool {
        using L = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<L>)
        {
          return visit_object(
            [](auto const typed_r, auto const typed_l) -> native_bool {
              using R = typename decltype(typed_r)::value_type;

              if constexpr(behavior::numberable<R>)
              {
                return typed_l < typed_r->data;
              }
              else
              {
                throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
              }
            },
            r,
            typed_l->data);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  native_bool lt(obj::integer_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_bool {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l < typed_r->data;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l->data);
  }

  native_bool lt(object_ptr const l, obj::integer_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_bool {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l->data < typed_r;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r->data);
  }

  native_bool lt(obj::integer_ptr const l, obj::integer_ptr const r)
  {
    return l->data < r->data;
  }

  native_bool lt(obj::real_ptr const l, obj::real_ptr const r)
  {
    return l->data < r->data;
  }

  native_bool lt(obj::real_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_bool {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l < typed_r->data;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l->data);
  }

  native_bool lt(object_ptr const l, obj::real_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_bool {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l->data < typed_r;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r->data);
  }

  native_bool lt(obj::real_ptr const l, obj::integer_ptr const r)
  {
    return l->data < r->data;
  }

  native_bool lt(obj::integer_ptr const l, obj::real_ptr const r)
  {
    return l->data < r->data;
  }

  native_bool lt(object_ptr const l, native_real const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_bool {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l->data < typed_r;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  native_bool lt(native_real const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_bool {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l < typed_r->data;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l);
  }

  native_bool lt(native_real const l, native_real const r)
  {
    return l < r;
  }

  native_bool lt(native_integer const l, native_real const r)
  {
    return l < r;
  }

  native_bool lt(native_real const l, native_integer const r)
  {
    return l < r;
  }

  native_bool lt(object_ptr const l, native_integer const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_bool {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l->data < typed_r;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  native_bool lt(native_integer const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_bool {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l < typed_r->data;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l);
  }

  native_bool lt(native_integer const l, native_integer const r)
  {
    return l < r;
  }

  native_bool lte(object_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const r) -> native_bool {
        using L = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<L>)
        {
          return visit_object(
            [](auto const typed_r, auto const typed_l) -> native_bool {
              using R = typename decltype(typed_r)::value_type;

              if constexpr(behavior::numberable<R>)
              {
                return typed_l <= typed_r->data;
              }
              else
              {
                throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
              }
            },
            r,
            typed_l->data);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  native_bool lte(obj::integer_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_bool {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l <= typed_r->data;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l->data);
  }

  native_bool lte(object_ptr const l, obj::integer_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_bool {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l->data <= typed_r;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r->data);
  }

  native_bool lte(obj::integer_ptr const l, obj::integer_ptr const r)
  {
    return l->data <= r->data;
  }

  native_bool lte(obj::real_ptr const l, obj::real_ptr const r)
  {
    return l->data <= r->data;
  }

  native_bool lte(obj::real_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_bool {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l <= typed_r->data;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l->data);
  }

  native_bool lte(object_ptr const l, obj::real_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_bool {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l->data <= typed_r;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r->data);
  }

  native_bool lte(obj::real_ptr const l, obj::integer_ptr const r)
  {
    return l->data <= r->data;
  }

  native_bool lte(obj::integer_ptr const l, obj::real_ptr const r)
  {
    return l->data <= r->data;
  }

  native_bool lte(object_ptr const l, native_real const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_bool {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l->data <= typed_r;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  native_bool lte(native_real const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_bool {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l <= typed_r->data;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
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
    return l <= r;
  }

  native_bool lte(native_real const l, native_integer const r)
  {
    return l <= r;
  }

  native_bool lte(object_ptr const l, native_integer const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_bool {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l->data <= typed_r;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  native_bool lte(native_integer const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_bool {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l <= typed_r->data;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l);
  }

  native_bool lte(native_integer const l, native_integer const r)
  {
    return l <= r;
  }

  object_ptr min(object_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const r) -> object_ptr {
        using L = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<L>)
        {
          return visit_object(
            [](auto const typed_r, auto const typed_l) -> object_ptr {
              using R = typename decltype(typed_r)::value_type;

              if constexpr(behavior::numberable<R>)
              {
                using C = std::common_type_t<decltype(typed_l), decltype(typed_r->data)>;
                return make_box(std::min(static_cast<C>(typed_l), static_cast<C>(typed_r->data)));
              }
              else
              {
                throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
              }
            },
            r,
            typed_l->data);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  object_ptr min(obj::integer_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> object_ptr {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l), decltype(typed_r->data)>;
          return make_box(std::min(static_cast<C>(typed_l), static_cast<C>(typed_r->data)));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l->data);
  }

  object_ptr min(object_ptr const l, obj::integer_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> object_ptr {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l->data), decltype(typed_r)>;
          return make_box(std::min(static_cast<C>(typed_l->data), static_cast<C>(typed_r)));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r->data);
  }

  native_integer min(obj::integer_ptr const l, obj::integer_ptr const r)
  {
    return std::min(l->data, r->data);
  }

  native_real min(obj::real_ptr const l, obj::real_ptr const r)
  {
    return std::min(l->data, r->data);
  }

  native_real min(obj::real_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_real {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l), decltype(typed_r->data)>;
          return std::min(static_cast<C>(typed_l), static_cast<C>(typed_r->data));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l->data);
  }

  native_real min(object_ptr const l, obj::real_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_real {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l->data), decltype(typed_r)>;
          return std::min(static_cast<C>(typed_l->data), static_cast<C>(typed_r));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r->data);
  }

  native_real min(obj::real_ptr const l, obj::integer_ptr const r)
  {
    return std::min(l->data, static_cast<native_real>(r->data));
  }

  native_real min(obj::integer_ptr const l, obj::real_ptr const r)
  {
    return std::min(static_cast<native_real>(l->data), r->data);
  }

  native_real min(object_ptr const l, native_real const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_real {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l->data), decltype(typed_r)>;
          return std::min(static_cast<C>(typed_l->data), static_cast<C>(typed_r));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  native_real min(native_real const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_real {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l), decltype(typed_r->data)>;
          return std::min(static_cast<C>(typed_l), static_cast<C>(typed_r->data));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
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

  object_ptr min(object_ptr const l, native_integer const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> object_ptr {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l->data), decltype(typed_r)>;
          return make_box(std::min(static_cast<C>(typed_l->data), static_cast<C>(typed_r)));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  object_ptr min(native_integer const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> object_ptr {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l), decltype(typed_r->data)>;
          return make_box(std::min(static_cast<C>(typed_l), static_cast<C>(typed_r->data)));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l);
  }

  native_integer min(native_integer const l, native_integer const r)
  {
    return std::min(l, r);
  }

  object_ptr max(object_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const r) -> object_ptr {
        using L = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<L>)
        {
          return visit_object(
            [](auto const typed_r, auto const typed_l) -> object_ptr {
              using R = typename decltype(typed_r)::value_type;

              if constexpr(behavior::numberable<R>)
              {
                using C = std::common_type_t<decltype(typed_l), decltype(typed_r->data)>;
                return make_box(std::max(static_cast<C>(typed_l), static_cast<C>(typed_r->data)));
              }
              else
              {
                throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
              }
            },
            r,
            typed_l->data);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  object_ptr max(obj::integer_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> object_ptr {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l), decltype(typed_r->data)>;
          return make_box(std::max(static_cast<C>(typed_l), static_cast<C>(typed_r->data)));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l->data);
  }

  object_ptr max(object_ptr const l, obj::integer_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> object_ptr {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l->data), decltype(typed_r)>;
          return make_box(std::max(static_cast<C>(typed_l->data), static_cast<C>(typed_r)));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r->data);
  }

  native_integer max(obj::integer_ptr const l, obj::integer_ptr const r)
  {
    return std::max(l->data, r->data);
  }

  native_real max(obj::real_ptr const l, obj::real_ptr const r)
  {
    return std::max(l->data, r->data);
  }

  native_real max(obj::real_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_real {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l), decltype(typed_r->data)>;
          return std::max(static_cast<C>(typed_l), static_cast<C>(typed_r->data));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l->data);
  }

  native_real max(object_ptr const l, obj::real_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_real {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l->data), decltype(typed_r)>;
          return std::max(static_cast<C>(typed_l->data), static_cast<C>(typed_r));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r->data);
  }

  native_real max(obj::real_ptr const l, obj::integer_ptr const r)
  {
    return std::max(l->data, static_cast<native_real>(r->data));
  }

  native_real max(obj::integer_ptr const l, obj::real_ptr const r)
  {
    return std::max(static_cast<native_real>(l->data), r->data);
  }

  native_real max(object_ptr const l, native_real const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_real {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l->data), decltype(typed_r)>;
          return std::max(static_cast<C>(typed_l->data), static_cast<C>(typed_r));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  native_real max(native_real const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_real {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l), decltype(typed_r->data)>;
          return std::max(static_cast<C>(typed_l), static_cast<C>(typed_r->data));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
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

  object_ptr max(object_ptr const l, native_integer const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> object_ptr {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l->data), decltype(typed_r)>;
          return make_box(std::max(static_cast<C>(typed_l->data), static_cast<C>(typed_r)));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  object_ptr max(native_integer const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> object_ptr {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l), decltype(typed_r->data)>;
          return make_box(std::max(static_cast<C>(typed_l), static_cast<C>(typed_r->data)));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l);
  }

  native_integer max(native_integer const l, native_integer const r)
  {
    return std::max(l, r);
  }

  object_ptr abs(object_ptr const l)
  {
    return visit_object(
      [](auto const typed_l) -> object_ptr {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(std::is_same_v<T, obj::integer>)
        {
          return make_box(std::abs(typed_l->data));
        }
        if constexpr(std::is_same_v<T, obj::real>)
        {
          return make_box(std::fabs(typed_l->data));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l);
  }

  native_integer abs(obj::integer_ptr const l)
  {
    return std::abs(l->data);
  }

  native_real abs(obj::real_ptr const l)
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

  native_real sqrt(object_ptr const l)
  {
    return visit_object(
      [](auto const typed_l) -> native_real {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return std::sqrt(typed_l->to_real());
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l);
  }

  native_real sqrt(obj::integer_ptr const l)
  {
    return std::sqrt(l->data);
  }

  native_real sqrt(obj::real_ptr const l)
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

  native_real pow(object_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const r) -> native_real {
        using L = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<L>)
        {
          return visit_object(
            [](auto const typed_r, auto const typed_l) -> native_real {
              using R = typename decltype(typed_r)::value_type;

              if constexpr(behavior::numberable<R>)
              {
                using C = std::common_type_t<decltype(typed_l), decltype(typed_r->data)>;
                return std::pow(static_cast<C>(typed_l), static_cast<C>(typed_r->data));
              }
              else
              {
                throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
              }
            },
            r,
            typed_l->data);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  native_real pow(obj::integer_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_real {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l), decltype(typed_r->data)>;
          return std::pow(static_cast<C>(typed_l), static_cast<C>(typed_r->data));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l->data);
  }

  native_real pow(object_ptr const l, obj::integer_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_real {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l->data), decltype(typed_r)>;
          return std::pow(static_cast<C>(typed_l->data), static_cast<C>(typed_r));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r->data);
  }

  native_real pow(obj::integer_ptr const l, obj::integer_ptr const r)
  {
    return std::pow(l->data, r->data);
  }

  native_real pow(obj::real_ptr const l, obj::real_ptr const r)
  {
    return std::pow(l->data, r->data);
  }

  native_real pow(obj::real_ptr const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_real {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l), decltype(typed_r->data)>;
          return std::pow(static_cast<C>(typed_l), static_cast<C>(typed_r->data));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l->data);
  }

  native_real pow(object_ptr const l, obj::real_ptr const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_real {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l->data), decltype(typed_r)>;
          return std::pow(static_cast<C>(typed_l->data), static_cast<C>(typed_r));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r->data);
  }

  native_real pow(obj::real_ptr const l, obj::integer_ptr const r)
  {
    return std::pow(l->data, static_cast<native_real>(r->data));
  }

  native_real pow(obj::integer_ptr const l, obj::real_ptr const r)
  {
    return std::pow(static_cast<native_real>(l->data), r->data);
  }

  native_real pow(object_ptr const l, native_real const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_real {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l->data), decltype(typed_r)>;
          return std::pow(static_cast<C>(typed_l->data), static_cast<C>(typed_r));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  native_real pow(native_real const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_real {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l), decltype(typed_r->data)>;
          return std::pow(static_cast<C>(typed_l), static_cast<C>(typed_r->data));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
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

  native_real pow(object_ptr const l, native_integer const r)
  {
    return visit_object(
      [](auto const typed_l, auto const typed_r) -> native_real {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l->data), decltype(typed_r)>;
          return std::pow(static_cast<C>(typed_l->data), static_cast<C>(typed_r));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l,
      r);
  }

  native_real pow(native_integer const l, object_ptr const r)
  {
    return visit_object(
      [](auto const typed_r, auto const typed_l) -> native_real {
        using T = typename decltype(typed_r)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          using C = std::common_type_t<decltype(typed_l), decltype(typed_r->data)>;
          return std::pow(static_cast<C>(typed_l), static_cast<C>(typed_r->data));
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_r->to_string()) };
        }
      },
      r,
      l);
  }

  native_real pow(native_integer const l, native_integer const r)
  {
    return std::pow(l, r);
  }

  native_integer to_int(object_ptr const l)
  {
    return visit_object(
      [](auto const typed_l) -> native_integer {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        {
          return typed_l->to_integer();
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) };
        }
      },
      l);
  }

  native_integer to_int(obj::integer_ptr const l)
  {
    return l->data;
  }

  native_integer to_int(obj::real_ptr const l)
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
}
