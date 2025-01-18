#include <bit>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <codecvt>
#include <locale>
#pragma clang diagnostic pop

#include <jank/util/string_builder.hpp>

namespace jank::util
{
  static void realloc(string_builder &sb, size_t const required)
  {
    auto const new_capacity{ std::bit_ceil(required) };
    auto const new_data{ new(PointerFreeGC) string_builder::value_type[new_capacity] };
    string_builder::traits_type::copy(new_data, sb.buffer, sb.pos);
    delete sb.buffer;
    sb.buffer = new_data;
    sb.capacity = new_capacity;
  }

  static void maybe_realloc(string_builder &sb, size_t const additional_size)
  {
    auto const required_size{ sb.pos + additional_size + 1 };
    if(sb.capacity < required_size)
    {
      realloc(sb, required_size);
    }
  }

  static void write(string_builder &sb, char const * const str, size_t const size)
  {
    string_builder::traits_type::copy(sb.buffer + sb.pos, str, size);
    sb.pos += size;
  }

  string_builder::string_builder()
  {
    realloc(*this, capacity);
  }

  string_builder::string_builder(size_t const capacity)
    : capacity{ capacity }
  {
    realloc(*this, capacity);
  }

  string_builder::~string_builder()
  {
    delete buffer;
  }

  string_builder &string_builder::operator()(native_bool const d) &
  {
    if(d)
    {
      auto const required{ 4 };
      maybe_realloc(*this, required);
      write(*this, "true", required);
    }
    else
    {
      auto const required{ 5 };
      maybe_realloc(*this, required);
      write(*this, "false", required);
    }

    return *this;
  }

  string_builder &string_builder::operator()(native_integer const d) &
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    auto const required{ snprintf(nullptr, 0, "%lld", d) };
    maybe_realloc(*this, required);

    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    snprintf(buffer + pos, capacity - pos, "%lld", d);
    pos += required;

    return *this;
  }

  string_builder &string_builder::operator()(native_real const d) &
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    auto const required{ snprintf(nullptr, 0, "%f", d) };
    maybe_realloc(*this, required);

    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    snprintf(buffer + pos, capacity - pos, "%f", d);
    pos += required;

    return *this;
  }

  string_builder &string_builder::operator()(native_hash const d) &
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    auto const required{ snprintf(nullptr, 0, "%d", d) };
    maybe_realloc(*this, required);

    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    snprintf(buffer + pos, capacity - pos, "%d", d);
    pos += required;

    return *this;
  }

  string_builder &string_builder::operator()(void const * const d) &
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    auto const required{ snprintf(nullptr, 0, "%p", d) };
    maybe_realloc(*this, required);

    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    snprintf(buffer + pos, capacity - pos, "%p", d);
    pos += required;

    return *this;
  }

  string_builder &string_builder::operator()(int const d) &
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    auto const required{ snprintf(nullptr, 0, "%d", d) };
    maybe_realloc(*this, required);

    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    snprintf(buffer + pos, capacity - pos, "%d", d);
    pos += required;

    return *this;
  }

  string_builder &string_builder::operator()(size_t const d) &
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    auto const required{ snprintf(nullptr, 0, "%zu", d) };
    maybe_realloc(*this, required);

    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    snprintf(buffer + pos, capacity - pos, "%zu", d);
    pos += required;

    return *this;
  }

  string_builder &string_builder::operator()(char const d) &
  {
    maybe_realloc(*this, 1);

    buffer[pos] = d;
    ++pos;

    return *this;
  }

  string_builder &string_builder::operator()(char32_t const d) &
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
#pragma clang diagnostic pop
    return (*this)(converter.to_bytes(d));
  }

  string_builder &string_builder::operator()(char const * const d) &
  {
    auto const required{ strlen(d) };
    maybe_realloc(*this, required);

    write(*this, d, required);

    return *this;
  }

  string_builder &string_builder::operator()(native_transient_string const &d) &
  {
    auto const required{ d.size() };
    maybe_realloc(*this, required);

    write(*this, d.data(), required);

    return *this;
  }

  string_builder &string_builder::operator()(native_persistent_string const &d) &
  {
    auto const required{ d.size() };
    maybe_realloc(*this, required);

    write(*this, d.data(), required);

    return *this;
  }

  void string_builder::push_back(native_bool const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(native_integer const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(native_real const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(native_hash const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(void const * const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(int const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(size_t const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(char const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(char32_t const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(char const * const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(native_transient_string const &d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(native_persistent_string const &d) &
  {
    (*this)(d);
  }

  void string_builder::reserve(size_t const new_capacity)
  {
    if(capacity < new_capacity)
    {
      realloc(*this, new_capacity);
    }
  }

  string_builder::value_type *string_builder::data() const
  {
    return buffer;
  }

  size_t string_builder::size() const
  {
    return pos;
  }

  native_persistent_string string_builder::release()
  {
    assert(pos < capacity);

    native_persistent_string ret;
    if(pos <= native_persistent_string::max_small_size)
    {
      ret.init_small(buffer, pos);
    }
    else
    {
      ret.init_large_shared(buffer, pos);
    }

    pos = capacity = 0;
    buffer = nullptr;

    return ret;
  }

  native_transient_string string_builder::str() const
  {
    assert(pos < capacity);
    buffer[pos] = 0;
    return { buffer, pos };
  }

  native_persistent_string_view string_builder::view() const &
  {
    assert(pos < capacity);
    buffer[pos] = 0;
    return { buffer, pos };
  }
}
